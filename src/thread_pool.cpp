#include "thread_pool.h"

// 线程本地存储定义
thread_local size_t thread_pool::index = 0;

// join_threads 实现
join_threads::join_threads(std::vector<std::thread>& threads_) : threads(threads_) {}

join_threads::~join_threads() {
    std::cout << "Joining threads..." << std::endl;
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "All threads joined." << std::endl;
}

// function_wrapper 实现
function_wrapper::function_wrapper(function_wrapper&& other) noexcept : impl(other.impl) {
    other.impl = nullptr;
}

function_wrapper& function_wrapper::operator=(function_wrapper&& other) noexcept {
    if(this != &other) {
        delete impl;
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

function_wrapper::~function_wrapper() {
    if(impl) {
        delete impl;
        impl = nullptr;
    }
}

void function_wrapper::operator()() {
    if(impl) {
        impl->call();
    }
}

// pipe_line 实现
pipe_line::pipe_line() 
    : mtx(std::make_unique<std::mutex>()), 
      cond_var(std::make_unique<std::condition_variable>()) {}

// thread_pool 实现
thread_pool::thread_pool(size_t size) 
    : done(false), thread_count(size), joiner(threads) {
    try{
        threads.reserve(size);
        local_queues.resize(size);
        global_queue.reserve(size);
        for(size_t i = 0; i < size; ++i) {
            threads.emplace_back(&thread_pool::worker_thread, this);
            local_queue_index.insert(std::make_pair(threads[threads.size() - 1].get_id(), i));
            global_queue.push_back(&local_queues[i]);
        }
        for(size_t i = 0; i < size; ++i) {
           active_threads[i] = false;
        }
    }
    catch(const std::exception& e) {
        done = true;
        throw std::runtime_error("Failed to create thread pool: " + std::string(e.what()));
    }
}

thread_pool::~thread_pool() {
    wait();
    shutdown();
    std::cout << "Thread pool destroyed." << std::endl;
}

void thread_pool::push_task(function_wrapper&& task) {
    global_queue.front()->tasks.push_back(std::move(task));
    global_queue.front()->cond_var->notify_one();
    std::make_heap(global_queue.begin(), global_queue.end(), [&](pipe_line* left, pipe_line* right){
        return left->tasks.size() > right->tasks.size();
    });
}

void thread_pool::worker_thread() {
    index = local_queue_index[std::this_thread::get_id()];
    std::unique_lock<std::mutex> lock(*(local_queues[index].mtx));
    function_wrapper task;
   
    while(!done) {
        ((local_queues[index].cond_var))->wait(lock, [this] {
            return !local_queues[index].tasks.empty() || done;
        });

        if(local_queues[index].tasks.empty() || done) {
            continue;
        }

        active_threads[index] = true;

        task = std::move(local_queues[index].tasks.front());
        
        try {
            task();
            local_queues[index].tasks.pop_front();
        }
        catch(const std::exception& e) {
            std::cerr << "Exception in thread " << std::this_thread::get_id() << ": " << e.what() << std::endl;
        }
    }
}

void thread_pool::steal(size_t index) {
    for(size_t i = 0; i < thread_count; ++i) {
        if(i == index || !active_threads[i]) continue;

        if(!local_queues[i].tasks.empty()) {
            function_wrapper task = std::move(local_queues[i].tasks.back());
            local_queues[i].tasks.pop_back();
            local_queues[index].tasks.push_front(std::move(task));
            break;
        }
    }
}

void thread_pool::print_task_count_in_thread() {
    for(size_t i = 0; i < thread_count; i++) {
        std::cout << "Thread " << i << " has " << local_queues[i].tasks.size() << " tasks." << std::endl;
    }
}

void thread_pool::shutdown() {
    done = true;
    for(auto& queue : local_queues) {
         queue.cond_var->notify_all();
    }
}

void thread_pool::wait() {
    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for(auto& queue : local_queues) {
            all_done &= queue.tasks.empty();
        }
    }
}
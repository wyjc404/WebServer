#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <future>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <memory>

// 前向声明
class join_threads;
class function_wrapper;
struct pipe_line;

class join_threads {
    std::vector<std::thread>& threads;
public: 
    explicit join_threads(std::vector<std::thread>& threads_);
    ~join_threads();
};

class function_wrapper {
private:
    struct impl_base {
        impl_base() = default;
        impl_base(const impl_base&) = delete;
        impl_base& operator=(const impl_base&) = delete;
        virtual ~impl_base() = default;
        virtual void call() = 0;
    };

    template <typename FunctionType>
    struct impl_type : impl_base {
        FunctionType func;
        impl_type(FunctionType&& f);
        void call() override;
    };

    impl_base* impl = nullptr;

public:
    template<typename FunctionType>
    function_wrapper(FunctionType&& func);
    
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) noexcept;
    function_wrapper& operator=(function_wrapper&& other) noexcept;
    ~function_wrapper();
    
    void operator()();
};

struct pipe_line {
    pipe_line();
    std::deque<function_wrapper> tasks;
    std::unique_ptr<std::mutex> mtx;
    std::unique_ptr<std::condition_variable> cond_var;
};

class thread_pool final {
private:
    void push_task(function_wrapper&& task);
    void worker_thread();
    void steal(size_t index);

private:
    std::atomic<bool> done;
    std::atomic<size_t> thread_count;

    std::vector<pipe_line> local_queues;
    std::vector<pipe_line*> global_queue;

    std::unordered_map<std::thread::id, size_t> local_queue_index;
    
    static thread_local size_t index;

    bool active_threads[20];

    std::vector<std::thread> threads;
    join_threads joiner;

public:
    thread_pool(size_t size = std::thread::hardware_concurrency());
    ~thread_pool();

    template <typename FunctionType>
    std::future<std::invoke_result_t<FunctionType>> submit(FunctionType&& func);

    void print_task_count_in_thread();
    void shutdown();
    void wait();
};

template <typename FunctionType>
function_wrapper::impl_type<FunctionType>::impl_type(FunctionType&& f) : func(std::move(f)) {}

template <typename FunctionType>
void function_wrapper::impl_type<FunctionType>::call() { 
    func(); 
}

template<typename FunctionType>
function_wrapper::function_wrapper(FunctionType&& func)
    : impl(new impl_type<FunctionType>(std::forward<FunctionType>(func))) {
    if(!impl) {
        throw std::bad_alloc();
    }
}

template <typename FunctionType>
std::future<std::invoke_result_t<FunctionType>> thread_pool::submit(FunctionType&& func) {
    using return_type = std::invoke_result_t<FunctionType>;
    std::packaged_task<return_type()> task(std::forward<FunctionType>(func));
    std::future<return_type> res = task.get_future();

    if(done) {
        throw std::runtime_error("Thread pool is already stopped");
    }

    push_task(std::move(task));
    
    return res;
}

#endif // THREAD_POOL_H
#pragma once

#include <cstddef>
#include <string.h>

class Buffer {
public:
    Buffer(size_t size_);
    void clear();
    void resize(size_t size_);
    char* getBuffer();
    size_t getSize();
    size_t getLen();
private:
    size_t size;
    char* buffer;
};

#include <Buffer.h>
#include <cstddef>

Buffer::Buffer(size_t size_) : size(size_) {
    buffer = new char[size];
}
void Buffer::clear() {
    bzero(buffer, size);
}

void Buffer::resize(size_t size_) {
    delete buffer;
    buffer = new char(size_);
    size = size_;
}

char* Buffer::getBuffer() { return buffer; }
size_t Buffer::getSize() { return size; }
size_t Buffer::getLen() { return strlen(buffer); }
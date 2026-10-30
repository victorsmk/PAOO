#ifndef PAOO_BAD_EXAMPLES_HPP
#define PAOO_BAD_EXAMPLES_HPP

#include <l1/tracker.hpp>

#include <algorithm>
#include <cstddef>

namespace paoo {

///@brief NO destructor -> every object leaks its heap array.
class LeakyBuffer
{
public:
    explicit LeakyBuffer(std::size_t size) : size_(size), data_(new int[size]())
    {
        ++liveBlocks();
    }
    // missing: ~LeakyBuffer() { delete[] data_; }

private:
    std::size_t size_;
    int* data_;
};

///@brief Has a destructor, but NO copy constructor -> the compiler generates
///       a member-wise (shallow) copy: both objects share the same array.
class ShallowBuffer
{
public:
    explicit ShallowBuffer(std::size_t size) : size_(size), data_(new int[size]())
    {
        ++liveBlocks();
    }

    ~ShallowBuffer()
    {
        delete[] data_;
        --liveBlocks();
    }

    int* data() { return data_; }

private:
    std::size_t size_;
    int* data_;
};

///@brief Correct deep copy, but NO move constructor. Declaring a copy
///       constructor/destructor suppresses the implicit move constructor,
///       so every "move" silently becomes an expensive copy.
class CopyOnlyBuffer
{
public:
    explicit CopyOnlyBuffer(std::size_t size) : size_(size), data_(new int[size]())
    {
        ++liveBlocks();
    }

    CopyOnlyBuffer(const CopyOnlyBuffer& other)
        : size_(other.size_), data_(new int[other.size_])
    {
        std::copy(other.data_, other.data_ + size_, data_);
        ++liveBlocks();
        ++copyCount();
    }

    ~CopyOnlyBuffer()
    {
        delete[] data_;
        --liveBlocks();
    }

private:
    std::size_t size_;
    int* data_;
};

} // namespace paoo

#endif // PAOO_BAD_EXAMPLES_HPP
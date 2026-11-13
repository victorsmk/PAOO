#include <l1/buffer.hpp>
#include <l1/tracker.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace paoo
{

    // Members are initialized in the initializer list, in declaration order
    // (name_, size_, data_).
    Buffer::Buffer(const std::string &name, std::size_t size, int fill)
        : name_(name), size_(size), data_(new int[size])
    {
        std::fill(data_, data_ + size_, fill);
        ++liveBlocks();
    }

    Buffer::Buffer(const Buffer &other)
        : name_(other.name_ + " (copy)"), size_(other.size_), data_(new int[other.size_]) // NEW block, not the pointer of 'other'
    {
        std::copy(other.data_, other.data_ + size_, data_);
        ++liveBlocks();
        ++copyCount();
    }

    Buffer::Buffer(Buffer &&other) noexcept
        : name_(std::move(other.name_)), size_(other.size_), data_(other.data_) // steal the pointer, no allocation, no copy
    {
        other.size_ = 0;       // leave 'other' valid but empty
        other.data_ = nullptr; // so its destructor does not free our block
        ++moveCount();
    }

    Buffer &Buffer::operator=(const Buffer &other)
    {
        if (this == &other) // self-assignment
            return *this;

        int *newData = new int[other.size_]; // allocate first: if it throws, *this is unchanged
        std::copy(other.data_, other.data_ + other.size_, newData);

        if (data_ != nullptr)
            --liveBlocks();
        delete[] data_;

        data_ = newData; // copy ALL parts
        size_ = other.size_;
        name_ = other.name_;
        ++liveBlocks();
        ++copyCount();

        return *this; 
    }

    Buffer &Buffer::operator=(Buffer &&other) noexcept
    {
        if (this == &other)
            return *this;

        if (data_ != nullptr)
        {
            delete[] data_;
            --liveBlocks();
        }

        name_ = std::move(other.name_);
        size_ = other.size_;
        data_ = other.data_;

        other.size_ = 0;
        other.data_ = nullptr;
        ++moveCount();

        return *this;
    }

    Buffer::~Buffer()
    {
        if (data_ != nullptr)
        {
            delete[] data_;
            --liveBlocks();
        }
    }

    const std::string &Buffer::name() const { return name_; }
    std::size_t Buffer::size() const { return size_; }
    const int *Buffer::raw() const { return data_; }

    int Buffer::at(std::size_t index) const
    {
        if (index >= size_)
            throw std::out_of_range("Buffer::at");
        return data_[index];
    }

    void Buffer::set(std::size_t index, int value)
    {
        if (index >= size_)
            throw std::out_of_range("Buffer::set");
        data_[index] = value;
    }

} // namespace paoo
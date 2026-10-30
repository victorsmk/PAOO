#ifndef PAOO_BUFFER_HPP
#define PAOO_BUFFER_HPP

#include <cstddef>
#include <string>

namespace paoo {

///@brief Correct implementation: owns a heap array of ints.
class Buffer
{
public:
    Buffer(const std::string& name, std::size_t size, int fill = 0);

    ///@brief Copy constructor: deep copy (new array, same content)
    Buffer(const Buffer& other);

    ///@brief Move constructor: steals the array of 'other'
    Buffer(Buffer&& other) noexcept;

    ///@brief Destructor: releases the heap memory
    ~Buffer();

    // Assignment operators are out of scope for this assignment
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

public:
    const std::string& name() const;
    std::size_t size() const;
    int at(std::size_t index) const;          // bounds checked
    void set(std::size_t index, int value);   // bounds checked
    const int* raw() const;                   // only to compare addresses in demos

private:
    // The state is private: the invariant "data_ points to size_ valid ints
    // (or is nullptr when size_ == 0)" can only be broken from inside the class.
    std::string name_;
    std::size_t size_;
    int* data_;
};

} // namespace paoo

#endif // PAOO_BUFFER_HPP
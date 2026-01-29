#pragma once
#include <vector>
#include <memory>
#include <type_traits>

namespace Falcor
{
/// This is a very rudimentary drop in replacement for STL vector that,
/// unlike MSVC STL vector does not have a global lock on every push_back,
/// which makes Debug code significantly slower when running multithreaded
template<typename T>
class fast_vector
{
public:
    static_assert(std::is_trivial_v<T>, "Fast vector can only be used on trivial types.");

    fast_vector() = default;
    fast_vector(const std::vector<T>& other) { assign(other.begin(), other.end()); }

    fast_vector(const fast_vector& other) { assign(other.begin(), other.end()); }

    fast_vector(fast_vector&& other)
        : m_capacity(std::exchange(other.m_capacity, 0)), m_size(std::exchange(other.m_size, 0)), m_data(std::move(other.m_data))
    {}

    fast_vector& operator=(const std::vector<T>& other)
    {
        assign(other.begin(), other.end());
        return *this;
    }

    fast_vector& operator=(const fast_vector& other)
    {
        assign(other.begin(), other.end());
        return *this;
    }

    fast_vector& operator=(fast_vector&& other)
    {
        m_size = std::exchange(other.m_size, 0);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_data = std::move(other.m_data);
        return *this;
    }

    void reserve(size_t capacity) { grow(capacity); }

    void resize(size_t size)
    {
        grow(size);
        m_size = size;
    }

    void resize(size_t capacity, const T& value)
    {
        grow(capacity);
        for (size_t i = m_size; i < capacity; ++i)
            m_data[i] = value;
        m_size = capacity;
    }

    void push_back(const T& v)
    {
        if (m_size + 1 > m_capacity)
            grow(m_size + 1);
        m_data[m_size++] = v;
    }

    template<typename FwdIterator>
    void assign(FwdIterator b, FwdIterator e)
    {
        resize(std::distance(b, e));
        for (size_t i = 0; i < size(); ++i, ++b)
            m_data[i] = *b;
    }

    operator std::vector<T>() { return std::vector<T>(begin(), end()); }

    inline size_t size() const { return m_size; }
    inline size_t capacity() const { return m_capacity; }
    inline bool empty() const { return m_size == 0; }

    inline void clear() { m_size = 0; }

    inline T* begin() { return data(); }
    inline T* end() { return begin() + size(); }
    inline const T* begin() const { return data(); }
    inline const T* end() const { return begin() + size(); }

    inline T* data() { return m_data.get(); }
    inline const T* data() const { return m_data.get(); }

    inline T& operator[](size_t pos) { return m_data[pos]; }
    inline const T& operator[](size_t pos) const { return m_data[pos]; }

private:
    void grow(size_t required_size)
    {
        if (required_size <= m_capacity)
            return;
        size_t new_size = std::max(m_capacity * 2, required_size);
        std::unique_ptr<T[]> new_data = std::make_unique<T[]>(new_size);
        memcpy(new_data.get(), m_data.get(), m_size * sizeof(T));
        m_data = std::move(new_data);
        m_capacity = new_size;
    }

private:
    size_t m_capacity{0};
    size_t m_size{0};
    std::unique_ptr<T[]> m_data;
};
} // namespace Falcor

// Copyright (c) Microsoft. Licensed under the MIT License.
#pragma once

#include <cstdint>

template<typename T>
class Span
{
public:
    Span() : m_data(nullptr), m_count(0) {}
    Span(T* data, uint32_t count) : m_data(data), m_count(count) {}

    T* data() { return m_data; }
    const T* data() const { return m_data; }
    T& back() { return *(m_data + m_count - 1); }
    const T& back() const { return *(m_data + m_count - 1); }
    size_t size() const { return m_count; }
    T* begin() { return m_data; }
    T* end() { return m_data + m_count; }
    T& operator[](uint32_t i) { return *(m_data + i); }
    const T& operator[](uint32_t i) const { return *(m_data + i); }

private:
    T* m_data;
    uint32_t m_count;
};

template<typename T>
Span<T> MakeSpan(T* data, uint32_t size)
{
    return Span<T>(data, size);
}

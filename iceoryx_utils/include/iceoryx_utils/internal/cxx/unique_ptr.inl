
#ifndef IOX_UTILS_CXX_UNIQUE_PTR_INL
#define IOX_UTILS_CXX_UNIQUE_PTR_INL

#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace cxx
{

template<typename T>
unique_ptr<T>::unique_ptr(std::function<void(T*)>&& deleter) noexcept : m_deleter(deleter)
{}

template<typename T>
unique_ptr<T>::unique_ptr(ptr_t ptr, std::function<void(T*)>&& deleter) noexcept : m_ptr(ptr), m_deleter(deleter)
{}

// Can't have this function because the signature clashes with the case where T = void.
// If we definitely want it, need to make it considered only when T is not void...
//template<typename T>
//unique_ptr<T>::unique_ptr(void* allocation, std::function<void(T*)>&& deleter) noexcept
//    : m_ptr(reinterpret_cast<T*>(allocation)), m_deleter(deleter)
//{}

template<typename T>
unique_ptr<T>::unique_ptr(std::nullptr_t)  noexcept
{
  reset();
}

template<typename T>
unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr&& rhs) noexcept
{
    if(this != &rhs)
    {
        reset(rhs.m_ptr);
        m_deleter = std::move(rhs.m_deleter);
    }
    return *this;
}

template<typename T>
unique_ptr<T>& unique_ptr<T>::operator=(std::nullptr_t) noexcept
{
  reset();
  return *this;
}


template<typename T>
unique_ptr<T>::unique_ptr(unique_ptr&& rhs) noexcept
{
    *this = std::move(rhs);
}

template<typename T>
unique_ptr<T>::~unique_ptr() noexcept
{
    reset();
}

/// Dereference the stored pointer.
template<typename T>
T unique_ptr<T>::operator*() noexcept
{
    return *get();
}

/// Return the stored pointer.
template<typename T>
T* unique_ptr<T>::operator->() noexcept
{
    return get();
}


template<typename T>
T* unique_ptr<T>::get() const noexcept
{
  return m_ptr;
}

template<typename T>
T* unique_ptr<T>::release() noexcept
{
    auto ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
}

template<typename T>
void unique_ptr<T>::reset(T* ptr) noexcept
{
  if(m_ptr && m_deleter)
  {
      m_deleter(m_ptr);
  }
  m_ptr = ptr;
}

template<typename T>
void unique_ptr<T>::swap(unique_ptr<T>& other) noexcept
{
    if(other)
    {
        // Release pointers from both instances.
        auto thisPtr = release();
        auto otherPtr = other.release();
        // Set new pointers on both instances.
        reset(otherPtr);
        other.reset(thisPtr);
    }
    else
    {
        reset();
    }

}


// Comparison Operators

template<typename T, typename U>
bool operator==(const unique_ptr<T>& x, const unique_ptr<U>& y)
{
    return x.get() == y.get();
}

template<typename T>
bool operator==(const unique_ptr<T>& x, std::nullptr_t)
{
    return !x;
}

template<typename T>
bool operator==(std::nullptr_t, const unique_ptr<T>& x)
{
    return !x;
}

template<typename T, typename U>
bool operator!=(const unique_ptr<T>& x, const unique_ptr<U>& y)
{
    return x.get() != y.get();
}

template<typename T>
bool operator!=(const unique_ptr<T>& x, std::nullptr_t)
{
    return (bool)x;
}

template<typename T>
bool operator!=(std::nullptr_t, const unique_ptr<T>& x)
{
    return (bool)x;
}

} // namespace iox
} // namespace popo

#endif // IOX_UTILS_CXX_UNIQUE_PTR_INL

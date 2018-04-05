/** @file os/api_common.hpp
 *  @brief Common components shared by the jel os API.
 *
 *  @author Jonathan Thomson 
 */
/**
 * MIT License
 * 
 * Copyright 2018, Jonathan Thomson 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

/** C/C++ Standard Library Headers */
#include <cstdint>
#include <iterator>


/** jel Library Headers */

namespace jel
{
/* Iterator
* Template class for implementating an interator for simple, array based containers.
* Iterator begin() and end() functions should be provided in the container definition to
* enable use with ranged for loops.
*/
template <typename T>
class Iterator : public std::iterator<std::random_access_iterator_tag, T>
{
public:
  constexpr Iterator(T* baseItem, size_t index = 0) { ptr = baseItem; ptr += index; }
  Iterator& operator++() { this->ptr++; return *this; }
  Iterator operator++(int) { auto temp(*this); ++ptr; return temp; }
  Iterator& operator--() { this->ptr--; return *this; }
  Iterator operator--(int) { auto temp(*this); --ptr; return temp; }
  bool operator==(const Iterator& rhs) const 
    { if(this->ptr != rhs.ptr) { return true; } return false; }
  bool operator!=(const Iterator& rhs) const 
    { if(this->ptr != rhs.ptr) { return true; } return false; }
  T& operator*() { return *ptr; }
  T& operator->() { return *ptr; }
private:
  T* ptr;
};

template <typename T>
class ReverseIterator : public std::reverse_iterator<Iterator<T>>
{
public:
  constexpr ReverseIterator(T* baseItem, size_t index = 0) { ptr = baseItem; ptr -= index; }
  ReverseIterator& operator++() { this->ptr--; return *this; }
  ReverseIterator operator++(int) { auto temp(*this); --ptr; return temp; }
  ReverseIterator& operator--() { this->ptr++; return *this; }
  ReverseIterator operator--(int) { auto temp(*this); ++ptr; return temp; }
  bool operator==(const ReverseIterator& rhs) const 
    { if(this->ptr == rhs.ptr) { return true; } return false; }
  bool operator!=(const ReverseIterator& rhs) const 
    { if(this->ptr != rhs.ptr) { return true; } return false; }
  T& operator*() { return *ptr; }
  T& operator->() { return *ptr; }
private:
  T* ptr;
};

/* ConstIterator
* Template class for implementating an interator for simple, const array based containers.
*/
template <typename T>
class ConstIterator : public std::iterator<std::random_access_iterator_tag, T>
{
public:
  constexpr ConstIterator(const T* baseItem, size_t index = 0) 
    { ptr = baseItem; ptr += index; }
  ConstIterator& operator++() { this->ptr++; return *this; }
  ConstIterator operator++(int) { auto temp(*this); ++ptr; return temp; }
  ConstIterator& operator--() { this->ptr--; return *this; }
  ConstIterator operator--(int) { auto temp(*this); --ptr; return temp; }
  bool operator==(const ConstIterator& rhs) const
    { if(this->ptr == rhs.ptr) { return true; } return false; }
  bool operator!=(const ConstIterator& rhs) const
    { if(this->ptr != rhs.ptr) { return true; } return false; }
  const T& operator*() { return *ptr; }
  const T& operator->() { return *ptr; }
private:
  const T* ptr;
};

template <typename T>
class ConstReverseIterator : public std::reverse_iterator<ConstIterator<T>>
{
public:
  constexpr ConstReverseIterator(const T* baseItem, size_t index = 0) 
    { ptr = baseItem; ptr -= index; }
  ConstReverseIterator& operator++() { this->ptr--; return *this; }
  ConstReverseIterator operator++(int) { auto temp(*this); --ptr; return temp; }
  ConstReverseIterator& operator--() { this->ptr++; return *this; }
  ConstReverseIterator operator--(int) { auto temp(*this); ++ptr; return temp; }
  bool operator==(const ConstReverseIterator& rhs) const
    { if(this->ptr == rhs.ptr) { return true; } return false; }
  bool operator!=(const ConstReverseIterator& rhs) const
    { if(this->ptr != rhs.ptr) { return true; } return false; }
  const T& operator*() { return *ptr; }
  const T& operator->() { return *ptr; }
private:
  const T* ptr;
};

/** @class ScopeGuard
 *  @brief Used to optionally perform an RAII action when created and destroyed.
 *  */
template<typename F>
class ScopeGuard
{
public:
  ScopeGuard(const F& onExit) : exit_(onExit) { }
  template<typename Fcreate>
  ScopeGuard(const Fcreate& onEntry, const F& onExit) : exit_(onExit) 
    { runOnExit = true; onEntry(); }
  ~ScopeGuard() { if(runOnExit) { exit_(); } }
  bool runOnExit;
private:
  F exit_;
};

/** 
 *  ScopeGuard specialization, only an exit function will be run.
 * */
template<typename F>
class ScopeGuard<F> ToScopeGuard(const F& onExit)
{
  return ScopeGuard<F>(onExit);
}

/** 
 *  ScopeGuard specialization, both an entry and exit function will be run.
 * */
template<typename Fentry, typename F>
class ScopeGuard<F> ToScopeGuard(const Fentry& onEntry, const F& onExit)
{
  return ScopeGuard<F>(onEntry, onExit);
}

enum class Status
{
  success = 0,
  failure
};

} /** namespace jel */

/** @file os/api_string.hpp
 *  @brief A custom string class more suitable for embedded use than std::string.
 *
 *  @detail
 *    A custom jel String class. The jel String class is based on the std::string implementation
 *    as closely as possible, with a few key differences. These are:
 *      -Specalized allocator support to allow for fine tuning allocation paramaters to an extent
 *      not possible by simply providing a custom alloctor to a std::string. It is even possible
 *      to statically allocate in a buffer and disable allocation completely.
 *      -Gauranteed underlying char* type storage accesible for write operations (on non-const
 *      strings). This allows writing to the string directly from lower level routines. The
 *      downside to this is that length() checks may require a call to std::strlen() when called.
 *      -Capability of being stored directly in ROM/Flash memory with zero RAM requirement.
 *      -Support for debug protections on element access functions. Bounds are checked and out of
 *      bounds access will cause an assertion. Note: For performance reasons, when assertions are
 *      disabled this is also disabled (i.e. on release builds).
 *      -std::initializer_list is not supported.
 *      -Only single byte/8bit characters are supported.
 *      -Currently std::ios <</>> operators are not supported. This is due to the heavy memory
 *      footprint of the <iostream> library.
 *      -Conversion functions (stoi, stol, etc) are available as member functions of the jel::String
 *      class, as well as to_string functions.
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

/** jel Library Headers */
#include "os/api_common.hpp"

namespace jel
{

/** String
 *  The jel String class provides a custom, embedded friendly C++ std::string style string object.
 * */
class String
{
public:
  using StrIterator = Iterator<char>; 
  using RStringIterator = ReverseIterator<char>;
  using ConstStrIterator = ConstIterator<char>;
  using RConstStrIterator = ReverseIterator<char>;
  /** Used for various things in the string. See C++ STL standard for details, this is used in an
   * identical manner. */
  static const size_t npos = -1;
  /** Default construct an empty string. */
  String();
  String(const char* cStr);
  /** Destructor. */
  ~String();
  String(const String& other);
  String& operator=(const String& other);
  String& operator=(const char* cStr);
  String(String&& other) noexcept;
  String& operator=(String&& other) noexcept;
  /** Element Access */
  char& at(const size_t position) noexcept;
  char& operator[](const size_t index) noexcept;
  char& front() noexcept;
  char& back() noexcept;
  /** Similiar to the C++17 std::string class, the pointer returned by data() points to the
   * underlying const char* buffer. Up to capacity() characters (not including a null terminator)
   * can be stored safely within the buffer. */
  char* data() noexcept;
  const char& at(const size_t position) const noexcept;
  const char& operator[](const size_t index) const noexcept;
  const char& front() const noexcept;
  const char& back() const noexcept;
  const char* c_str() const noexcept;
  const char* data() const noexcept;
  /** Iterators */
  StrIterator begin() noexcept;
  StrIterator end() noexcept;
  RStringIterator rbegin() noexcept;
  RStringIterator rend() noexcept;
  ConstStrIterator begin() const noexcept;
  ConstStrIterator cbegin() const noexcept;
  ConstStrIterator end() const noexcept;
  ConstStrIterator cend() const noexcept;
  RConstStrIterator rbegin() const noexcept;
  RConstStrIterator crbegin() const noexcept;
  RConstStrIterator rend() const noexcept;
  RConstStrIterator crend() const noexcept;
  /** Capacity */
  bool empty() const noexcept;
  size_t size() const noexcept;
  size_t length() const noexcept;
  size_t max_size() const noexcept;
  void reserve(const size_t newSize = 0);
  size_t capacity() const noexcept;
  /** Operations */
  void clear() noexcept;
  String& insert(const size_t idx, const size_t cnt, const char c);
  String& insert(const size_t idx, const char* cStr);
  String& insert(const size_t idx, const char* cStr, const size_t cnt);
  String& insert(const size_t idx, const String& str);
  String& insert(const size_t idx, const String& str, const size_t strIdx, const size_t cnt = npos);
  StrIterator insert(StrIterator pos, const char c);
  StrIterator insert(StrIterator pos, StrIterator first, StrIterator last);
  StrIterator insert(ConstStrIterator pos, const size_t cnt, const char c);
  String& erase(const size_t idx = 0, const size_t cnt = npos);
  StrIterator erase(StrIterator pos);
  StrIterator erase(ConstStrIterator pos);
  StrIterator erase(StrIterator first, StrIterator last);
  StrIterator erase(ConstStrIterator first, ConstStrIterator last);
  void push_back(const char c);
  void pop_back();
  String& append(const size_t cnt, const char c);
  String& append(const String& str);
  String& append(const String& str, const size_t pos, const size_t cnt = npos);
  String& append(const char* cStr, const size_t cnt);
  String& append(const char* cStr);
  String& append(StrIterator first, StrIterator last);
  String& operator+=(const String& str);
  String& operator+=(const char c);
  String& operator+=(const char* c);
  int32_t compare(const String& str) const;
  int32_t compare(const size_t pos1, const size_t cnt1, 
    const String& str, const size_t pos2, const size_t count2 = npos) const;
  int32_t compare(const char* cStr) const;
  int32_t compare(const size_t pos1, const size_t cnt1, const char* cStr) const;
  int32_t compare(const size_t pos1, const size_t cnt1, const char* cStr, const size_t cnt2) const;
  String& replace(const size_t pos, const size_t cnt, const String& str);
  String& replace(ConstStrIterator first, ConstStrIterator last, const String& str);
  String& replace(const size_t pos, const size_t cnt, 
    const String& str, const size_t pos2, const size_t cnt2 = npos);
  String& replace(ConstStrIterator first, ConstStrIterator last, 
    StrIterator first2, StrIterator last2);
  String& replace(const size_t pos, const size_t cnt, const char* cStr, const size_t cnt2);
  String& replace(ConstStrIterator first, ConstStrIterator last,
    const char* cStr, const size_t cnt2);
  String& replace(const size_t pos, const size_t cnt, const char* cStr);
  String& replace(ConstStrIterator first, ConstStrIterator last, const char* cStr);
  String& replace(const size_t pos, const size_t cnt, const size_t cnt2, const char c);
  String& replace(ConstStrIterator first, ConstStrIterator last, const size_t cnt2, const char c);
  String substr(const size_t pos = 0, const size_t cnt = npos) const;
  size_t copy(char* dest, const size_t cnt, const size_t pos = 0) const;
  void resize(const size_t cnt);
  void resize(const size_t cnt, const char c);
  void swap(String& other);
  /** Search */
  size_t find(const String& str, const size_t pos = 0) const;
  size_t find(const char* cStr, const size_t pos, const size_t cnt) const;
  size_t find(const char* cStr, const size_t pos = 0) const;
  size_t find(const char c, const size_t pos = 0) const;
  size_t rfind(const String& str, const size_t pos = npos) const;
  size_t rfind(const char* cStr, const size_t pos, const size_t cnt) const;
  size_t rfind(const char* cStr, const size_t pos = npos) const;
  size_t rfind(const char c, const size_t pos = npos) const;
  size_t find_first_of(const String& str, const size_t pos = 0) const;
  size_t find_first_of(const char* cStr, const size_t pos, const size_t cnt) const;
  size_t find_first_of(const char* cStr, const size_t pos = 0) const;
  size_t find_first_of(const char c, const size_t pos = 0) const;
  size_t find_first_not_of(const String& str, const size_t pos = 0) const;
  size_t find_first_not_of(const char* cStr, const size_t pos, const size_t cnt) const;
  size_t find_first_not_of(const char* cStr, const size_t pos = 0) const;
  size_t find_first_not_of(const char c, const size_t pos = 0) const;
  size_t find_last_of(const String& str, const size_t pos = npos) const;
  size_t find_last_of(const char* cStr, const size_t pos, const size_t cnt) const;
  size_t find_last_of(const char* cStr, const size_t pos = npos) const;
  size_t find_last_of(const char c, const size_t pos = npos) const;
  size_t find_last_not_of(const String& str, const size_t pos = npos) const;
  size_t find_last_not_of(const char* cStr, const size_t pos, const size_t cnt) const;
  size_t find_last_not_of(const char* cStr, const size_t pos = npos) const;
  size_t find_last_not_of(const char c, const size_t pos = npos) const;
  /** Conversion functions */
  int32_t to_int32() const;
  int64_t to_int64() const;
  uint32_t to_uint32() const;
  uint64_t to_uint64() const;
  float to_float() const;
  double to_double() const;
  /** Note: These functions differ somewhat in behaviour from the std::to_string functions. They
   * will erase the current string data and convert the argument value into a string (or an empty
   * string if the argument can't be converted). The advantage to these functions is they do not
   * throw an exception if the argument is invalid - this can save considerable execution time if
   * the input is messy and does not consistently convert successfully. */
  void to_string(const int64_t value);
  void to_string(const uint64_t value);
  void to_string(const double value);
private:
  char* buf;
  size_t len;
  size_t cap;
};
/** Concantenation operators */
String operator+(const String& lhs, const String& rhs);
String operator+(const char* lhs, const String& rhs);
String operator+(const char lhs, const String& rhs);
String operator+(const String& lhs, const char* rhs);
String operator+(const String& lhs, const char rhs);
/** Comparison operators */
bool operator==(const String& lhs, const String& rhs);
bool operator!=(const String& lhs, const String& rhs);
bool operator<(const String& lhs, const String& rhs);
bool operator<=(const String& lhs, const String& rhs);
bool operator>(const String& lhs, const String& rhs);
bool operator>=(const String& lhs, const String& rhs);
bool operator==(const char* lhs, const String& rhs);
bool operator==(const String& lhs, const char* rhs);
bool operator!=(const char* lhs, const String& rhs);
bool operator!=(const String& lhs, const char* rhs);
bool operator<(const char* lhs, const String& rhs);
bool operator<(const String& lhs, const char* rhs);
bool operator<=(const char* lhs, const String& rhs);
bool operator<=(const String& lhs, const char* rhs);
bool operator>(const char* lhs, const String& rhs);
bool operator>(const String& lhs, const char* rhs);
bool operator>=(const char* lhs, const String& rhs);
bool operator>=(const String& lhs, const char* rhs);
/** Construct a String from numeric input. */
String to_string(const int64_t value);
String to_string(const uint64_t value);
String to_string(const double value);

}

#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <cstring>
#include <strings.h>
#include <type_traits>
#include <utility>

constexpr int size_start = 8;
constexpr int malloc_times = 2;

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T> class vector {
private:
  T *pointer_;
  // 1-based
  size_t size_now;
  size_t size_total;
  bool copy_ = 0;

  //空间扩张。
  void space() {
    T *new_pointer_ = (T *)operator new(sizeof(T) * size_total * malloc_times);
    //所有权不必转移，资源不应释放，因此不能析构。
    if (copy_) {
      memmove(new_pointer_, pointer_, sizeof(T) * size_total);
    } else {
      for (int i = 0; i < size_now; ++i) {
        new (new_pointer_ + i) T(std::move(pointer_[i]));
      }
    }
    // pointer_对应空间失去所有者，释放。
    operator delete(pointer_, size_total * sizeof(T));
    pointer_ = new_pointer_;
    size_total = malloc_times * size_total;
  }

public:
  class const_iterator;
  class iterator;

  //构造函数：默认，拷贝，移动。
  vector() {
    if (std::is_trivially_copyable<T>::value) {
      copy_ = 1;
    }
    pointer_ = (T *)operator new(sizeof(T) * size_start);
    size_now = 0;
    size_total = size_start;
  }
  vector(const vector &other) {
    //复制资源。
    size_now = other.size_now;
    size_total = other.size_total;
    while (size_total / malloc_times > size_now) {
      size_total /= malloc_times;
    }
    pointer_ = (T *)operator new(sizeof(T) * size_total);
    //直接复制有共用所有权导致可能bug的嫌疑，应当利用拷贝构造。
    for (int i = 0; i < other.size_now; ++i) {
      new (pointer_ + i) T(other[i]);
    }
  }
  vector(vector &&other) {
    //直接接管资源。
    pointer_ = other.pointer_;
    size_now = other.size_now;
    size_total = other.size_total;
    other.pointer_ = nullptr;
  }

  ~vector() {
    //显式调用析构函数，释放资源。（初始化即分配，有构造则有析构）
    for (int i = 0; i < size_now; ++i) {
      pointer_[i].~T();
    }
    operator delete(pointer_, size_total * sizeof(T));
  }

  vector &operator=(const vector &other) {
    if (other.pointer_ == pointer_) {
      return *this;
    }
    for (int i = 0; i < size_now; ++i) {
      pointer_[i].~T();
    }
    operator delete(pointer_, size_total * sizeof(T));
    size_now = other.size_now;
    size_total = other.size_total;
    while (size_total / malloc_times > size_now) {
      size_total /= malloc_times;
    }
    pointer_ = (T *)operator new(sizeof(T) * size_total);
    for (int i = 0; i < other.size_now; ++i) {
      new (pointer_ + i) T(other[i]);
    }
    return *this;
  }

  vector &operator=(vector &&other) {
    if (other.pointer_ == pointer_) {
      return *this;
    }
    for (int i = 0; i < size_now; ++i) {
      pointer_[i].~T();
    }
    operator delete(pointer_, size_total * sizeof(T));
    pointer_ = other.pointer_;
    size_now = other.size_now;
    size_total = other.size_total;
    other.pointer_ = nullptr;
    return *this;
  }

  T &at(const size_t &pos) {
    if (pos >= size_now) {
      throw index_out_of_bound();
    }
    return pointer_[pos];
  }
  const T &at(const size_t &pos) const {
    if (pos >= size_now) {
      throw index_out_of_bound();
    }
    return pointer_[pos];
  }

  T &operator[](const size_t &pos) {
    if (pos >= size_now) {
      throw index_out_of_bound();
    }
    return pointer_[pos];
  }
  const T &operator[](const size_t &pos) const {
    if (pos >= size_now) {
      throw index_out_of_bound();
    }
    return pointer_[pos];
  }

  const T &front() const {
    if (size_now == 0) {
      throw container_is_empty();
    }
    return *pointer_;
  }

  const T &back() const {
    if (size_now == 0) {
      throw container_is_empty();
    }
    return pointer_[size_now - 1];
  }

  bool empty() const { return size_now == 0; }

  size_t size() const { return size_now; }

  void clear() {
    for (int i = 0; i < size_now; ++i) {
      pointer_[i].~T();
    }
    operator delete(pointer_, size_total * sizeof(T));
    pointer_ = (T *)operator new(sizeof(T) * size_start);
    size_now = 0;
    size_total = size_start;
  }

  void push_back(const T &value) {
    ++size_now;
    if (size_now >= size_total) {
      space();
    }
    new (pointer_ + size_now - 1) T(value);
  }

  void pop_back() {
    if (size_now == 0) {
      throw container_is_empty();
    }
    pointer_[size_now - 1].~T();
    --size_now;
  }

  class iterator {
    // The following code is written for the C++ type_traits library.
    // Type traits is a C++ feature for describing certain properties of a
    // type. For instance, for an iterator, iterator::value_type is the type
    // that the iterator points to. STL algorithms and containers may use
    // these type_traits (e.g. the following typedef) to work properly. In
    // particular, without the following code,
    // @code{std::sort(iter, iter1);} would not compile.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type:
    // https://blog.csdn.net/u01size_startmalloc_times99153/article/details/7malloc_timessize_start19713
    // About iterator_category: https://en.cppreference.com/w/cpp/iterator
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer__ = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

  private:
    T *start_;
    T *content_;

  public:
    iterator() : start_(nullptr), content_(nullptr) {}

    iterator(T *start, T *address) : start_(start), content_(address){};
    iterator operator+(const int &n) const {
      return iterator(start_, content_ + n);
    }
    iterator operator-(const int &n) const {
      return iterator(start_, content_ - n);
    }

    int operator-(const iterator &rhs) const {
      if (start_ != rhs.start_) {
        throw invalid_iterator();
      }
      return content_ - rhs.content_;
    }
    iterator &operator+=(const int &n) {
      content_ += n;
      return *this;
    }
    iterator &operator-=(const int &n) {
      content_ -= n;
      return *this;
    }

    iterator operator++(int) {
      iterator tmp(*this);
      ++content_;
      return tmp;
    }

    iterator &operator++() {
      ++content_;
      return *this;
    }

    iterator operator--(int) {
      iterator tmp(*this);
      --content_;
      return tmp;
    }

    iterator &operator--() {
      --content_;
      return *this;
    }

    T &operator*() const { return *content_; }

    bool operator==(const iterator &rhs) const {
      return start_ == rhs.start_ && content_ == rhs.content_;
    }
    bool operator==(const const_iterator &rhs) const {
      return start_ == rhs.start_ && content_ == rhs.content_;
    }

    bool operator!=(const iterator &rhs) const {
      return start_ != rhs.start_ || content_ != rhs.content_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return start_ != rhs.start_ || content_ != rhs.content_;
    }
    friend class vector;
  };
  class const_iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer__ = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

  private:
    T *start_;
    const T *content_;

  public:
    const_iterator() : start_(nullptr), content_(nullptr) {}
    const_iterator(T *start, T *address) : start_(start), content_(address) {}

    const_iterator operator+(const int &n) const {
      return const_iterator(start_, content_ + n);
    }
    const_iterator operator-(const int &n) const {
      return const_iterator(start_, content_ - n);
    }

    int operator-(const iterator &rhs) const {
      if (start_ != rhs.start_) {
        throw invalid_iterator();
      }
      return content_ - rhs.content_;
    }

    const_iterator operator++(int) {
      const_iterator tmp(*this);
      ++content_;
      return tmp;
    }

    const_iterator &operator++() {
      ++content_;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator tmp(*this);
      --content_;
      return tmp;
    }

    const_iterator &operator--() {
      --content_;
      return *this;
    }

    const T &operator*() const { return *content_; }

    bool operator==(const iterator &rhs) const {
      return start_ == rhs.start_ && content_ == rhs.content_;
    }
    bool operator==(const const_iterator &rhs) const {
      return start_ == rhs.start_ && content_ == rhs.content_;
    }

    bool operator!=(const iterator &rhs) const {
      return start_ != rhs.start_ || content_ != rhs.content_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return start_ != rhs.start_ || content_ != rhs.content_;
    }
    friend class vector;
  };

  iterator begin() { return iterator(pointer_, pointer_); }
  const_iterator cbegin() const { return const_iterator(pointer_, pointer_); }

  iterator end() { return iterator(pointer_, pointer_ + size_now); }
  const_iterator cend() const {
    return const_iterator(pointer_, pointer_ + size_now);
  }

  iterator insert(iterator pos, const T &value) {
    ++size_now;
    if (size_now >= size_total) {
      space();
    }
    //有区别但我看不出来。
    if (copy_) {
      memmove(pos.content_ + 1, pos.content_,
              (size_now - (pos.content_ - pointer_) - 1) * sizeof(T));
    } else {
      for (auto i = pointer_ + size_now; i > pos.content_; --i) {
        new (i) T(std::move(*(i - 1)));
      }
    }
    new (pos.content_) T(value);
    ++pos.content_;
    return iterator(pointer_, pos.content_ - 1);
  }

  iterator insert(const size_t &ind, const T &value) {
    if (ind > size_now) {
      throw index_out_of_bound();
    }
    ++size_now;
    if (size_now >= size_total) {
      space();
    }
    if (copy_) {
      memmove(pointer_ + ind + 1, pointer_ + ind,
              (size_now - ind - 1) * sizeof(T));
    } else {
      for (auto i = size_now; i > ind; --i) {
        new (pointer_ + i) T(std::move(pointer_[i - 1]));
      }
    }
    new (pointer_ + ind) T(value);
    return iterator(pointer_, pointer_ + ind);
  }

  iterator erase(iterator pos) {
    if (pos == end()) {
      return pos;
    }
    --size_now;
    pos->~T();
    if (copy_) {
      memmove(pos.content_, pos.content_ + 1,
              (size_now - (pos.content_ - pos.start_)) * sizeof(T));
    } else {
      for (auto i = pos.content_; i < pointer_ + size_now; ++i) {
        new (i) T(std::move(*(i + 1)));
      }
    }
    //此时末尾出现了一个不应支配资源但仍可解读的数据，是否会出问题？
    return pos;
  }

  iterator erase(const size_t &ind) {
    if (ind >= size_now) {
      throw index_out_of_bound();
    }
    (pointer_ + ind)->~T();
    if (copy_) {
      memmove(pointer_ + ind, pointer_ + ind + 1, (size_now - ind) * sizeof(T));
    } else {
      for (int i = ind; i <= size_now; ++i) {
        new (pointer_ + i) T(std::move(pointer_[i + 1]));
      }
    }
    return iterator(pointer_, pointer_ + ind);
  }
};

} // namespace sjtu

#endif
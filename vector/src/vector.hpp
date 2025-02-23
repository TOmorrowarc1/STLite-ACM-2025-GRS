#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <cstring>
#include <strings.h>

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

  //空间扩张。
  void space() {
    T *new_pointer_ = (T *)operator new(sizeof(T) * size_total * 2);
    memmove(new_pointer_, pointer_, size_total);
    operator delete(pointer_, sizeof(T) * size_total);
    pointer_ = new_pointer_;
    size_total = 2 * size_total;
  }

public:
  class const_iterator;
  class iterator;
  /**
   * TODO
   * a type for actions of the elements of a vector, and you should write
   *   a class named const_iterator with same interfaces.
   */
  /**
   * you can see RandomAccessIterator at CppReference for help.
   */

  /**
   * TODO Constructs
   * At least two: default constructor, copy constructor
   */
  vector() {
    pointer_ = (T *)operator new(sizeof(T) * 4);
    size_now = 0;
    size_total = 4;
  }
  vector(const vector &other) {
    pointer_ = (T *)operator new(sizeof(T) * other.size_total);
    size_now = other.size_now;
    size_total = other.size_total;
    memmove(pointer_, other.pointer_, sizeof(T) * size_total);
  }
  vector(vector &&other) {
    pointer_ = other.pointer_;
    size_now = other.size_now;
    size_total = other.size_total;
    other.pointer_ = nullptr;
  }

  /**
   * TODO Destructor
   */
  ~vector() { operator delete(pointer_, size_total); }
  /**
   * TODO Assignment operator
   */
  vector &operator=(const vector &other) {
    pointer_ = (T *)operator new(sizeof(T) * other.size_total);
    size_now = other.size_now;
    size_total = other.size_total;
    memmove(pointer_, other.pointer_, sizeof(T) * size_total);
    return *this;
  }
  vector &operator=(vector &&other) {
    if (other.pointer_ == pointer_) {
      return *this;
    }
    operator delete(pointer_, size_total);
    pointer_ = other.pointer_;
    size_now = other.size_now;
    size_total = other.size_total;
    other.pointer_ = nullptr;
    return *this;
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   */
  T &at(const size_t &pos) {
    if (pos >= size_now) {
      throw index_out_of_bound();
    } else {
      return pointer_[pos];
    }
  }
  const T &at(const size_t &pos) const {
    if (pos >= size_now) {
      throw index_out_of_bound();
    } else {
      return pointer_[pos];
    }
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   * !!! Pay attentions
   *   In STL this operator does not check the boundary but I want you to do.
   */
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
  /**
   * access the first element.
   * throw container_is_empty if size == 0
   */
  const T &front() const {
    if (size_now == 0) {
      throw container_is_empty();
    }
    return *pointer_;
  }
  /**
   * access the last element.
   * throw container_is_empty if size == 0
   */
  const T &back() const {
    if (size_now == 0) {
      throw container_is_empty();
    }
    return pointer_[size_now - 1];
  }
  /**
   * checks whether the container is empty
   */
  bool empty() const { return size_now == 0; }
  /**
   * returns the number of elements
   */
  size_t size() const { return size_now; }
  /**
   * clears the contents
   */
  void clear() {
    operator delete(pointer_, size_total);
    pointer_ = (T *)operator new(sizeof(T) * 4);
    size_now = 0;
    size_total = 4;
  }
  /**
   * adds an element to the end.
   */
  void push_back(const T &value) {
    ++size_now;
    if (size_now >= size_total) {
      space();
    }
    pointer_[size_now - 1] = value;
  }
  /**
   * remove the last element from the end.
   * throw container_is_empty if size() == 0
   */
  void pop_back() {
    if (size_now == 0) {
      throw container_is_empty();
    }
    --size_now;
  }

  class iterator {
    // The following code is written for the C++ type_traits library.
    // Type traits is a C++ feature for describing certain properties of a type.
    // For instance, for an iterator, iterator::value_type is the type that the
    // iterator points to.
    // STL algorithms and containers may use these type_traits (e.g. the
    // following typedef) to work properly. In particular, without the following
    // code,
    // @code{std::sort(iter, iter1);} would not compile.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type:
    // https://blog.csdn.net/u014299153/article/details/72419713 About
    // iterator_category: https://en.cppreference.com/w/cpp/iterator
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer__ = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

  private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    T *start_;
    // 0-based
    int number_;

  public:
    iterator() {
      start_ = nullptr;
      number_ = 0;
    }
    iterator(T *start, int number) : start_(start), number_(number) {}
    /**
     * return a new iterator which pointer__ n-next elements
     * as well as operator-
     */
    iterator operator+(const int &n) const {
      return iterator(start_, number_ + n);
    }
    iterator operator-(const int &n) const {
      return iterator(start_, number_ - n);
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw
    // invaild_iterator.
    int operator-(const iterator &rhs) const {
      if (start_ != rhs.start_) {
        throw invalid_iterator();
      }
      return number_ - rhs.number_;
    }
    iterator &operator+=(const int &n) {
      number_ += n;
      return *this;
    }
    iterator &operator-=(const int &n) {
      number_ -= n;
      return *this;
    }
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      // copy one time.
      iterator tmp(*this);
      ++number_;
      return tmp;
    }
    /**
     * TODO ++iter
     */
    iterator &operator++() {
      ++number_;
      return *this;
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator tmp(*this);
      --number_;
      return tmp;
    }
    /**
     * TODO --iter
     */
    iterator &operator--() {
      --number_;
      return *this;
    }
    /**
     * TODO *it
     */
    T &operator*() const { return start_[number_]; }
    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory address).
     */
    bool operator==(const iterator &rhs) const {
      return start_ == rhs.start_ && number_ == rhs.number_;
    }
    bool operator==(const const_iterator &rhs) const {
      return start_ == rhs.start_ && number_ == rhs.number_;
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return start_ != rhs.start_ || number_ != rhs.number_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return start_ != rhs.start_ || number_ != rhs.number_;
    }
    friend class vector;
  };
  /**
   * TODO
   * has same function as iterator, just for a const object.
   */
  class const_iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer__ = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

  private:
  private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    const T *start_;
    const int number_;

  public:
    const_iterator() : start_(nullptr), number_(0) {}
    const_iterator(T *start, int number) : start_(start), number_(number) {}
    /**
     * return a new iterator which pointer__ n-next elements
     * as well as operator-
     */
    const_iterator operator+(const int &n) const {
      return const_iterator(start_, number_ + n);
    }
    const_iterator operator-(const int &n) const {
      return const_iterator(start_, number_ - n);
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw
    // invaild_iterator.
    int operator-(const iterator &rhs) const {
      if (start_ != rhs.start_) {
        throw invalid_iterator();
      }
      return number_ - rhs.number_;
    }
    /**
     * TODO *it
     */
    T &operator*() const { return start_[number_]; }
    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory address).
     */
    bool operator==(const iterator &rhs) const {
      return start_ == rhs.start_ && number_ == rhs.number_;
    }
    bool operator==(const const_iterator &rhs) const {
      return start_ == rhs.start_ && number_ == rhs.number_;
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return start_ != rhs.start_ || number_ != rhs.number_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return start_ != rhs.start_ || number_ != rhs.number_;
    }
    friend class vector;
  };
  /**
   * returns an iterator to the beginning.
   */
  iterator begin() { return iterator(pointer_, 0); }
  const_iterator cbegin() const { return const_iterator(pointer_, 0); }
  /**
   * returns an iterator to the end.
   */
  iterator end() { return iterator(pointer_, size_now - 1); }
  const_iterator cend() const { return const_iterator(pointer_, size_now - 1); }
  /**
   * inserts value before pos
   * returns an iterator pointing to the inserted value.
   */
  iterator insert(iterator pos, const T &value) {
    ++size_now;
    if (size_now >= size_total) {
      space();
    }
    memmove(pointer_ + (pos.number_ + 1) * sizeof(T),
            pointer_ + pos.number_ * sizeof(T), size_now - pos.number_);
    pointer_[pos.number_] = value;
    ++pos.number_;
    return iterator(pointer_, pos.number_);
  }
  /**
   * inserts value at index ind.
   * after inserting, this->at(ind) == value
   * returns an iterator pointing to the inserted value.
   * throw index_out_of_bound if ind > size (in this situation ind can be size
   * because after inserting the size will increase 1.)
   */
  iterator insert(const size_t &ind, const T &value) {
    if (ind > size_now) {
      throw index_out_of_bound();
    }
    ++size_now;
    if (size_now >= size_total) {
      space();
    }
    memmove(pointer_ + (ind + 1) * sizeof(T), pointer_ + ind * sizeof(T),
            size_now - ind);
    pointer_[ind] = value;
    return iterator(pointer_, ind);
  }
  /**
   * removes the element at pos.
   * return an iterator pointing to the following element.
   * If the iterator pos refers the last element, the end() iterator is
   * returned.
   */
  iterator erase(iterator pos) {
    if (pos == end()) {
      return pos;
    }
    --size_now;
    memmove(pointer_ + (pos.number_) * sizeof(T),
            pointer_ + (pos.number_ + 1) * sizeof(T),
            (size_now - pos.number_) * sizeof(T));
    return pos;
  }
  /**
   * removes the element with index ind.
   * return an iterator pointing to the following element.
   * throw index_out_of_bound if ind >= size
   */
  iterator erase(const size_t &ind) {
    if (ind >= size_now) {
      throw index_out_of_bound();
    }
    memmove(pointer_ + ind * sizeof(T), pointer_ + (ind + 1) * sizeof(T),
            (size_now - ind) * sizeof(T));
    return iterator(pointer_, ind - 1);
  }
};

} // namespace sjtu

#endif
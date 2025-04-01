/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include "exceptions.hpp"
#include "utility.hpp"
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <strings.h>

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key>> class map {
public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef pair<const Key, T> value_type;
  const bool red = 1;
  const bool black = 0;
  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator;

private:
  class Node {
  private:
    bool color_;
    Node *parent_;
    Node *left_child_;
    Node *right_child_;
    value_type *content_;

  public:
    Node()
        : color_(0), parent_(nullptr), left_child_(nullptr),
          right_child_(nullptr), content_(nullptr) {}

    Node(const value_type &content)
        : color_(0), parent_(nullptr), left_child_(nullptr),
          right_child_(nullptr) {
      content_ = new value_type(content);
    }

    ~Node() {
      parent_ = left_child_ = right_child_ = nullptr;
      delete content_;
    }

    /*For a parent and its right_child, rotate and exchange them.*/
    Node *left_rotation(Node *parent_before, Node *parent_after) {
      if (parent_after == nullptr) {
        throw std::exception();
      }
      parent_after->parent_ = parent_before->parent_;
      if (parent_before->parent_ != nullptr) {
        if (parent_before->parent_->left_child_ == parent_before) {
          parent_before->parent_->left_child_ = parent_after;
        } else {
          parent_before->parent_->right_child_ = parent_after;
        }
      }
      parent_before->right_child_ = parent_after->left_child_;
      if (parent_after->left_child_ != nullptr) {
        parent_after->left_child_->parent_ = parent_before;
      }
      parent_after->left_child_ = parent_before;
      parent_before->parent_ = parent_after;
      return parent_after;
    }

    Node *right_rotation(Node *parent_before, Node *parent_after) {
      if (parent_after == nullptr) {
        throw std::exception();
      }
      parent_after->parent_ = parent_before->parent_;
      if (parent_before->parent_ != nullptr) {
        if (parent_before->parent_->left_child_ == parent_before) {
          parent_before->parent_->left_child_ = parent_after;
        } else {
          parent_before->parent_->right_child_ = parent_after;
        }
      }
      parent_before->left_child_ = parent_after->right_child_;
      if (parent_after->right_child_ != nullptr) {
        parent_after->right_child_->parent_ = parent_before;
      }
      parent_after->right_child_ = parent_before;
      parent_before->parent_ = parent_after;
      return parent_after;
    }

    void swap(Node *high, Node *low) {
      Node *temp;
      bool temp_color = high->color_;
      high->color_ = temp->color_;
      low->color_ = temp_color;
      if (high->left_child_ == low) {
        low->parent_ = high->parent_;
        high->parent_ = low;
        high->left_child_ = low->left_child_;
        low->left_child_ = high;
        temp = high->right_child_;
        high->right_child_ = low->right_child_;
        low->right_child_ = temp;
        if (low->parent_ != nullptr) {
          if (low->parent_->left_child_ == high) {
            low->parent_->left_child_ = low;
          } else {
            low->parent_->right_child_ = low;
          }
        }
        if (low->right_child_ != nullptr) {
          low->right_child_->parent_ = low;
        }
        if (high->left_child_ != nullptr) {
          high->left_child_->parent_ = high;
        }
      } else if (high->right_child_ == low) {
        low->parent_ = high->parent_;
        high->parent_ = low;
        high->right_child_ = low->right_child_;
        low->right_child_ = high;
        temp = high->left_child_;
        high->right_child_ = low->left_child_;
        low->left_child_ = temp;
        if (low->parent_ != nullptr) {
          if (low->parent_->left_child_ == high) {
            low->parent_->left_child_ = low;
          } else {
            low->parent_->right_child_ = low;
          }
        }
        if (low->right_child_ != nullptr) {
          low->right_child_->parent_ = low;
        }
        if (high->left_child_ != nullptr) {
          high->left_child_->parent_ = high;
        }
      } else {
        temp = high->parent_;
        high->parent_ = low->parent_;
        low->parent_ = high->parent_;
        temp = high->left_child_;
        high->left_child_ = low->left_child_;
        low->left_child_ = temp;
        temp = high->right_child_;
        high->right_child_ = low->right_child_;
        low->right_child_ = temp;
        if (low->parent_ != nullptr) {
          if (low->parent_->left_child_ == high) {
            low->parent_->left_child_ = low;
          } else {
            low->parent_->right_child_ = low;
          }
        }
        if (high->parent_ != nullptr) {
          if (high->parent_->left_child_ == low) {
            high->parent_->left_child_ = high;
          } else {
            high->parent_->right_child_ = high;
          }
        }
        if (low->right_child_ != nullptr) {
          low->right_child_->parent_ = low;
        }
        if (high->right_child_ != nullptr) {
          high->right_child_->parent_ = high;
        }
        if (low->left_child_ != nullptr) {
          low->left_child_->parent_ = low;
        }
        if (high->left_child_ != nullptr) {
          high->left_child_->parent_ = high;
        }
      }
    }

    friend class map;
  };

  Node *root_;
  Node *sentinar_ = new Node();
  int nodes_num_;

public:
  map() {
    root_ = nullptr;
    nodes_num_ = 0;
  }

  /*copy the nodes recursively*/
  Node *copy(Node *root, Node *other) {
    root = new Node(*(other->content_));
    root->color_ = other->color_;
    if (other->left_child_ != nullptr) {
      root->left_child_ = copy(root->left_child_, other->left_child_);
      root->left_child_->parent_ = root;
    }
    if (other->right_child_ != nullptr) {
      root->right_child_ = copy(root->right_child_, other->right_child_);
      root->right_child_->parent_ = root;
    }
    return root;
  }

  map(const map &other) {
    nodes_num_ = other.nodes_num_;
    root_ = nullptr;
    if (other.nodes_num_ != 0) {
      root_ = copy(root_, other.root_);
    }
  }

  bool empty() const { return nodes_num_ == 0; }

  size_t size() const { return nodes_num_; }

  void erase(Node *root) {
    if (root == nullptr) {
      return;
    }
    erase(root->left_child_);
    erase(root->right_child_);
    delete root;
  }

  void clear() {
    if (nodes_num_ != 0) {
      erase(root_);
    }
    root_ = nullptr;
    nodes_num_ = 0;
  }

  ~map() {
    erase(root_);
    root_ = nullptr;
    delete sentinar_;
  }

  map &operator=(const map &other) {
    if (root_ == other.root_) {
      return *this;
    }
    erase(root_);
    root_ = nullptr;
    nodes_num_ = other.nodes_num_;
    root_ = copy(root_, other.root_);
    return *this;
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent
   * to key. If no such element exists, an exception of type
   * `index_out_of_bound'
   */
  Node *search(const Key &key) const {
    if (root_ == nullptr) {
      return nullptr;
    }
    Node *target = root_;
    Node *parent = nullptr;
    while (target != nullptr) {
      if (!(Compare{}(target->content_->first, key) ||
            Compare{}(key, target->content_->first))) {
        return target;
      }
      if (Compare{}(key, target->content_->first)) {
        parent = target;
        target = target->left_child_;
        continue;
      } else {
        parent = target;
        target = target->right_child_;
        continue;
      }
    }
    return parent;
  }

  T &at(const Key &key) {
    Node *place = search(key);
    if (place != nullptr && !(Compare{}(place->content_->first, key) ||
                              Compare{}(key, place->content_->first))) {
      return place->content_->second;
    }
    throw index_out_of_bound();
    return place->content_->second;
  }

  const T &at(const Key &key) const {
    Node *place = search(key);
    if (place != nullptr && !(Compare{}(place->content_->first, key) ||
                              Compare{}(key, place->content_->first))) {
      return place->content_->second;
    }
    throw index_out_of_bound();
    return place->content_->second;
  }

  /*
  access specified element

  Returns a reference to the value that is mapped to a key equivalent to key,
  performing an insertion if such key does not already exist.
  */
  void insert_maintain(Node *target) {
    Node *parent = nullptr;
    Node *grandparent = nullptr;
    Node *uncle = nullptr;
    while (target != root_) {
      if (target->parent_->color_ == black) {
        break;
      }
      /*If the parent is red, the grandparent(if existed) must black and
      there will be two cases for analysis:
        1. The uncle is black, which is equivlant to the target is inserted in a
      2-item B-Tree node, resulting in rotations and repainting to make a 3-item
      B-Tree Node.
        2. The uncle is red, which means that the target inserting in a 3-item
      full B-Tree node, resulting in repainting equals to a split.*/
      parent = target->parent_;
      grandparent = parent->parent_;
      if (parent == grandparent->left_child_) {
        uncle = grandparent->right_child_;
      } else {
        uncle = grandparent->left_child_;
      }
      if (uncle != nullptr && uncle->color_ == red) {
        parent->color_ = black;
        uncle->color_ = black;
        grandparent->color_ = red;
        target = grandparent;
      } else {
        if (grandparent->left_child_ == parent) {
          if (parent->right_child_ == target) {
            Node *temp = parent;
            parent = target->left_rotation(parent, target);
            target = temp;
          }
          parent->color_ = black;
          grandparent->color_ = red;
          parent->right_rotation(grandparent, parent);
        } else {
          if (parent->left_child_ == target) {
            Node *temp = parent;
            parent = target->right_rotation(parent, target);
            target = temp;
          }
          parent->color_ = black;
          grandparent->color_ = red;
          parent->left_rotation(grandparent, parent);
        }
        break;
      }
    }
    while (root_->parent_ != nullptr) {
      root_ = root_->parent_;
    }
    root_->color_ = black;
  }

  T &operator[](const Key &key) {
    Node *place = search(key);
    if (place == nullptr) {
      ++nodes_num_;
      value_type blank(key, T());
      root_ = new Node(blank);
      return root_->content_->second;
    }
    if (!(Compare{}(place->content_->first, key) ||
          Compare{}(key, place->content_->first))) {
      return place->content_->second;
    }
    ++nodes_num_;
    value_type blank(key, T());
    Node *target = new Node(blank);
    if (Compare{}(key, place->content_->first)) {
      target->color_ = red;
      target->parent_ = place;
      place->left_child_ = target;
    } else {
      target->color_ = red;
      target->parent_ = place;
      place->right_child_ = target;
    }
    insert_maintain(target);
    return target->content_->second;
  }

  /*behave like at() throw index_out_of_bound if such key does not exist.*/
  const T &operator[](const Key &key) const { return at(key); }

  Node *getmin() const {
    if (nodes_num_ == 0) {
      return sentinar_;
    }
    Node *target = root_;
    while (target->left_child_ != nullptr) {
      target = target->left_child_;
    }
    return target;
  }

  Node *getmax() const {
    Node *target = root_;
    while (target->right_child_ != nullptr) {
      target = target->right_child_;
    }
    return target;
  }

  Node *predecessor(const Node *base) const {
    Node *target = const_cast<Node *>(base);
    if (target->left_child_ != nullptr) {
      target = target->left_child_;
      while (target->right_child_ != nullptr) {
        target = target->right_child_;
      }
      return target;
    }
    while (target != root_) {
      if (target->parent_->right_child_ == target) {
        return target->parent_;
      }
      target = target->parent_;
    }
    return nullptr;
  }

  Node *successor(const Node *base) const {
    Node *target = const_cast<Node *>(base);
    if (target->right_child_ != nullptr) {
      target = target->right_child_;
      while (target->left_child_ != nullptr) {
        target = target->left_child_;
      }
      return target;
    }
    while (target != root_) {
      if (target->parent_->left_child_ == target) {
        return target->parent_;
      }
      target = target->parent_;
    }
    return nullptr;
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    Node *place = search(key);
    if (place == nullptr) {
      return 0;
    }
    return !(Compare{}(place->content_->first, key) ||
             Compare{}(key, place->content_->first));
  }

  void traverse(Node *root) {
    if (root == nullptr) {
      return;
    }
    traverse(root->left_child_);
    std::cout << root->content_->second << '\n';
    traverse(root->right_child_);
  }

  bool traver0() {
    // traverse(root_);
    int c = 0;
    if (!checkblack(root_, c, 0)) {
      return 0;
    }
    return 1;
  }

  bool checkblack(Node *root, int &standard, int now) {
    if (root == nullptr) {
      if (standard == 0) {
        standard = now;
      } else {
        if (standard != now) {
          return 0;
        }
      }
      return 1;
    }
    bool left = 0, right = 0;
    if (root->left_child_ == nullptr || root->left_child_->color_ == red) {
      left = checkblack(root->left_child_, standard, now);
    } else {
      left = checkblack(root->left_child_, standard, now + 1);
    }
    if (root->right_child_ == nullptr || root->right_child_->color_ == red) {
      right = checkblack(root->right_child_, standard, now);
    } else {
      right = checkblack(root->right_child_, standard, now + 1);
    }
    return left & right;
  }

  class iterator {
  private:
    friend class map;
    const map *it_;
    Node *at_;

  public:
    iterator() {
      it_ = nullptr;
      at_ = nullptr;
    }
    iterator(const map *it, Node *at) {
      it_ = it;
      at_ = at;
    }
    iterator(const iterator &other) {
      it_ = other.it_;
      at_ = other.at_;
    }

    iterator operator++(int) {
      iterator temp(*this);
      if (at_ == nullptr || at_ == it_->sentinar_) {
        throw invalid_iterator();
      }
      if (at_ == it_->getmax()) {
        at_ = it_->sentinar_;
      } else {
        at_ = it_->successor(at_);
      }
      return temp;
    }
    iterator &operator++() {
      if (at_ == nullptr || at_ == it_->sentinar_) {
        throw invalid_iterator();
      }
      if (at_ == it_->getmax()) {
        at_ = it_->sentinar_;
      } else {
        at_ = it_->successor(at_);
      }
      return *this;
    }
    iterator operator--(int) {
      iterator temp(*this);
      if (at_ == nullptr || at_ == it_->getmin()) {
        throw invalid_iterator();
      }
      if (at_ == it_->sentinar_) {
        at_ = it_->getmax();
      } else {
        at_ = it_->predecessor(at_);
      }
      return temp;
    }
    iterator &operator--() {
      if (at_ == nullptr || at_ == it_->getmin()) {
        throw invalid_iterator();
      }
      if (at_ == it_->sentinar_) {
        at_ = it_->getmax();
      } else {
        at_ = it_->predecessor(at_);
      }
      return *this;
    }

    value_type &operator*() const { return *(at_->content_); }
    value_type *operator->() const noexcept { return at_->content_; }

    bool operator==(const iterator &rhs) const {
      return it_ == rhs.it_ && at_ == rhs.at_;
    }
    bool operator==(const const_iterator &rhs) const {
      return it_ == rhs.it_ && at_ == rhs.at_;
    }
    bool operator!=(const iterator &rhs) const {
      return !(it_ == rhs.it_ && at_ == rhs.at_);
    }
    bool operator!=(const const_iterator &rhs) const {
      return !(it_ == rhs.it_ && at_ == rhs.at_);
    }
  };

  class const_iterator {
  private:
    friend class map;
    const map *it_;
    const Node *at_;

  public:
    const_iterator() {
      it_ = nullptr;
      at_ = nullptr;
    }
    const_iterator(const map *it, const Node *at) {
      it_ = it;
      at_ = at;
    }
    const_iterator(const const_iterator &other) {
      it_ = other.it_;
      at_ = other.at_;
    }
    const_iterator(const iterator &other) {
      it_ = other.it_;
      at_ = other.at_;
    }

    const_iterator operator++(int) {
      const_iterator temp(*this);
      if (at_ == nullptr || at_ == it_->sentinar_) {
        throw invalid_iterator();
      }
      if (at_ == it_->getmax()) {
        at_ = it_->sentinar_;
      } else {
        at_ = it_->successor(at_);
      }
      return temp;
    }
    const_iterator &operator++() {
      if (at_ == nullptr || at_ == it_->sentinar_) {
        throw invalid_iterator();
      }
      if (at_ == it_->getmax()) {
        at_ = it_->sentinar_;
      } else {
        at_ = it_->successor(at_);
      }
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator temp(*this);
      if (at_ == nullptr || at_ == it_->getmin()) {
        throw invalid_iterator();
      }
      if (at_ == it_->sentinar_) {
        at_ = it_->getmax();
      } else {
        at_ = it_->predecessor(at_);
      }
      return temp;
    }
    const_iterator &operator--() {
      if (at_ == nullptr || at_ == it_->getmin()) {
        throw invalid_iterator();
      }
      if (at_ == it_->sentinar_) {
        at_ = it_->getmax();
      } else {
        at_ = it_->predecessor(at_);
      }
      return *this;
    }

    value_type &operator*() const { return *(at_->content_); }
    value_type *operator->() const noexcept { return at_->content_; }

    bool operator==(const iterator &rhs) const {
      return it_ == rhs.it_ && at_ == rhs.at_;
    }
    bool operator==(const const_iterator &rhs) const {
      return it_ == rhs.it_ && at_ == rhs.at_;
    }
    bool operator!=(const iterator &rhs) const {
      return !(it_ == rhs.it_ && at_ == rhs.at_);
    }
    bool operator!=(const const_iterator &rhs) const {
      return !(it_ == rhs.it_ && at_ == rhs.at_);
    }
  };

  iterator begin() { return iterator(this, getmin()); }
  const_iterator cbegin() const { return const_iterator(this, getmin()); }
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() { return iterator(this, sentinar_); }
  const_iterator cend() const { return const_iterator(this, sentinar_); }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key &key) {
    Node *target = search(key);
    if (target == nullptr || (Compare{}(target->content_->first, key) ||
                              Compare{}(key, target->content_->first))) {
      return end();
    }
    return iterator(this, target);
  }
  const_iterator find(const Key &key) const {
    Node *target = search(key);
    if (target == nullptr || (Compare{}(target->content_->first, key) ||
                              Compare{}(key, target->content_->first))) {
      return cend();
    }
    return const_iterator(this, target);
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the
   * insertion), the second one is red if insert successfully, or black.
   */
  pair<iterator, bool> insert(const value_type &value) {
    Node *place = search(value.first);
    if (place == nullptr) {
      ++nodes_num_;
      root_ = new Node(value);
      return pair<iterator, bool>(iterator(this, root_), 1);
    }
    if (!(Compare{}(place->content_->first, value.first) ||
          Compare{}(value.first, place->content_->first))) {
      return pair<iterator, bool>(iterator(this, place), 0);
    }
    ++nodes_num_;
    Node *target = new Node(value);
    if (Compare{}(value.first, place->content_->first)) {
      target->color_ = red;
      target->parent_ = place;
      place->left_child_ = target;
    } else {
      target->color_ = red;
      target->parent_ = place;
      place->right_child_ = target;
    }
    insert_maintain(target);
    return pair<iterator, bool>(iterator(this, target), 1);
  }

  void erase_maintain(Node *target) {
    /*
      If remove a node on the leaf, adjustment of the tree falls in several
    cases:
      1. The target is red, which means that we erase an item from a 2/3 item
    B-Tree node. The only task is to throw it away and change the pointer.
      2. The target is black, which means that we kill a B-Tree node. We need to
    analysis its brother B-Tree node so first we rotate to make its sibling a
    black node that equivalent to a sibling in B-Tree instead of its parent.
      2.1 The brother has at least one red child: make sure the child on the
    opposite direction towards the target is red, then rotate to make the
    sibling uplift and repaint.
      2.2 The sibling has two black child: there is no abundant child of the
    sibling, so the only choice is to merge the node. Rotate the sibling
    upwards, repaint and adjust the tree recursively.
    */
    Node *parent = nullptr;
    Node *sibling = nullptr;
    while (target != root_ && (target == nullptr || target->color_ == black)) {
      parent = target->parent_;
      if (parent->left_child_ == target) {
        sibling = parent->right_child_;
        if (sibling->color_ == red) {
          parent->color_ = red;
          sibling->color_ = black;
          sibling->left_rotation(parent, sibling);
          sibling = parent->right_child_;
        }
        if (sibling->right_child_ != nullptr &&
            sibling->right_child_->color_ == red) {
          sibling->color_ = parent->color_;
          parent->color_ = black;
          sibling->right_child_->color_ = black;
          sibling->left_rotation(parent, sibling);
          break;
        }
        if (sibling->left_child_ != nullptr &&
            sibling->left_child_->color_ == red) {
          sibling = sibling->right_rotation(sibling, sibling->left_child_);
          sibling->color_ = parent->color_;
          parent->color_ = black;
          sibling->right_child_->color_ = black;
          sibling->left_rotation(parent, sibling);
          break;
        }
        if (parent->color_ == red) {
          parent->color_ = black;
          sibling->color_ = red;
          break;
        }
        sibling->color_ = red;
        target = parent;
      } else {
        sibling = parent->left_child_;
        if (sibling->color_ == red) {
          parent->color_ = red;
          sibling->color_ = black;
          sibling->right_rotation(parent, sibling);
          sibling = parent->left_child_;
        }
        if (sibling->left_child_ != nullptr &&
            sibling->left_child_->color_ == red) {
          sibling->color_ = parent->color_;
          parent->color_ = black;
          sibling->left_child_->color_ = black;
          sibling->right_rotation(parent, sibling);
          break;
        }
        if (sibling->right_child_ != nullptr &&
            sibling->right_child_->color_ == red) {
          sibling = sibling->left_rotation(sibling, sibling->right_child_);
          sibling->color_ = parent->color_;
          parent->color_ = black;
          sibling->left_child_->color_ = black;
          sibling->right_rotation(parent, sibling);
          break;
        }
        if (parent->color_ == red) {
          parent->color_ = black;
          sibling->color_ = red;
          break;
        }
        sibling->color_ = red;
        target = parent;
      }
    }
    while (root_->parent_ != nullptr) {
      root_ = root_->parent_;
    }
    root_->color_ = black;
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points
   * an element out of this)
   */
  void erase(iterator pos) {
    if (pos.it_ != this || pos.at_ == nullptr || pos.at_ == sentinar_) {
      throw invalid_iterator();
    }
    if (nodes_num_ <= 1) {
      delete root_;
      root_ = nullptr;
      pos.at_ = nullptr;
      nodes_num_ = 0;
      return;
    }
    /*
      If the node is on the leaf, then we can erase it then maintain the R-B
    characteristic. Otherwise, we need to find its predecessor or successor
    and swap their location, then erase it.
    */
    Node *target = pos.at_;
    if (pos.at_->left_child_ != nullptr) {
      target = predecessor(pos.at_);
      target->swap(pos.at_, target);
      target = pos.at_;
      if (target->left_child_ != nullptr) {
        target->swap(target, target->left_child_);
      }
    } else if (pos.at_->right_child_ != nullptr) {
      target = successor(pos.at_);
      target->swap(pos.at_, target);
      target = pos.at_;
      if (target->right_child_ != nullptr) {
        target->swap(target, target->right_child_);
      }
    }
    erase_maintain(target);
    if (target->parent_->left_child_ == target) {
      target->parent_->left_child_ = nullptr;
    } else {
      target->parent_->right_child_ = nullptr;
    }
    --nodes_num_;
    delete target;
    pos.at_ = nullptr;
    return;
  }
};

} // namespace sjtu

#endif
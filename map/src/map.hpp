/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include "exceptions.hpp"
#include "utility.hpp"
#include <cstddef>
#include <functional>

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key>> class map {
public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef std::pair<const Key, T> value_type;
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

    Node(const value_type &content) { content_ = new value_type(content); }

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
      if (parent_before->parent_->left_child_ == parent_before) {
        parent_before->parent_->left_child_ = parent_after;
      } else {
        parent_before->parent_->right_child_ = parent_after;
      }
      parent_before->right_child_ = parent_after->left_child_;
      parent_after->left_child_->parent_ = parent_before;
      parent_after->left_child_ = parent_before;
      parent_before->parent_ = parent_after;
      return parent_after;
    }

    Node *right_rotation(Node *parent_before, Node *parent_after) {
      if (parent_after == nullptr) {
        throw std::exception();
      }
      parent_after->parent_ = parent_before->parent_;
      if (parent_before->parent_->left_child_ == parent_before) {
        parent_before->parent_->left_child_ = parent_after;
      } else {
        parent_before->parent_->right_child_ = parent_after;
      }
      parent_before->left_child_ = parent_after->right_child_;
      parent_after->right_child_->parent_ = parent_before;
      parent_after->right_child_ = parent_before;
      parent_before->parent_ = parent_after;
      return parent_after;
    }

    friend class map;
  };

  Node *root_;
  int nodes_num_;

  friend bool operator<(Key &lhs, Key &rhs) { return Compare()(lhs, rhs); }
  friend bool operator>(Key &lhs, Key &rhs) { return Compare()(rhs, lhs); }
  friend bool operator==(Key &lhs, Key &rhs) {
    return !(lhs < rhs || rhs < lhs);
  }

public:
  map() {
    root_ = nullptr;
    nodes_num_ = 0;
  }

  /*copy the nodes recursively*/
  Node *copy(Node *root, Node *other) {
    root = new Node(*(other->content));
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
    copy(root_);
  }

  bool empty() const { return nodes_num_ == 0; }

  size_t size() const { return nodes_num_; }

  void erase(Node *root) {
    if (root->left_child_ != nullptr) {
      erase(root->left_child_);
      delete root->left_child_;
    }
    if (root->right_child_ != nullptr) {
      erase(root->right_child_);
      delete root->right_child_;
    }
    delete root;
    return;
  }

  void clear() {
    erase(root_);
    nodes_num_ = 0;
  }

  ~map() { erase(root_); }

  map &operator=(const map &other) {
    if (root_ == other.root_) {
      return *this;
    }
    erase(root_);
    nodes_num_ = other.nodes_num_;
    copy(root_);
    return *this;
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent
   * to key. If no such element exists, an exception of type
   * `index_out_of_bound'
   */
  Node *search(const Key &key) {
    if (root_ == nullptr) {
      return nullptr;
    }
    Node *target = root_;
    Node *parent = nullptr;
    while (target != nullptr) {
      if (target->content_->first == key) {
        return target;
      }
      if (target->content_->first > key) {
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
    if (place != nullptr && place->content_->first == key) {
      return place->content_->second;
    }
    throw index_out_of_bound();
    return NULL;
  }

  const T &at(const Key &key) const {
    Node *place = search(key);
    if (place != nullptr && place->content_->first == key) {
      return place->content_->second;
    }
    throw std::exception();
    return NULL;
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
      if (target->parent->color_ == black) {
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
      grandparent =
          parent->parent_; // The root is black before repaint in maintain.
      if (grandparent->left_child_ == parent) {
        uncle = grandparent->right_child_;
      } else {
        uncle = grandparent->left_child_;
      }
      if (!(grandparent->left_child_ == parent &&
            parent->left_child_ == target) &&
          !(grandparent->right_child_ == parent &&
            parent->right_child_ == target)) {
        Node *temp = parent;
        if (parent->right_child_ == target) {
          parent = target->left_rotation(parent, target);
          target = temp;
        } else {
          parent = target->right_rotation(parent, target);
          target = temp;
        }
      }
      if (uncle->color_ == black) {
        if (grandparent->left_child_ == parent) {
          target = parent->right_rotation(grandparent, parent);
        } else {
          target = parent->left_rotation(grandparent, parent);
        }
        target->color_ = black;
        parent->color_ = red;
      } else {
        parent->color_ = black;
        grandparent->color_ = black;
        break;
      }
    }
    root_->color_ = black;
  }

  T &operator[](const Key &key) {
    Node *place = search(key);
    if (place == nullptr) {
      ++nodes_num_;
      value_type blank;
      root_ = new Node(blank);
      return NULL;
    }
    if (place->content_->first == key) {
      return place->content_->second;
    }
    ++nodes_num_;
    value_type blank;
    blank.first = key;
    Node *target = new Node(blank);
    if (place->content_->first > key) {
      target->color_ = red;
      target->parent_ = place;
      place->left_child_ = target;
    } else {
      target->color_ = red;
      target->parent_ = place;
      place->right_child_ = target;
    }
    insert_maintain(target);
    return NULL;
  }

  /*behave like at() throw index_out_of_bound if such key does not exist.*/
  const T &operator[](const Key &key) const { return at(key); }

  Node *predecessor(Node *base) {
    Node *target = base;
    if (target->left_child_ == nullptr) {
      while (target != root_ && target->parent_->right_child_ == target) {
        target = target->parent_;
      }
    } else {
      target = target->left_child_;
      while (target->right_child_ != nullptr) {
        target = target->right_child_;
      }
    }
    if (base == target) {
      return nullptr;
    }
    return target;
  }

  Node *successor(Node *base) {
    Node *target = base;
    if (target->right_child_ == nullptr) {
      while (target != root_ && target->parent_->left_child_ == target) {
        target = target->parent_;
      }
    } else {
      target = target->right_child_;
      while (target->left_child_ != nullptr) {
        target = target->left_child_;
      }
    }
    if (base == target) {
      return nullptr;
    }
    return target;
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
    return place->content_->first == key;
  }

  class iterator {
  private:
    friend class map;
    map *it_;
    Node *at_;

  public:
    iterator() {
      it_ = nullptr;
      at_ = nullptr;
    }
    iterator(const iterator &other) {
      it_ = other.it_;
      at_ = other.content_;
    }

    iterator operator++(int) {
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      iterator temp(*this);
      at_ = it_->successor(at_);
      return temp;
    }
    iterator &operator++() {
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      at_ = it_->successor(at_);
      return *this;
    }
    iterator operator--(int) {
      iterator temp(*this);
      at_ = it_->predecessor(at_);
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      return temp;
    }
    iterator &operator--() {
      at_ = it_->predecessor(at_);
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      return *this;
    }

    value_type &operator*() const { return at_->content_; }
    value_type *operator->() const noexcept { return at_->content_; }

    bool operator==(const iterator &rhs) const { return at_ == rhs.at_; }
    bool operator==(const const_iterator &rhs) const { return at_ == rhs.at_; }
    bool operator!=(const iterator &rhs) const { return !at_ == rhs.at_; }
    bool operator!=(const const_iterator &rhs) const { return !at_ == rhs.at_; }
  };

  class const_iterator {
  private:
    friend class map;
    map *it_;
    Node *at_;

  public:
    const_iterator() {
      it_ = nullptr;
      at_ = nullptr;
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
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      const_iterator temp(*this);
      at_ = it_->successor(at_);
      return temp;
    }
    const_iterator &operator++() {
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      at_ = it_->successor(at_);
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator temp(*this);
      at_ = it_->predecessor(at_);
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      return temp;
    }
    const_iterator &operator--() {
      at_ = it_->predecessor(at_);
      if (at_ == nullptr) {
        throw invalid_iterator();
      }
      return *this;
    }

    value_type &operator*() const { return at_->content_; }
    value_type *operator->() const noexcept { return at_->content_; }

    bool operator==(const iterator &rhs) const { return at_ == rhs.at_; }
    bool operator==(const const_iterator &rhs) const { return at_ == rhs.at_; }
    bool operator!=(const iterator &rhs) const { return !at_ == rhs.at_; }
    bool operator!=(const const_iterator &rhs) const { return !at_ == rhs.at_; }
  };

  iterator begin() {
    Node *target = root_;
    while (target->left_child_ != nullptr) {
      target = target->left_child_;
    }
    return iterator(target);
  }
  const_iterator cbegin() const {
    Node *target = root_;
    while (target->left_child_ != nullptr) {
      target = target->left_child_;
    }
    return const_iterator(target);
  }
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() { return iterator(nullptr); }
  const_iterator cend() const { return const_iterator(nullptr); }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key &key) {
    Node *target = search(key);
    if (target == nullptr || target->content_->first != key) {
      return nullptr;
    }
    return iterator(target);
  }
  const_iterator find(const Key &key) const {
    Node *target = search(key);
    if (target == nullptr || target->content_->first != key) {
      return nullptr;
    }
    return const_iterator(target);
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the
   * insertion), the second one is red if insert successfully, or black.
   */
  std::pair<iterator, bool> insert(const value_type &value) {
    Node *place = search(value.first);
    if (place == nullptr) {
      ++nodes_num_;
      root_ = new Node(value);
      return std::pair<iterator, bool>(root_, 1);
    }
    if (place->content_->first == value.first) {
      return std::pair<iterator, bool>(place, 0);
    }
    ++nodes_num_;
    Node *target = new Node(value);
    if (place->content_->first > value.first) {
      target->color_ = red;
      target->parent_ = place;
      place->left_child_ = target;
    } else {
      target->color_ = red;
      target->parent_ = place;
      place->right_child_ = target;
    }
    insert_maintain(target);
    return std::pair<iterator, bool>(target, 1);
  }

  void erase_maintain(Node *target) {
    /*
      After remove a node on the leaf, adjustment of the tree falls in several
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
    bool record_direction = 0;
    while (target != root_) {
      if (target->color_ == red) {
        break;
      }
      parent = target->parent_;
      if (parent->left_child_ == target) {
        sibling = parent->right_child_;
        record_direction = 1;
      } else {
        sibling = parent->left_child_;
        record_direction = 0;
      }
      if (sibling->color_ == red) {
        if (record_direction == 1) {
          parent = sibling->left_rotation(parent, sibling)->left_child_;
          parent->color_ = red;
          parent->parent_->color_ = black;
          sibling = parent->right_child_;
        } else {
          parent = sibling->right_rotation(parent, sibling)->right_child_;
          parent->color_ = red;
          parent->parent_->color_ = black;
          sibling = parent->left_child_;
        }
      }
      if (sibling->left_child_->color_ == black &&
          sibling->right_child_->color_ == black) {
        if (record_direction == 1) {
          target = sibling->left_rotation(parent, sibling);
          target->color_ = black;
          target->left_child_->color_ = red;
        } else {
          target = sibling->right_rotation(parent, sibling);
          target->color_ = black;
          target->right_child_->color_ = red;
        }
      } else {
        if (record_direction == 1) {
          if (sibling->right_child_->color_ == black) {
            sibling = sibling->right_rotation(sibling, sibling->left_child_);
            sibling->color_ = black;
            sibling->right_child_->color_ = red;
          }
          sibling->left_rotation(parent, sibling);
          sibling->color_ = parent->color_;
          parent->color_ = black;
        } else {
          if (sibling->left_child_->color_ == black) {
            sibling = sibling->left_rotation(sibling, sibling->right_child_);
            sibling->color_ = black;
            sibling->left_child_->color_ = red;
          }
          sibling->right_rotation(parent, sibling);
          sibling->color_ = parent->color_;
          parent->color_ = black;
        }
        break;
      }
    }
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points
   * an element out of this)
   */
  void erase(iterator pos) {
    /*
      If the node is on the leaf, then we can erase it then maintain the R-B
    characteristic. Otherwise, we need to find its predecessor or successor
    and swap their location, then erase it.
    */
    if (pos.content->left_child_ != nullptr) {
      Node *target = predecessor(pos.content);
      swap(target, pos.content);
      erase_maintain(pos.content);
      erase(pos.content);
    } else if (pos.content->right_child_ != nullptr) {
      Node *target = successor(pos.content);
      swap(target, pos.content);
      erase_maintain(pos.content);
      erase(pos.content);
    } else {
      erase_maintain(pos.content);
      erase(pos.content);
    }
    return;
  }
};
} // namespace sjtu

#endif
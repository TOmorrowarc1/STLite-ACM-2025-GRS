#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include <new>

#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for
 * certain data. In such cases, any ongoing operation should be terminated, and
 * the priority queue should be restored to its original state before the
 * operation began.
 */
template <typename T, class Compare = std::less<T>>
class priority_queue {
   private:
    struct Node {
        Node* left_child_;
        Node* right_child_;
        T* content_;
        int distance_;

        Node() {
            left_child_ = nullptr;
            right_child_ = nullptr;
            content_ = (T*)operator new(sizeof(T));
            distance_ = 0;
        }

        ~Node() {
            left_child_ = right_child_ = nullptr;
            operator delete(content_, sizeof(T));
        }

        bool operator<(const Node& rhs) {
            return Compare{}(*content_, *rhs.content_);
        }

        void swap_child() {
            Node* temp = left_child_;
            left_child_ = right_child_;
            right_child_ = temp;
            return;
        }
    };

    Node* root_;
    int node_num_;

   public:
    /**
     * @brief default constructor
     */
    priority_queue() {
        root_ = nullptr;
        node_num_ = 0;
    }

    /**
     * @brief copy constructor
     * @param other the priority_queue to be copied
     */
    Node* copy(Node* des, Node* src) {
        new (des->content_) T(*(src->content_));
        if (src->left_child_ != nullptr) {
            des->left_child_ = new Node();
            copy(des->left_child_, src->left_child_);
        }
        if (src->right_child_ != nullptr) {
            des->right_child_ = new Node();
            copy(des->right_child_, src->right_child_);
        }
        if (des->right_child_ == nullptr) {
            des->distance_ = 0;
        } else {
            des->distance_ = des->right_child_->distance_ + 1;
        }
        return des;
    }

    priority_queue(const priority_queue& other) {
        root_ = new Node();
        copy(root_, other.root_);
        node_num_ = other.node_num_;
    }

    /*
    erase(node*):a tool to erase the node and its subtree recursivly.
    */

    void erase(Node* root) {
        if (root == nullptr) {
            return;
        }
        --node_num_;
        erase(root->left_child_);
        erase(root->right_child_);
        root->content_->~T();
        delete root;
        return;
    }

    ~priority_queue() {
        erase(root_);
    }

    /**
     * @brief Assignment operator
     * @param other the priority_queue to be assigned from
     * @return a reference to this priority_queue after assignment
     */
    priority_queue& operator=(const priority_queue& other) {
        if (root_ == other.root_) {
            return *this;
        }
        erase(root_);
        root_ = new Node();
        copy(root_, other.root_);
        node_num_ = other.node_num_;
        return *this;
    }

    /**
     * @brief get the top element of the priority queue.
     * @return a reference of the top element.
     * @throws container_is_empty if empty() returns true
     */
    const T& top() const {
        if (node_num_ == 0) {
            throw container_is_empty();
        }
        return *(root_->content_);
    }

    /**
     * @brief merge another priority_queue into this one.
     * The other priority_queue will be cleared after merging.
     * The complexity is at most O(logn).
     * @param other the priority_queue to be merged.
     */

    /*
    The merge of two nodes and their subtree,return the root_ of the left_slide
    heap after the operation.
    */
    Node* merge_two(Node* lhs, Node* rhs) {
        if (lhs == rhs) {
            return lhs;
        }
        if (lhs == nullptr) {
            return rhs;
        }
        if (rhs == nullptr) {
            return lhs;
        }
        bool flag = 0;
        try {
            flag = *rhs < *lhs;
        } catch (sjtu::runtime_error) {
            return lhs;
        }
        if (flag == true) {
            lhs->right_child_ = merge_two(rhs, lhs->right_child_);
            if (lhs->left_child_ == nullptr ||
                lhs->right_child_->distance_ > lhs->left_child_->distance_) {
                lhs->swap_child();
            }
            if (lhs->right_child_ == nullptr) {
                lhs->distance_ = 0;
            } else {
                lhs->distance_ = lhs->right_child_->distance_ + 1;
            }
            return lhs;
        }
        rhs->right_child_ = merge_two(lhs, rhs->right_child_);
        if (rhs->left_child_ == nullptr ||
            rhs->right_child_->distance_ > rhs->left_child_->distance_) {
            rhs->swap_child();
        }
        if (rhs->right_child_ == nullptr) {
            rhs->distance_ = 0;
        } else {
            rhs->distance_ = rhs->right_child_->distance_ + 1;
        }
        return rhs;
    }

    void merge(priority_queue& other) {
        root_ = merge_two(root_, other.root_);
        node_num_ += other.node_num_;
        return;
    }

    /**
     * @brief push new element to the priority queue.
     * @param e the element to be pushed
     */
    void push(const T& e) {
        Node* new_node = new Node();
        new (new_node->content_) T(e);
        if (node_num_ == 0) {
            root_ = new_node;
        } else {
            root_ = merge_two(root_, new_node);
        }
        ++node_num_;
        return;
    }

    /**
     * @brief delete the top element from the priority queue.
     * @throws container_is_empty if empty() returns true
     */
    void pop() {
        if (node_num_ == 0) {
            throw container_is_empty();
        }
        Node* temp = merge_two(root_->left_child_, root_->right_child_);
        root_->content_->~T();
        delete root_;
        root_ = temp;
        --node_num_;
        return;
    }

    /**
     * @brief return the number of elements in the priority queue.
     * @return the number of elements.
     */
    int size() const {
        return node_num_;
    }

    /**
     * @brief check if the container is empty.
     * @return true if it is empty, false otherwise.
     */
    bool empty() const {
        return node_num_ == 0;
    }
};

}  // namespace sjtu

#endif
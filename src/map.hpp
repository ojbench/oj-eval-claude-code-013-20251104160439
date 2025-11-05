/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include <iostream>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;

  private:
    enum Color { RED, BLACK };

    struct Node {
        value_type *data;
        Node *left, *right, *parent;
        Color color;

        Node() : data(nullptr), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}
        Node(const value_type &val, Node *p = nullptr)
            : data(new value_type(val)), left(nullptr), right(nullptr), parent(p), color(RED) {}
        ~Node() { delete data; }
    };

    Node *root;
    size_t node_count;
    Compare comp;

    Node *getGrandparent(Node *node) {
        if (node != nullptr && node->parent != nullptr)
            return node->parent->parent;
        return nullptr;
    }

    Node *getUncle(Node *node) {
        Node *grandparent = getGrandparent(node);
        if (grandparent == nullptr) return nullptr;
        if (node->parent == grandparent->left)
            return grandparent->right;
        else
            return grandparent->left;
    }

    void rotateLeft(Node *node) {
        Node *new_parent = node->right;
        if (new_parent == nullptr) return;

        node->right = new_parent->left;
        if (new_parent->left != nullptr)
            new_parent->left->parent = node;

        new_parent->parent = node->parent;
        if (node->parent == nullptr)
            root = new_parent;
        else if (node == node->parent->left)
            node->parent->left = new_parent;
        else
            node->parent->right = new_parent;

        new_parent->left = node;
        node->parent = new_parent;
    }

    void rotateRight(Node *node) {
        Node *new_parent = node->left;
        if (new_parent == nullptr) return;

        node->left = new_parent->right;
        if (new_parent->right != nullptr)
            new_parent->right->parent = node;

        new_parent->parent = node->parent;
        if (node->parent == nullptr)
            root = new_parent;
        else if (node == node->parent->right)
            node->parent->right = new_parent;
        else
            node->parent->left = new_parent;

        new_parent->right = node;
        node->parent = new_parent;
    }

    void insertFixup(Node *node) {
        while (node != root && node->parent->color == RED) {
            Node *uncle = getUncle(node);
            if (uncle != nullptr && uncle->color == RED) {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                Node *grandparent = getGrandparent(node);
                grandparent->color = RED;
                node = grandparent;
            } else {
                Node *parent = node->parent;
                Node *grandparent = getGrandparent(node);
                if (node == parent->right && parent == grandparent->left) {
                    rotateLeft(parent);
                    node = parent;
                    parent = node->parent;
                } else if (node == parent->left && parent == grandparent->right) {
                    rotateRight(parent);
                    node = parent;
                    parent = node->parent;
                }

                parent->color = BLACK;
                grandparent->color = RED;
                if (node == parent->left)
                    rotateRight(grandparent);
                else
                    rotateLeft(grandparent);
            }
        }
        root->color = BLACK;
    }

    Node *findNode(const Key &key) const {
        Node *current = root;
        while (current != nullptr) {
            if (comp(key, current->data->first))
                current = current->left;
            else if (comp(current->data->first, key))
                current = current->right;
            else
                return current;
        }
        return nullptr;
    }

    Node *findMin(Node *node) const {
        while (node->left != nullptr)
            node = node->left;
        return node;
    }

    Node *findMax(Node *node) const {
        while (node->right != nullptr)
            node = node->right;
        return node;
    }

    Node *successor(Node *node) const {
        if (node->right != nullptr)
            return findMin(node->right);

        Node *parent = node->parent;
        while (parent != nullptr && node == parent->right) {
            node = parent;
            parent = parent->parent;
        }
        return parent;
    }

    Node *predecessor(Node *node) const {
        if (node->left != nullptr)
            return findMax(node->left);

        Node *parent = node->parent;
        while (parent != nullptr && node == parent->left) {
            node = parent;
            parent = parent->parent;
        }
        return parent;
    }

    void destroyTree(Node *node) {
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

    Node *copyTree(Node *node, Node *parent = nullptr) {
        if (node == nullptr) return nullptr;

        Node *new_node = new Node(*node->data, parent);
        new_node->color = node->color;
        new_node->left = copyTree(node->left, new_node);
        new_node->right = copyTree(node->right, new_node);
        return new_node;
    }

  public:
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
      private:
       Node *node_ptr;
       const map *map_ptr;

       friend class map;
       friend class const_iterator;

       iterator(Node *node, const map *m) : node_ptr(node), map_ptr(m) {}

      public:
       iterator() : node_ptr(nullptr), map_ptr(nullptr) {}

       iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

       iterator operator++(int) {
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       iterator &operator++() {
           if (node_ptr == nullptr) throw invalid_iterator();
           node_ptr = map_ptr->successor(node_ptr);
           return *this;
       }

       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       iterator &operator--() {
           if (node_ptr == nullptr) {
               if (map_ptr->root == nullptr) throw invalid_iterator();
               node_ptr = map_ptr->findMax(map_ptr->root);
           } else {
               node_ptr = map_ptr->predecessor(node_ptr);
           }
           return *this;
       }

       value_type &operator*() const {
           if (node_ptr == nullptr || node_ptr->data == nullptr) throw invalid_iterator();
           return *(node_ptr->data);
       }

       bool operator==(const iterator &rhs) const {
           return node_ptr == rhs.node_ptr;
       }

       bool operator==(const const_iterator &rhs) const {
           return node_ptr == rhs.node_ptr;
       }

       bool operator!=(const iterator &rhs) const {
           return node_ptr != rhs.node_ptr;
       }

       bool operator!=(const const_iterator &rhs) const {
           return node_ptr != rhs.node_ptr;
       }

       value_type *operator->() const noexcept {
           if (node_ptr == nullptr || node_ptr->data == nullptr) return nullptr;
           return node_ptr->data;
       }
   };

   class const_iterator {
      private:
       const Node *node_ptr;
       const map *map_ptr;

       friend class map;

       const_iterator(const Node *node, const map *m) : node_ptr(node), map_ptr(m) {}

      public:
       const_iterator() : node_ptr(nullptr), map_ptr(nullptr) {}

       const_iterator(const const_iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

       const_iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}

       const_iterator operator++(int) {
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator++() {
           if (node_ptr == nullptr) throw invalid_iterator();
           node_ptr = map_ptr->successor(const_cast<Node*>(node_ptr));
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (node_ptr == nullptr) {
               if (map_ptr->root == nullptr) throw invalid_iterator();
               node_ptr = map_ptr->findMax(const_cast<Node*>(map_ptr->root));
           } else {
               node_ptr = map_ptr->predecessor(const_cast<Node*>(node_ptr));
           }
           return *this;
       }

       const value_type &operator*() const {
           if (node_ptr == nullptr || node_ptr->data == nullptr) throw invalid_iterator();
           return *(node_ptr->data);
       }

       bool operator==(const const_iterator &rhs) const {
           return node_ptr == rhs.node_ptr;
       }

       bool operator!=(const const_iterator &rhs) const {
           return node_ptr != rhs.node_ptr;
       }

       const value_type *operator->() const noexcept {
           if (node_ptr == nullptr || node_ptr->data == nullptr) return nullptr;
           return node_ptr->data;
       }
   };

   /**
  * TODO two constructors
    */
   map() : root(nullptr), node_count(0) {}

   map(const map &other) : root(nullptr), node_count(0), comp(other.comp) {
       root = copyTree(other.root);
       node_count = other.node_count;
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this != &other) {
           destroyTree(root);
           root = copyTree(other.root);
           node_count = other.node_count;
           comp = other.comp;
       }
       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() {
       destroyTree(root);
   }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *node = findNode(key);
       if (node == nullptr) throw index_out_of_bound();
       return node->data->second;
   }

   const T &at(const Key &key) const {
       Node *node = findNode(key);
       if (node == nullptr) throw index_out_of_bound();
       return node->data->second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       Node *node = findNode(key);
       if (node != nullptr) return node->data->second;

       // Use the existing insert function which handles the insertion properly
       value_type val(key, T());
       auto result = insert(val);
       return result.first->second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
       return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
       if (root == nullptr) return end();
       return iterator(findMin(root), this);
   }

   const_iterator cbegin() const {
       if (root == nullptr) return cend();
       return const_iterator(findMin(root), this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() {
       return iterator(nullptr, this);
   }

   const_iterator cend() const {
       return const_iterator(nullptr, this);
   }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const {
       return node_count == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
       return node_count;
   }

   /**
  * clears the contents
    */
   void clear() {
       destroyTree(root);
       root = nullptr;
       node_count = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *parent = nullptr;
       Node *current = root;

       while (current != nullptr) {
           parent = current;
           if (comp(value.first, current->data->first))
               current = current->left;
           else if (comp(current->data->first, value.first))
               current = current->right;
           else
               return pair<iterator, bool>(iterator(current, this), false);
       }

       Node *new_node = new Node(value, parent);
       if (parent == nullptr) {
           root = new_node;
           root->color = BLACK;
       } else if (comp(value.first, parent->data->first)) {
           parent->left = new_node;
       } else {
           parent->right = new_node;
       }

       node_count++;
       insertFixup(new_node);
       return pair<iterator, bool>(iterator(new_node, this), true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos.node_ptr == nullptr || pos.map_ptr != this) throw invalid_iterator();

       Node *z = pos.node_ptr;
       Node *y = z;
       Node *x = nullptr;
       Color y_original_color = y->color;

       if (z->left == nullptr) {
           x = z->right;
           transplant(z, z->right);
       } else if (z->right == nullptr) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = findMin(z->right);
           y_original_color = y->color;
           x = y->right;
           if (y->parent == z) {
               if (x != nullptr) x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->color = z->color;
       }

       delete z;
       node_count--;

       if (y_original_color == BLACK)
           deleteFixup(x);
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
       return findNode(key) != nullptr ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
       Node *node = findNode(key);
       return iterator(node, this);
   }

   const_iterator find(const Key &key) const {
       Node *node = findNode(key);
       return const_iterator(node, this);
   }

  private:
   void transplant(Node *u, Node *v) {
       if (u->parent == nullptr) {
           root = v;
       } else if (u == u->parent->left) {
           u->parent->left = v;
       } else {
           u->parent->right = v;
       }
       if (v != nullptr) {
           v->parent = u->parent;
       }
   }

   void deleteFixup(Node *x) {
       while (x != root && (x == nullptr || x->color == BLACK)) {
           if (x == nullptr) {
               x = root;
               break;
           }
           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w->color == RED) {
                   w->color = BLACK;
                   x->parent->color = RED;
                   rotateLeft(x->parent);
                   w = x->parent->right;
               }
               if ((w->left == nullptr || w->left->color == BLACK) &&
                   (w->right == nullptr || w->right->color == BLACK)) {
                   w->color = RED;
                   x = x->parent;
               } else {
                   if (w->right == nullptr || w->right->color == BLACK) {
                       if (w->left != nullptr) w->left->color = BLACK;
                       w->color = RED;
                       rotateRight(w);
                       w = x->parent->right;
                   }
                   w->color = x->parent->color;
                   x->parent->color = BLACK;
                   if (w->right != nullptr) w->right->color = BLACK;
                   rotateLeft(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w->color == RED) {
                   w->color = BLACK;
                   x->parent->color = RED;
                   rotateRight(x->parent);
                   w = x->parent->left;
               }
               if ((w->right == nullptr || w->right->color == BLACK) &&
                   (w->left == nullptr || w->left->color == BLACK)) {
                   w->color = RED;
                   x = x->parent;
               } else {
                   if (w->left == nullptr || w->left->color == BLACK) {
                       if (w->right != nullptr) w->right->color = BLACK;
                       w->color = RED;
                       rotateLeft(w);
                       w = x->parent->left;
                   }
                   w->color = x->parent->color;
                   x->parent->color = BLACK;
                   if (w->left != nullptr) w->left->color = BLACK;
                   rotateRight(x->parent);
                   x = root;
               }
           }
       }
       if (x != nullptr) x->color = BLACK;
   }
};

}

#endif
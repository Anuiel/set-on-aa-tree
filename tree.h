#include <initializer_list>
#include <cstddef>

template<class ValueType>
class Set {
private:
    static size_t const ONE = 1;

    struct Node {
        ValueType value;
        Node* left;
        Node* right;
        Node* parent;
        size_t level;

        Node() : left(nullptr), right(nullptr), parent(nullptr), level(ONE) {}

        Node(const ValueType& value, Node* left, Node* right, Node* parent, size_t level = ONE) :
                value(value), left(left), right(right), parent(parent), level(level) {}

    };

    static Node* DeepCopy(Node* node, Node* pred = nullptr) {
        if (node == nullptr) {
            return nullptr;
        }
        auto new_node = new Node(node->value, nullptr, nullptr, pred, node->level);
        new_node->left = DeepCopy(node->left, new_node);
        new_node->right = DeepCopy(node->right, new_node);
        return new_node;
    }

    Node* root_ = nullptr;
    size_t size_ = 0;

public:
    class iterator {
    public:
        iterator() : owner_(nullptr), node_(nullptr) {}

        iterator(const Set* owner, Node* node) : owner_(owner), node_(node) {}

        iterator& operator=(const iterator& other) {
            owner_ = other.owner_;
            node_ = other.node_;
            return *this;
        }

        iterator& operator++() {
            if (node_ == nullptr) {
                return *this;
            }
            if (node_->right != nullptr) {
                node_ = Successor(node_);
                return *this;
            }
            while (node_->parent != nullptr && node_->parent->left != node_) {
                node_ = node_->parent;
            }
            node_ = node_->parent;
            return *this;
        }

        const iterator operator++(int) {
            const iterator it = iterator(owner_, node_);
            ++(*this);
            return it;
        }

        iterator& operator--() {
            if (node_ == nullptr) {
                node_ = owner_->root_;
                while (node_->right != nullptr) {
                    node_ = node_->right;
                }
                return *this;
            }
            if (node_->left != nullptr) {
                node_ = Predicator(node_);
                return *this;
            }
            while (node_->parent != nullptr && node_->parent->right != node_) {
                node_ = node_->parent;
            }
            node_ = node_->parent;
            return *this;
        }

        const iterator operator--(int) {
            const iterator it = iterator(owner_, node_);
            --(*this);
            return it;
        }

        bool operator!=(const iterator& other) const {
            return node_ != other.node_ || owner_ != other.owner_;
        }

        bool operator==(const iterator& other) const {
            return node_ == other.node_ && owner_ == other.owner_;
        }

        const ValueType& operator*() const {
            return node_->value;
        }

        const ValueType* operator->() const {
            return &(node_->value);
        }


    private:
        const Set* owner_ = nullptr;
        Node* node_ = nullptr;
    };

    Set() : root_(nullptr), size_(0) {}

    template<class ClassIterator>
    Set(const ClassIterator begin, const ClassIterator end) {
        root_ = nullptr;
        size_ = 0;
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    Set(const std::initializer_list<ValueType>& list) {
        size_ = 0;
        root_ = nullptr;
        for (ValueType& x: list) {
            insert(x);
        }
    }

    Set(const Set& other) {
        root_ = DeepCopy(other.root_);
        size_ = other.size_;
    }

    Set& operator=(const Set& other) {
        if (this == &other) {
            return *this;
        }
        Clear();
        root_ = DeepCopy(other.root_);
        size_ = other.size_;
        return *this;
    }

    ~Set() {
        Clear();
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void insert(const ValueType& value) {
        bool was_inserted = false;
        root_ = insert(root_, value, was_inserted);
        if (was_inserted) {
            ++size_;
        }
    }

    void erase(const ValueType& value) {
        bool was_erased = false;
        root_ = erase(root_, value, was_erased);
        if (was_erased) {
            --size_;
        }
    }

    iterator find(const ValueType& value) const {
        return iterator(this, find(root_, value));
    }

    iterator lower_bound(const ValueType& value) const {
        return iterator(this, lower_bound(root_, value));
    }

    iterator begin() const {
        if (root_ == nullptr) {
            return iterator(this, nullptr);
        }
        Node* cur_node = root_;
        while (cur_node->left != nullptr) {
            cur_node = cur_node->left;
        }
        return iterator(this, cur_node);
    }

    iterator end() const {
        return iterator(this, nullptr);
    }

private:
    void Clear() {
        Clear(root_);
        root_ = nullptr;
        size_ = 0;
    }

    static void Clear(Node* root) {
        if (root != nullptr) {
            Clear(root->left);
            Clear(root->right);
            delete root;
        }
    }

    static void Update(Node* root) {
        if (root == nullptr) {
            return;
        }
        if (root->left != nullptr) {
            root->left->parent = root;
        }
        if (root->right != nullptr) {
            root->right->parent = root;
        }
    }

    static Node* Skew(Node* root) {
        if (root == nullptr) {
            return nullptr;
        }
        if (root->left == nullptr) {
            return root;
        }
        if (root->left->level == root->level) {
            auto tmp = root->left;
            root->left = tmp->right;
            tmp->right = root;

            tmp->parent = root->parent;
            root->parent = tmp;
            if (root->left != nullptr) {
                root->left->parent = root;
            }

            return tmp;
        }
        return root;
    }

    static Node* Split(Node* root) {
        if (root == nullptr) {
            return nullptr;
        }
        if (root->right == nullptr || root->right->right == nullptr) {
            return root;
        }
        if (root->level == root->right->right->level) {
            auto tmp = root->right;
            root->right = tmp->left;
            tmp->left = root;
            ++(tmp->level);

            tmp->parent = root->parent;
            root->parent = tmp;
            if (root->right != nullptr) {
                root->right->parent = root;
            }

            return tmp;
        }
        return root;
    }

    static Node* insert(Node* root, const ValueType& value, bool& was_inserted) {
        if (root == nullptr) {
            was_inserted = true;
            return new Node(value, nullptr, nullptr, nullptr);
        }
        if (value < root->value) {
            root->left = insert(root->left, value, was_inserted);
            if (root->left != nullptr) {
                root->left->parent = root;
            }
        } else if (root->value < value) {
            root->right = insert(root->right, value, was_inserted);
            if (root->right != nullptr) {
                root->right->parent = root;
            }
        }
        root = Skew(root);
        root = Split(root);
        Update(root);
        return root;
    }

    static Node* DecreaseLevel(Node* root) {
        if (root->left == nullptr || root->right == nullptr) {
            return root;
        }
        size_t opt_level = (root->left->level > root->right->level ? root->right->level : root->left->level) + 1;
        if (opt_level < root->level) {
            root->level = opt_level;
            if (opt_level < root->right->level) {
                root->right->level = opt_level;
            }
        }
        return root;
    }

    static Node* Predicator(Node* root_) {
        auto cur_node = root_->left;
        while (cur_node->right != nullptr) {
            cur_node = cur_node->right;
        }
        return cur_node;
    }

    static Node* Successor(Node* root_) {
        auto cur_node = root_->right;
        while (cur_node->left != nullptr) {
            cur_node = cur_node->left;
        }
        return cur_node;
    }

    static Node* erase(Node* root, const ValueType& value, bool& was_erased) {
        if (root == nullptr) {
            return nullptr;
        }
        if (value < root->value) {
            root->left = erase(root->left, value, was_erased);
            if (root->left != nullptr) {
                root->left->parent = root;
            }
        } else if (root->value < value) {
            root->right = erase(root->right, value, was_erased);
            if (root->right != nullptr) {
                root->right->parent = root;
            }
        } else {
            if (root->left == nullptr && root->right == nullptr) {
                was_erased = true;
                delete root;
                return nullptr;
            }
            if (root->left == nullptr) {
                auto tmp = Successor(root);
                root->value = tmp->value;
                root->right = erase(root->right, root->value, was_erased);
                if (root->right != nullptr) {
                    root->right->parent = root;
                }
            } else {
                auto tmp = Predicator(root);
                root->value = tmp->value;
                root->left = erase(root->left, root->value, was_erased);
                if (root->left != nullptr) {
                    root->left->parent = root;
                }
            }
        }
        root = DecreaseLevel(root);
        root = Skew(root);
        root->right = Skew(root->right);
        if (root->right != nullptr) {
            root->right->right = Skew(root->right->right);
        }
        root = Split(root);
        root->right = Split(root->right);
        return root;
    }

    static Node* find(Node* root, const ValueType& value) {
        if (root == nullptr) {
            return nullptr;
        }
        if (value < root->value) {
            return find(root->left, value);
        } else if (root->value < value) {
            return find(root->right, value);
        }
        return root;
    }

    static Node* lower_bound(Node* root, const ValueType& value) {
        if (root == nullptr) {
            return nullptr;
        }
        if (value < root->value) {
            auto it = lower_bound(root->left, value);
            if (it == nullptr) {
                return root;
            }
            return it;
        } else if (root->value < value) {
            return lower_bound(root->right, value);
        }
        return root;
    }
};

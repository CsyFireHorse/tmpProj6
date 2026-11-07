/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-27
 * @ArtifactStatusAuthority: Cursor Grok 4.6
 */
#ifndef ZMC_LIST_H_
#define ZMC_LIST_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

namespace zmc {

// Intrusive singly-linked FIFO. Node must expose get_next() / set_next(Node *).
template <typename Node>
struct List {
    List();

    bool empty() const { return head == NULL; }
    int32_t size() const { return count; }
    Node *front() const { return head; }

    void push_back(Node *node);
    Node *pop_front();

    // Drop head/tail/count only; caller must already have recycled every node.
    void reset();

    Node *head;
    Node *tail;
    int32_t count;

   private:
    List(const List &);
    List &operator=(const List &);
};

// Intrusive LIFO free list that owns its Node storage. No heap work in allocate/deallocate.
template <typename Node>
struct Pool {
    Pool();

    // Own slots[0..count) and relink them into the free list.
    void resize(int32_t count);
    // Destroy owned nodes; drop free-list links. Capacity follows std::vector::clear.
    void clear();

    Node *allocate();
    void deallocate(Node *node);

    // Pop every node from list into this pool, then reset the list links.
    void deallocate_list(List<Node> &list);

    Node &operator[](int32_t i);
    const Node &operator[](int32_t i) const;

    int32_t free_count() const { return pool_free; }
    int32_t capacity() const { return pool_capacity; }

    Node *free_head;
    int32_t pool_capacity;
    int32_t pool_free;

   private:
    Pool(const Pool &);
    Pool &operator=(const Pool &);
    std::vector<Node> nodes;
};

template <typename Node>
List<Node>::List() : head(NULL), tail(NULL), count(0) {}

template <typename Node>
void List<Node>::push_back(Node *node) {
    node->set_next(NULL);
    if (!head) {
        head = node;
        tail = node;
    } else {
        tail->set_next(node);
        tail = node;
    }
    ++count;
}

template <typename Node>
Node *List<Node>::pop_front() {
    if (!head) {
        return NULL;
    }
    Node *node = head;
    head = node->get_next();
    if (!head) {
        tail = NULL;
    }
    node->set_next(NULL);
    --count;
    return node;
}

template <typename Node>
void List<Node>::reset() {
    head = NULL;
    tail = NULL;
    count = 0;
}

template <typename Node>
Pool<Node>::Pool() : free_head(NULL), pool_capacity(0), pool_free(0) {}

template <typename Node>
void Pool<Node>::resize(int32_t count) {
    nodes.resize(static_cast<size_t>(count));
    free_head = nodes.empty() ? NULL : &nodes[0];
    pool_capacity = count;
    pool_free = count;
    for (int32_t i = 0; i < count; ++i) {
        nodes[static_cast<size_t>(i)].set_next(
            (i + 1 < count) ? &nodes[static_cast<size_t>(i + 1)] : NULL);
    }
}

template <typename Node>
void Pool<Node>::clear() {
    nodes.clear();
    free_head = NULL;
    pool_capacity = 0;
    pool_free = 0;
}

template <typename Node>
Node *Pool<Node>::allocate() {
    if (!free_head) {
        return NULL;
    }
    Node *node = free_head;
    free_head = node->get_next();
    node->set_next(NULL);
    --pool_free;
    return node;
}

template <typename Node>
void Pool<Node>::deallocate(Node *node) {
    if (!node) {
        return;
    }
    node->set_next(free_head);
    free_head = node;
    ++pool_free;
}

template <typename Node>
void Pool<Node>::deallocate_list(List<Node> &list) {
    while (!list.empty()) {
        deallocate(list.pop_front());
    }
}

template <typename Node>
Node &Pool<Node>::operator[](int32_t i) {
    return nodes[static_cast<size_t>(i)];
}

template <typename Node>
const Node &Pool<Node>::operator[](int32_t i) const {
    return nodes[static_cast<size_t>(i)];
}

}  // namespace zmc

#endif  // ZMC_LIST_H_

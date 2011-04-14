
/*
 * Copyright (C) 2008 Torch Mobile, Inc. All rights reserved.
 */

#pragma once

#include <algorithm>

#ifndef DEBUG_BTREE
#define DEBUG_BTREE		0
#endif

template <typename T, class NodeAllocator, unsigned Order = (4096 - 100) / (sizeof(T) + 4)>
class BTree
{
    enum {
		MinNumChildren = Order < 8 ? 3 : Order / 2,
		MaxNumChildren = MinNumChildren * 2,
		MinNumKeys = MinNumChildren - 1,
		MaxNumKeys = MaxNumChildren - 1,
	};

	struct Node {
		T		m_keys[MaxNumKeys];
		Node*	m_children[MaxNumChildren];
		Node*	m_parent;
		unsigned m_numKeys;

		bool isLeaf() const { return !*m_children; }

		Node* leftChild(unsigned keyIndex) const { return m_children[keyIndex]; }
		Node* rightChild(unsigned keyIndex) const { return m_children[keyIndex + 1]; }
		Node* leftMostLeaf() const {
			Node* rtn = const_cast<Node*>(this);
			while (!rtn->isLeaf()) {
				rtn = rtn->m_children[0];
			}
			return rtn;
		}
		Node* rightMostLeaf() const {
			Node* rtn = const_cast<Node*>(this);
			while (!rtn->isLeaf()) {
				rtn = rtn->m_children[rtn->m_numKeys];
			}
			return rtn;
		}
		unsigned lower_bound(const T& key) const { return std::lower_bound(m_keys, m_keys + m_numKeys, key) - m_keys; }

		void setChild(unsigned index, Node* child) {
			m_children[index] = child;
			if (child)
				child->m_parent = this;
		}
		void insert(unsigned index, const T& key, Node* rightChild) {
			if (index < m_numKeys) {
				unsigned toMove = m_numKeys - index;
				Node::moveOneBackward(m_keys + index, toMove);
				Node::moveOneBackward(m_children + index + 1, toMove);
			}
			m_keys[index] = key;
			setChild(index + 1, rightChild);
			++m_numKeys;
		}
		void eraseKeyFromLeaf(unsigned index) {
			--m_numKeys;
			if (m_numKeys > index) {
				Node::moveOneForward(m_keys + index + 1, m_numKeys - index);
			}
		}
		void eraseKeyFromNode(unsigned index) {
			--m_numKeys;
			if (m_numKeys > index) {
				unsigned toMove = m_numKeys - index;
				Node::moveOneForward(m_keys + index + 1, toMove);
				Node::moveOneForward(m_children + index + 2, toMove);
			}
		}
		void insertKeyToLeaf(unsigned index, const T& key) {
			if (index < m_numKeys) {
				unsigned toMove = m_numKeys - index;
				Node::moveOneBackward(m_keys + index, toMove);
			}
			m_keys[index] = key;
			++m_numKeys;
		}
		template <typename T1>
		static void copy(T1* dst, const T1* src, int num) {
			const T1* const dstEnd = dst + num;
			while (dst < dstEnd)
				*dst++ = *src++;
		}
		template <typename T1>
		static void moveOneForward(T1* src, int num) {
			T1* dst = src - 1;
			const T1* const dstEnd = dst + num;
			while (dst < dstEnd) {
				T1* prev = dst++;
				*prev = *dst;
			}
		}
		template <typename T1>
		static void moveOneBackward(T1* src, int num) {
			T1* dst = src + num;
			while (dst > src) {
				T1* next = dst--;
				*next = *dst;
			}
		}
	};

public:
	class iterator {
	public:
		iterator(Node* node = 0, unsigned keyIndex = 0) : m_node(node), m_keyIndex(keyIndex) {}
		bool operator==(const iterator& o) const { return m_node ? o.m_node == m_node && o.m_keyIndex == m_keyIndex : o.m_node == 0; }
		bool operator!=(const iterator& o) const { return !operator==(o); }
		T& operator*() { return m_node->m_keys[m_keyIndex]; }
		T* operator->() { return m_node->m_keys + m_keyIndex; }
        iterator& operator++() { 
			if (m_node) {
				if (!m_node->isLeaf()) {
					m_node = m_node->rightChild(m_keyIndex)->leftMostLeaf();
					m_keyIndex = 0;
				} else if (++m_keyIndex < m_node->m_numKeys) {
				} else {
					for (;;) {
						const T& firstKey = m_node->m_keys[0];
						m_node = m_node->m_parent;
						if (!m_node) {
							m_keyIndex = 0;
							break;
						}
						m_keyIndex = m_node->lower_bound(firstKey);
						if (m_keyIndex < m_node->m_numKeys) {
							break;
						}
					}
				}
			}
			return *this;
		}
        iterator operator++(int) {
			iterator rtn = *this;
			++*this;
			return rtn;
		}
	private:
		Node* m_node;
		unsigned m_keyIndex;

		friend class BTree;
	};

	BTree() : m_root(0) {}
	~BTree() { clear();	}
	bool empty() const { return m_root == 0; }
    void clear() { _freeTree(m_root); m_root = 0; }
	iterator begin() const { return iterator(m_root ? m_root->leftMostLeaf() : 0, 0); }
	iterator end() const { return iterator(); }
	iterator rbegin() const {
        if (!m_root)
            return end();
        Node* node = m_root->rightMostLeaf();
        return iterator(node, node->m_numKeys - 1);
    }
	iterator lower_bound(const T& key) const {
		if (!m_root)
			return end();

		Node* node = m_root;
		unsigned index;
		iterator closest;
		for (;;) {
			index = node->lower_bound(key);
			if (index < node->m_numKeys) {
				closest.m_node = node;
				closest.m_keyIndex = index;
				if (node->m_keys[index] == key)
					break;
			}
			if (node->isLeaf())
				break;

			node = node->m_children[index];
		}
		return closest;
	}
	void insert(const T& key) {
		if (!m_root) {
			m_root = _allocateNode(0, 1);
			m_root->m_keys[0] = key;
			return;
		}
		Node* node = m_root;
		for (;;) {
			if (node->m_numKeys == MaxNumKeys) {
				unsigned medianIndex = node->m_numKeys / 2;
				T separator = node->m_keys[medianIndex];
				// Right node:
				Node* rightNode = _allocateNode(node->m_parent, MaxNumKeys - medianIndex - 1);
				Node::copy(rightNode->m_keys, node->m_keys + medianIndex + 1, rightNode->m_numKeys);
				if (!node->isLeaf()) {
					Node::copy(rightNode->m_children, node->m_children + medianIndex + 1, rightNode->m_numKeys + 1);
					for (unsigned int i = 0; i <= rightNode->m_numKeys; ++i) {
						rightNode->m_children[i]->m_parent = rightNode;
					}
				}
				// Left node:
				// No need to clear the vectors
				node->m_numKeys = medianIndex;
				if (node == m_root) {
					m_root = _allocateNode(0, 0);
					m_root->setChild(0, node);
				}
				unsigned index = node->m_parent->lower_bound(separator);
				node->m_parent->insert(index, separator, rightNode);

				if (key == separator) {
					node->m_parent->m_keys[index] = key;
					break;
				}

				if (separator < key)
					node = rightNode;
			}

			unsigned index = node->lower_bound(key);
			if (index < node->m_numKeys && node->m_keys[index] == key) {
				// Update value
				node->m_keys[index] = key;
				break;
			}

			if (node->isLeaf()) {
				node->insertKeyToLeaf(index, key);
				break;
			}

			node = node->m_children[index];
		}
#if DEBUG_BTREE
		if (!checkIntegrity()) {
			printf("bug found!\n");
		}
#endif
	}
	void erase(const iterator& i) {
		_erase(i.m_node, i.m_keyIndex);
#if DEBUG_BTREE
		if (!checkIntegrity()) {
			printf("bug found!\n");
		}
#endif
	}
	void erase(const T& key) {
		iterator i = lower_bound(key);
		if (i != end() && *i == key) {
			erase(i);
		}
	}

#if DEBUG_BTREE
	bool checkIntegrity() const {
		if (empty())
			return true;
		iterator i = begin();
		if (i == end() || !_checkIntegrity(i))
			return false;
		iterator prev = i++;
		for (; i != end(); ++i) {
			if (!_checkIntegrity(i))
				return false;
			if (!(*prev < *i))
				return false;
			prev = i;
		}
		return true;
	}
#endif

private:
	void _freeSingleNode(Node* node) {
        NodeAllocator::releaseNode(node, m_allocatorData);
	}
	void _freeTree(Node* node) {
		if (node) {
			if (!node->isLeaf()) {
				for (unsigned int i = 0; i <= node->m_numKeys; ++i) {
					_freeTree(node->m_children[i]);
				}
			}
			NodeAllocator::releaseNode(node, m_allocatorData);
		}
	}

    Node* _allocateNode(Node* parent, unsigned numKeys) {
		Node* node = (Node*)NodeAllocator::allocateNode(sizeof(Node), m_allocatorData);
		node->m_children[0] = 0;
		node->m_numKeys = numKeys;
		node->m_parent = parent;
		return node;
	}

	void _erase(Node* node, unsigned index) {
		for (;;) {
			if (node->isLeaf()) {
				node->eraseKeyFromLeaf(index);
				_rebalance(node);
				break;
			}

			Node* leftChild = node->leftChild(index);
			Node* rightChild = node->rightChild(index);
			if (leftChild->m_numKeys > MinNumKeys) {
				Node* leaf = leftChild->rightMostLeaf();
				unsigned leafKeyIndex = leaf->m_numKeys - 1;
				node->m_keys[index] = leaf->m_keys[leafKeyIndex];
				leaf->eraseKeyFromLeaf(leafKeyIndex);
				_rebalance(leaf);
				break;
			} 
			
			if (rightChild->m_numKeys > MinNumKeys) {
				Node* leaf = rightChild->leftMostLeaf();
				node->m_keys[index] = leaf->m_keys[0];
				leaf->eraseKeyFromLeaf(0);
				_rebalance(leaf);
				break;
			}
			
			unsigned newIndex = leftChild->m_numKeys;
			_mergeSiblings(leftChild, rightChild, node->m_keys[index]);

			node->eraseKeyFromNode(index);
			_rebalance(node);

			node = leftChild;
			index = newIndex;
		}
	}

	void _rebalance(Node* node) {
		for (;;) {
			if (node == m_root) {
				if (!node->m_numKeys) {
					if (node->isLeaf()) {
						m_root = 0;
					} else {
						m_root = node->m_children[0];
						m_root->m_parent = 0;
					}
					_freeSingleNode(node);
				}
				break;
			}
			if (node->m_numKeys >= MinNumKeys)
				break;

			Node* parent = node->m_parent;
			unsigned index = parent->lower_bound(node->m_keys[0]);
			Node* leftSibling = index ? parent->leftChild(index - 1) : 0;
			Node* rightSibling = index >= parent->m_numKeys ? 0 : parent->rightChild(index);
			if (leftSibling && leftSibling->m_numKeys > MinNumKeys) {
				--index;
				if (node->isLeaf()) {
					node->insertKeyToLeaf(0, parent->m_keys[index]);
				} else {
					node->insert(0, parent->m_keys[index], node->m_children[0]);
					node->setChild(0, leftSibling->m_children[leftSibling->m_numKeys]);
				}
				--leftSibling->m_numKeys;
				parent->m_keys[index] = leftSibling->m_keys[leftSibling->m_numKeys];
				break;
			} else if (rightSibling && rightSibling->m_numKeys > MinNumKeys) {
				if (node->isLeaf()) {
					node->insertKeyToLeaf(node->m_numKeys, parent->m_keys[index]);
				} else {
					node->insert(node->m_numKeys, parent->m_keys[index], rightSibling->m_children[0]);
				}
				parent->m_keys[index] = rightSibling->m_keys[0];
				Node::moveOneForward(rightSibling->m_keys + 1, rightSibling->m_numKeys - 1);
				if (!rightSibling->isLeaf())
					Node::moveOneForward(rightSibling->m_children + 1, rightSibling->m_numKeys);
				--rightSibling->m_numKeys;
				break;
			} 
			
			if (rightSibling) {
				_mergeSiblings(node, rightSibling, parent->m_keys[index]);
			} else {
				--index;
				_mergeSiblings(leftSibling, node, parent->m_keys[index]);
			}
			parent->eraseKeyFromNode(index);
			node = parent;
		}
	}
	void _mergeSiblings(Node* left, Node *right, const T& separator) {
		left->m_keys[left->m_numKeys] = separator;
		++left->m_numKeys;
		unsigned totalKeys = left->m_numKeys + right->m_numKeys;
		if (!left->isLeaf()) {
			Node::copy(left->m_children + left->m_numKeys, right->m_children, right->m_numKeys + 1);
			for (unsigned int i = left->m_numKeys; i <= totalKeys; ++i) {
				left->m_children[i]->m_parent = left;
			}
		}
		Node::copy(left->m_keys + left->m_numKeys, right->m_keys, right->m_numKeys);
		left->m_numKeys = totalKeys;
		_freeSingleNode(right);
	}

#if	DEBUG_BTREE
	bool _checkIntegrity(iterator i) const {
		if (i.m_node == m_root ? i.m_node->m_numKeys == 0 : i.m_node->m_numKeys < MinNumKeys)
			return false;
		if (i.m_node->m_numKeys > MaxNumKeys)
			return false;
		return true;
	}
#endif

	// Not copyable yet
	BTree(const BTree&);
	BTree& operator=(const BTree&);

	Node* m_root;
    static void* m_allocatorData;
};

template <typename T, class NodeAllocator, unsigned Order>
void* BTree<T, NodeAllocator, Order>::m_allocatorData = 0;

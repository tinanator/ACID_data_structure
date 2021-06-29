#pragma once
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class Iterator;

template<typename T>
struct Node
{
	std::shared_mutex mutex;
	T val;
	std::atomic<int> countRef = 0;
	std::atomic<bool> deleted = false;
	Node* next;
	Node* prev;
	~Node() {
		next = nullptr;
		prev = nullptr;
		countRef.store(0);
	};
};


template<typename T>
class ConsistentList
{
public:

	friend Iterator<T>;

	ConsistentList() {
		_size = 0;
		realSize = _size;
		beginNode = new Node<T>();
		endNode = new Node<T>();
		beginNode->countRef = 1;
		beginNode->next = endNode;
		beginNode->prev = nullptr;
		endNode->prev = beginNode;
		endNode->next = nullptr;
		endNode->countRef = 1;
		head = beginNode;
		tail = endNode;
	};

	ConsistentList(std::initializer_list<T> init) {
		_size = 0;
		realSize = _size;
		beginNode = new Node<T>();
		endNode = new Node<T>();
		beginNode->countRef = 1;
		beginNode->next = endNode;
		beginNode->prev = nullptr;
		endNode->prev = beginNode;
		endNode->next = nullptr;
		endNode->countRef = 1;

		for (auto& el : init) {
			push_back(el);
		}
	}


	~ConsistentList() {
		Node<T>* node = beginNode->next;
		while (node != endNode) {
			Node<T>* prev = node;
			beginNode->next = node->next;
			node->next->prev = beginNode;
			node = node->next;
			delete prev;
		}
		delete beginNode;
		delete endNode;
		tail = nullptr;
		head = nullptr;
		_size = 0;
		realSize = 0;
	}


	void push_back(T val) {
		insert(end(), val);
	}

	void push_front(T val) {
		if (_size == 0) {
			insert(begin(), val);
		}
		else {
			insert(begin()->next, val);
		}
	}

	Iterator<T> insert(Iterator<T> pos, const T& val) {
		for (bool retry = true; retry;) {
			retry = false;

			auto lock = std::shared_lock(pos.pnode->mutex);

			if (pos.pnode->countRef <= 0) {
				return pos;
			}

			auto prev = pos.pnode->prev;
			assert(prev->countRef);

			lock.unlock();

			auto lock2 = std::unique_lock(pos.pnode->prev->mutex);
			auto lock1 = std::unique_lock(pos.pnode->mutex);

			if (pos.pnode->prev == prev) {
				Node<T>* newNode = new Node<T>();
				newNode->val = val;

				newNode->next = pos.pnode;
				newNode->prev = pos.pnode->prev;
				pos.pnode->prev = newNode;
				newNode->prev->next = newNode;

				_size++;
				realSize = _size;

				newNode->countRef += 2;
			}
			else {
				retry = true;
				lock2.unlock();
				lock1.unlock();
			}
		}
		return pos;
	}

	int getRealSize() {
		return realSize;
	}

	Node<T>* getBeginNode() {
		return beginNode;
	}

	Node<T>* getEndNode() {
		return endNode;
	}

	Iterator<T> begin() {
		if (_size > 0) {
			Iterator<T> iter(&beginNode->next, this);
			return iter;
		}
		else {
			Iterator<T> iter(&endNode, this);
			return iter;
		}
	}

	Iterator<T> end() {
		Iterator<T> iter(&endNode, this);
		return iter;
	}
	
	int size() {
		return _size;
	}

	bool empty() {
		return _size == 0;
	}

	

	void advance(Iterator<T>& it, int n) {
		while (n > 0) {
			--n;
			Node<T>* prev = it.pnode;
			acquire(&it.pnode, it.pnode->next);
			release(prev);
		}
		while (n < 0) {
			++n;
			Node<T>* prev = it.pnode;
			acquire(&it.pnode, it.pnode->prev);
			release(prev);
		}
	}


	void erase(Iterator<T>& pos) {

		Node<T>* node = pos.pnode;

		for (bool retry = true; retry;) {
			retry = false;

			auto lock = std::shared_lock(node->mutex);

			if (node->countRef <= 0 || node == endNode || node == beginNode) {
				return;
			}
			
			auto prev = node->prev;
			assert(prev->countRef);
			prev->countRef++;

			auto next = node->next;
			assert(next->countRef);
			next->countRef++;

			lock.unlock();

			auto lock1 = std::unique_lock(prev->mutex);
			auto lock2 = std::shared_lock(node->mutex);
			auto lock3 = std::unique_lock(next->mutex);
			if (prev == node->prev && next == node->next) {

				if (!node->deleted) {
					_size--;
					node->deleted = true;
				}

				if (next->prev == node) {
					next->prev = prev;
					node->countRef--;
				}
				if (prev->next == node) {
					prev->next = next;
					node->countRef--;
				}

				Node<T>* prev1 = node;
				acquire(&(node), next);
				release(prev1);
			}
			else {
				retry = true;
				prev->countRef--;
				next->countRef--;
				lock1.unlock();
				lock2.unlock();
				lock3.unlock();
			}

		}
	}

	int front() {
		if (beginNode->next == endNode) {
			throw std::runtime_error("empty list");
		}
		return beginNode->next->val;
	}

	int back() {
		if (endNode->prev == beginNode) {
			throw std::runtime_error("empty list");
		}
		return endNode->prev->val;
	}


private:

	void release(Node<T>* node) {
		std::queue<Node<T>*> nodesToDelete;

		if (node) {
			node->countRef--;
			if (node->countRef <= 0) {
				nodesToDelete.push(node->next);
				nodesToDelete.push(node->prev);
				node->next = nullptr;
				node->prev = nullptr;
				delete node;
				node = nullptr;
				realSize--;
				while (!nodesToDelete.empty()) {
					Node<T>* n = nodesToDelete.front();
					nodesToDelete.pop();
					n->countRef--;
					if (n->countRef <= 0) {
						nodesToDelete.push(n->next);
						nodesToDelete.push(n->prev);
						n->next = nullptr;
						n->prev = nullptr;
						delete n;
						n = nullptr;
						realSize--;
					}
				}
			}
		}
	}

	void acquire(Node<T>** curPtr, Node<T>* nextPtr) {
		while (nextPtr->deleted) {
			nextPtr = nextPtr->next;
		}
		*curPtr = nextPtr;
		(*curPtr)->countRef++;
	}

private:

	std::shared_mutex m;

	std::condition_variable_any cv;

	int _size;

	int realSize;

	Node<T>* head;

	Node<T>* tail;

	Node<T>* beginNode;

	Node<T>* endNode;
};
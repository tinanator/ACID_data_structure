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
		_size = 0;;
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
		realSize.store(_size);
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

		Node<T>* node = pos.pnode;

		auto lock = std::unique_lock(node->mutex);

		if (node->deleted) {
			while (node->prev->deleted) {
				node = node->prev;
			}
			return Iterator<T>(&node, this);
		}

		if (node == beginNode) {
			return pos;
		}

		for (bool retry = true; retry;) {
			retry = false;

			auto prev = node->prev;
			assert(prev->countRef);

			auto lock2 = std::unique_lock(prev->mutex);
			
			if (prev->next == node) {

				Node<T>* newNode = new Node<T>();
				newNode->val = val;

				newNode->next = node;
				newNode->prev = prev;
				node->prev = newNode;
				prev->next = newNode;
				newNode->countRef += 2;

				_size++;
			}
			else {
				retry = true;
				lock2.unlock();
			}
		
		}
		return Iterator<T>(&(node->prev), this);
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


	Iterator<T> erase(Iterator<T>& pos) {

		Node<T>* node = pos.pnode;

		for (bool retry = true; retry;) {
			retry = false;

			auto lock = std::shared_lock(node->mutex);

			if (node->deleted) {
				return pos;
			}

			if (node == endNode || node == beginNode) {
				return pos;
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

			if (prev->next == node && next->prev == node) {

				node->deleted = true;

				node->next->prev = prev;
				node->countRef--;
				node->prev->next = next;
				node->countRef--;

				_size--;

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
		return Iterator<T>(&(node)->next, this);
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
				delete node;
				realSize--;
				while (!nodesToDelete.empty()) {
					Node<T>* n = nodesToDelete.front();
					nodesToDelete.pop();
					n->countRef--;
					if (n->countRef <= 0) {
						nodesToDelete.push(n->next);
						nodesToDelete.push(n->prev);
						delete n;
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

	std::atomic<int> _size;

	std::atomic<int> realSize;

	Node<T>* head;

	Node<T>* tail;

	Node<T>* beginNode;

	Node<T>* endNode;
};
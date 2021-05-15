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
	T val;
	std::atomic<int> countRef = 0;
	bool deleted = false;
	Node* next;
	Node* prev;
	~Node() {

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
		std::unique_lock<std::shared_mutex> lock(m);
		_size++;
		realSize = _size;
		Node<T>* newNode = new Node<T>();
		newNode->val = val;
		newNode->prev = endNode->prev;
		endNode->prev = newNode;
		newNode->next = endNode;
		newNode->prev->next = newNode;
		newNode->countRef += 2;

	}

	void push_front(T val) {
		std::unique_lock<std::shared_mutex> lock(m);
		_size++;
		realSize = _size;
		Node<T>* newNode = new Node<T>();
		newNode->val = val;
		newNode->next = beginNode->next;
		beginNode->next = newNode;
		newNode->prev = beginNode;
		newNode->next->prev = newNode;
		newNode->countRef += 2;
	}

	Iterator<T> insert(Iterator<T> pos, const T& val) {
		std::unique_lock<std::shared_mutex> lock(m);
		Node<T>* newNode = new Node<T>();
		newNode->val = val;
		
		if (pos.pnode == beginNode) {
			newNode->prev = pos.pnode;
			newNode->next = pos.pnode->next;
			pos.pnode->next = newNode;
			newNode->next->prev = newNode;
		}
		else {
			newNode->next = pos.pnode;
			newNode->prev = pos.pnode->prev;
			pos.pnode->prev = newNode;
			newNode->prev->next = newNode;
		}
		_size++;
		realSize = _size;

		newNode->countRef += 2;
		return pos;
	}

	int getRealSize() {
		std::shared_lock<std::shared_mutex> lock(m);
		return realSize;
	}

	Node<T>* getBeginNode() {
		std::shared_lock<std::shared_mutex> lock(m);
		return beginNode;
	}

	Node<T>* getEndNode() {
		std::shared_lock<std::shared_mutex> lock(m);
		return endNode;
	}

	Iterator<T> begin() {
		std::unique_lock<std::shared_mutex> lock(m);
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
		std::unique_lock<std::shared_mutex> lock(m);
		Iterator<T> iter(&endNode, this);
		return iter;
	}
	
	int size() {
		std::shared_lock<std::shared_mutex> lock(m);
		return _size;
	}

	bool empty() {
		std::shared_lock<std::shared_mutex> lock(m);
		return _size == 0;
	}

	

	void advance(Iterator<T>& it, int n) {
		std::unique_lock<std::shared_mutex> lock(m);
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
		std::unique_lock<std::shared_mutex> lock(m);
		Node<T>* node = pos.pnode;
		if (node == endNode || node == beginNode) {
			return;
		}
		if (!node->deleted) {
			_size--;
			node->deleted = true;
		}
		if (node->next->prev == node) {
			node->next->prev = node->prev;
			node->prev->countRef++;
			node->countRef--;
		}
		if (node->prev->next == node) {
			node->prev->next = node->next;
			node->next->countRef++;
			node->countRef--;
		}

		Node<T>* prev = pos.pnode;
		acquire(&(pos.pnode), pos.pnode->next);
		release(prev);
	}

	int front() {
		std::shared_lock<std::shared_mutex> lock(m);
		if (beginNode->next == endNode) {
			throw std::runtime_error("empty list");
		}
		return beginNode->next->val;
	}

	int back() {
		std::shared_lock<std::shared_mutex> lock(m);
		if (endNode->prev == beginNode) {
			throw std::runtime_error("empty list");
		}
		return endNode->prev->val;
	}

	std::shared_mutex& getMutex()
	{
		return m;
	}

	std::condition_variable getCondVar()
	{
		std::shared_lock<std::shared_mutex> lock(m);
		return cv;
	}

private:

	void release(Node<T>* node) {
		std::queue<Node<T>*> q;

		if (node) {
			node->countRef--;
			if (node->countRef == 0) {
				q.push(node->next);
				q.push(node->prev);
				node->next = nullptr;
				node->prev = nullptr;
				delete node;
				realSize--;
				while (!q.empty()) {
					Node<T>* n = q.front();
					q.pop();
					n->countRef--;
					if (n->countRef == 0) {
						q.push(n->next);
						q.push(n->prev);
						n->next = nullptr;
						n->prev = nullptr;
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

	int _size;

	int realSize;

	Node<T>* head;

	Node<T>* tail;

	Node<T>* beginNode;

	Node<T>* endNode;
};
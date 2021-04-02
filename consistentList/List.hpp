#pragma once
#include <iostream>

template<typename T>
class Iterator;

template<typename T>
struct Node
{
	T val;
	int countRef = 0;
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


	ConsistentList() {
		_size = 0;
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
	}


	void push_back(T val) {
		_size++;
		Node<T>* newNode = new Node<T>();
		newNode->val = val;
		newNode->prev = endNode->prev;
		endNode->prev = newNode;
		newNode->next = endNode;
		newNode->prev->next = newNode;
		newNode->countRef += 2;

	}

	void push_front(T val) {
		_size++;
		Node<T>* newNode = new Node<T>();
		newNode->val = val;
		newNode->next = beginNode->next;
		beginNode->next = newNode;
		newNode->prev = beginNode;
		newNode->next->prev = newNode;
		newNode->countRef += 2;
	}

	Iterator<T> insert(Iterator<T> pos, const T& val) {
		_size++;
		Node<T>* newNode = new Node<T>();
		newNode->val = val;
		newNode->next = pos.getPtr();
		newNode->prev = pos.getPtr()->prev;
		pos.getPtr()->prev = newNode;
		newNode->prev->next = newNode;
		newNode->countRef += 2;
		return pos;
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

	void release(Node<T>* node) {
		if (node) {
			node->countRef--;
			if (node->countRef == 0) {
				release(node->next);
				node->next = nullptr;
				release(node->prev);
				node->prev = nullptr;
				std::cout << "node " << node->val << " deleted" << std::endl;
				delete node;
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

	void erase(Iterator<T>& pos) {
		_size--;
		Node<T>* node = pos.getPtr();
		node->deleted = true;
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
		pos++;
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

	Node<int>* CreateNewNode() {
	
	}

private:

	int _size;

	Node<T>* head;

	Node<T>* tail;

	Node<T>* beginNode;

	Node<T>* endNode;
};
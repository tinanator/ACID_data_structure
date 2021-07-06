#pragma once
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <queue>




template<typename T>
struct PurgedItem
{
	PurgedItem(T* node) : node(node) {

	}

	T* node = nullptr;
	PurgedItem<T>* next = nullptr;
};



template<typename T>
class PurgedList {
public:
	PurgedList(): purgeThread(std::thread(&PurgedList<T>::runPurge, this)) {
		
	}

	~PurgedList() {
		stopPurge();
		purgeThread.join();
		delete head;
	}

	void pullToPurge(T* node) {
		PurgedItem<T>* pnode = new PurgedItem<T>(node);
		pnode->next = head;
		head = pnode;
	}

	void runPurge() {
		while (toRunPurge) {
			purge();
		}
		purge();
	}

	void stopPurge() {
		toRunPurge = false;
	}

	std::shared_mutex mutex;

private:

	void purge() {

		auto lock = std::unique_lock(mutex);
		auto purgeStart = head;
		lock.unlock();

		auto item = purgeStart;
		if (item == nullptr) {
			return;
		}
		while (item->next != nullptr) {
			PurgedItem<T>* toDelete = nullptr;
			if (item->next->node->countRef > 0 || item->next->node->marked) {
				toDelete = item;
			}
			else {
				item->next->node->marked = true;
			}
			item = item->next;
			if (toDelete) {
				delete toDelete;
			}
		}

		lock.lock();

		auto newStart = head;

		lock.unlock();

		item = newStart->next;

		while (item != purgeStart->next) {
			PurgedItem<T>* toDelete = nullptr;
			if (item->node->marked) {
				auto toDelete = item;
			}

			item = item->next;

			if (toDelete) {
				delete toDelete;
			}
		}

		item = purgeStart->next;

		purgeStart->next = nullptr;

		while (item != nullptr) {
			auto next = item->next;
			delete item->node;
			delete item;
			item = next;
		}
	}

	std::thread purgeThread;

	bool toRunPurge = true;

	PurgedItem<T>* head = nullptr;
};


template<typename T>
class Iterator;

template<typename T>
struct Node
{
	std::shared_mutex mutex;
	T val;
	std::atomic<int> countRef = 0;
	std::atomic<bool> deleted = false;

	std::atomic<bool> marked = false;

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

		gc = new PurgedList<Node<T>>();
		
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

		gc = new PurgedList<Node<T>>();

		for (auto& el : init) {
			push_back(el);
		}
	}


	~ConsistentList() {
		Node<T>* node = beginNode->next;
		while (node != endNode) {
			Node<T>* prev = node;
			node = node->next;
			delete prev;
			prev = nullptr;
		}
		delete beginNode;
		delete endNode;
		tail = nullptr;
		head = nullptr;
		_size = 0;
		realSize = 0;

		delete gc;
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
		Node<T>* node;
		if (_size > 0) {
			node = beginNode->next;
		}
		else {
			node = endNode;
		}
		std::unique_lock<std::shared_mutex>(node->mutex);
		Iterator<T> iter(&node, this);
		return iter;
	}

	Iterator<T> end() {
		Node<T>* node = endNode;
		std::unique_lock<std::shared_mutex>(node->mutex);
		Iterator<T> iter(&node, this);
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
			it++;
		}
		while (n < 0) {
			++n;
			it--;
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
				release(node);
				node->prev->next = next;
				release(node);

				_size--;

			}
			else {
				retry = true;
				release(prev);
				release(next);
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
		if (!node) {
			return;
		}
		auto lock = std::shared_lock(gc->mutex);
		std::queue<Node<T>*> nodesToDelete;
		int newVal = --node->countRef;
		if (newVal == 0) {
			gc->pullToPurge(node);
			nodesToDelete.push(node->next);
			nodesToDelete.push(node->prev);
			while (!nodesToDelete.empty()) {
				Node<T>* n = nodesToDelete.front();
				nodesToDelete.pop();
				n->countRef--;
				if (n->countRef == 0) {
					nodesToDelete.push(n->next);
					nodesToDelete.push(n->prev);
					gc->pullToPurge(n);
				}
			}
		}

	}

	void acquire(Node<T>** curPtr, Node<T>* nextPtr) {
		if (*curPtr == endNode) {
			std::cout << "ERROR";
		}
		while (nextPtr->deleted) {
			nextPtr = nextPtr->next;
		}
		*curPtr = nextPtr;
		(*curPtr)->countRef++;
	}

	void acquireBack(Node<T>** curPtr, Node<T>* prevPtr) {
		if (*curPtr == beginNode) {
			std::cout << "ERROR";
		}
		while (prevPtr->deleted) {
			prevPtr = prevPtr->prev;
		}
		*curPtr = prevPtr;
		(*curPtr)->countRef++;
	}

private:

	PurgedList<Node<T>>* gc;

	bool toRunPurge = true;

	std::shared_mutex m;

	std::condition_variable_any cv;

	std::atomic<int> _size;

	int realSize;

	Node<T>* head;

	Node<T>* tail;

	Node<T>* beginNode;

	Node<T>* endNode;

	std::thread purgeThread;
};
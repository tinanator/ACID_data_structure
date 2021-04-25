#pragma once
#include "List.hpp"

template<typename T>
class Iterator
{
public:
	Iterator<T>() {
		pnode = nullptr;
	}

	Iterator<T>(Node<T>** node, ConsistentList<T>* _list) : pnode(*node), list(_list) {
		(*node)->countRef++;
	}

	Iterator<T>(const Iterator<T>& other) : pnode(other.pnode), list(other.list) {
		pnode->countRef++;
	}

	~Iterator<T>() {
		Node<T>* node = pnode;
		list->release(node);
		list = nullptr;
	}

	Node<T>* getPtr() {
		return pnode;
	}

	Iterator<T>& operator ++() {
		std::unique_lock<std::shared_mutex> lock(list->getMutex());
		Node<T>* prev = pnode;
		list->acquire(&pnode, pnode->next);
		list->release(prev);
		return *this;
	}

	Iterator<T> operator ++(T) {
		Iterator<int> iterator = *this;
		++(*this);
		return *this;
	}

	Iterator<T>& operator --() {
		std::unique_lock<std::shared_mutex> lock(list->getMutex());
		Node<T>* prev = pnode;
		list->acquire(&pnode, pnode->prev);
		list->release(prev);
		return *this;
	}

	Iterator<T> operator --(T) {
		Iterator<int> iterator = *this;
		--(*this);
		return *this;
	}

	friend bool operator ==(const Iterator<T>& lhs, const Iterator<T>& rhs) {
		return lhs.pnode == rhs.pnode;
	}

	friend bool operator !=(const Iterator<T>& lhs, const Iterator<T>& rhs) {
		return !(lhs.pnode == rhs.pnode);
	}

	int& operator *() {
		std::shared_lock<std::shared_mutex> lock(list->getMutex());
		//list->getCondVar().wait(lock);
		return pnode->val;
	}

	Iterator<T>& operator=(Iterator<T>& other) {
		if (this == &other) {
			return *this;
		}
		pnode = other.pnode;
		list = other.list;
		pnode->countRef++;
		return *this;
	}

	Iterator<T>& operator=(Iterator<T>&& other) noexcept
	{
		if (this == &other)
			return *this;

		pnode = std::exchange(other.pnode, nullptr);
		list = std::exchange(other.list, nullptr);
		return *this;
	}

private:

	Node<T>* pnode;

	ConsistentList<T>* list;
};
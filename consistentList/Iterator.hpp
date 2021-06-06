#pragma once
#include "List.hpp"

template<typename T>
class Iterator
{
public:


	friend ConsistentList<T>;
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

	Iterator<T> operator ++(int) {
		Iterator<T> iterator = *this;
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

	Iterator<T> operator --(int) {
		Iterator<T> iterator = *this;
		--(*this);
		return *this;
	}

	friend bool operator ==(const Iterator<T>& lhs, const Iterator<T>& rhs) {
		if (lhs.list == rhs.list) {
			std::shared_lock<std::shared_mutex> lock1(lhs.list->getMutex());
			return lhs.pnode == rhs.pnode;
		}
		else {
			std::shared_lock<std::shared_mutex> lock1(lhs.list->getMutex());
			std::shared_lock<std::shared_mutex> lock2(rhs.list->getMutex());
			return lhs.pnode == rhs.pnode;
		}

	}

	friend bool operator !=(const Iterator<T>& lhs, const Iterator<T>& rhs) {
		if (lhs.list == rhs.list) {
			std::shared_lock<std::shared_mutex> lock1(lhs.list->getMutex());
			return !(lhs.pnode == rhs.pnode);
		}
		else {
			std::shared_lock<std::shared_mutex> lock1(lhs.list->getMutex());
			std::shared_lock<std::shared_mutex> lock2(rhs.list->getMutex());
			return !(lhs.pnode == rhs.pnode);
		}

	}

	T& operator *() {
		std::shared_lock<std::shared_mutex> lock(list->getMutex());
		return pnode->val;
	}

	T& get() {
		std::shared_lock<std::shared_mutex> lock(list->getMutex());
		return pnode->val;
	}

	void set(T val) {
		std::unique_lock<std::shared_mutex> lock(list->getMutex());
		pnode->val = val;
	}

	Iterator<T>& operator=(Iterator<T>& other) {
		if (list == other.list) {
			std::shared_lock<std::shared_mutex> lock1(list->getMutex());
			if (this == &other) {
				return *this;
			}
			pnode = other.pnode;
			list = other.list;
			pnode->countRef++;
			return *this;
		}
		else {
			std::unique_lock<std::shared_mutex> lock(list->getMutex());
			std::shared_lock<std::shared_mutex> lock2(other.list->getMutex());
			if (this == &other) {
				return *this;
			}
			pnode = other.pnode;
			list = other.list;
			pnode->countRef++;
			return *this;
		}
	}

	Iterator<T>& operator=(Iterator<T>&& other) noexcept
	{
		if (list == other.list) {
			std::shared_lock<std::shared_mutex> lock1(list->getMutex());

			if (this == &other)
				return *this;

			pnode = std::exchange(other.pnode, nullptr);
			list = std::exchange(other.list, nullptr);
			return *this;
		}
		else {
			std::unique_lock<std::shared_mutex> lock(list->getMutex());
			std::shared_lock<std::shared_mutex> lock2(other.list->getMutex());

			if (this == &other)
				return *this;

			pnode = std::exchange(other.pnode, nullptr);
			list = std::exchange(other.list, nullptr);
			return *this;
		}

	}

private:

	Node<T>* pnode;

	ConsistentList<T>* list;
};
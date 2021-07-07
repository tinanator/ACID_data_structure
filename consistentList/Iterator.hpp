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
		auto lock = std::shared_lock(pnode->mutex);
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
		auto lock = std::shared_lock(pnode->mutex);
		Node<T>* prev = pnode;
		list->acquireBack(&pnode, pnode->prev);
		list->release(prev);
		return *this;
	}

	Iterator<T> operator --(int) {
		Iterator<T> iterator = *this;
		--(*this);
		return *this;
	}

	friend bool operator ==(const Iterator<T>& lhs, const Iterator<T>& rhs) {
		return lhs.pnode == rhs.pnode;
	}

	friend bool operator !=(const Iterator<T>& lhs, const Iterator<T>& rhs) {
		return !(lhs.pnode == rhs.pnode);
	}

	T& operator *() {
		auto lock = std::shared_lock(pnode->mutex);
		return pnode->val;
	}

	T& get() {
		auto lock = std::shared_lock(pnode->mutex);
		return pnode->val;
	}

	void set(T val) {
		auto lock = std::unique_lock(pnode->mutex);
		pnode->val = val;
	}

	Iterator<T>& operator=(Iterator<T>& other) {

	//	auto lock = std::unique_lock(pnode->mutex);
		if (this == &other) {
			return *this;
		}
	//	auto lock1 = std::shared_lock(other.pnode->mutex);
		pnode = other.pnode;
		list = other.list;
		pnode->countRef++;
		return *this;
	}

	Iterator<T>& operator=(Iterator<T>&& other) noexcept
	{
	//	auto lock = std::unique_lock(pnode->mutex);
		if (this == &other)
			return *this;

	//	auto lock1 = std::unique_lock(other.pnode->mutex);
		pnode = std::exchange(other.pnode, nullptr);
		list = std::exchange(other.list, nullptr);
		return *this;
	}

private:

	Node<T>* pnode;

	ConsistentList<T>* list;
};
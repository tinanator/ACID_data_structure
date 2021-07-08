#pragma once
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "Iterator.hpp"
enum class TrxType {
	read, write
};

struct Trx {
	
	Trx(int ver, TrxType type) : ver(ver), type(type) {
		
	}

	friend bool operator ==(const Trx& lhs, const Trx& rhs) = default;

	friend bool operator !=(const Trx& lhs, const Trx& rhs) = default;
	
	int ver = 0;
	TrxType type;
};


class TrxManager {
public:

	TrxManager() {}

	~TrxManager() {}

	Trx beginWrite() {
		writer.lock();
		trxVec.emplace_back(getVersion(), TrxType::write);
		return trxVec.back();
	}

	Trx beginRead() {
		trxVec.emplace_back(getVersion(), TrxType::read);
		return trxVec.back();
	}

	void commit(Trx trx) {
		if (trx.type == TrxType::write) {
			writer.unlock();
		}
		auto it = std::find(trxVec.begin(), trxVec.end(), trx);
		if (it != trxVec.end()) {
			std::swap(*it, trxVec.back());
			trxVec.pop_back();
		}
	}

	int getVersion() {
		return ++nextVer;
	}

	std::vector<Trx>& getActiveTxr() {
		return trxVec;
	}

private:
	std::shared_mutex writer;
	int nextVer = 0;
	std::vector<Trx> trxVec;
};


template<typename T>
struct Node
{
	Node(T val, int startVer) : val(val), startVersion(startVer), endVersion(INT_MAX) {
	}

	Node(Node<T>& other) = delete;

	~Node() {}

	T val;
	Node<T>* next = nullptr;
	Node<T>* prev = nullptr;;
	int startVersion;
	int endVersion;
	bool deleted = false;
	std::shared_mutex mutex;
};

template<typename T>
class Iterator {
public:
	Iterator(Node<T>* node, Trx trx) : node(node), trx(trx) {}

	Iterator(const Iterator<T>& other) = default;

	~Iterator() {}

	Iterator& operator ++() {
		auto lock = std::shared_lock(node->mutex);
		if (!node) {
			return *this;
		}
		node = node->next;
		while (!isReadable()) {
			node = node->next;
		}
		return *this;
	}

	Iterator operator ++(int) {
		Iterator<T> iterator = *this;
		++(*this);
		return *this;
	}

	Iterator& operator --() {
		auto lock = std::shared_lock(node->mutex);
		if (!node) {
			return *this;
		}
		node = node->prev;
		while (!isReadable()) {
			node = node->prev;
		}
		return *this;
	}

	Node<T>* getNode() {
		return node;
	}

	Iterator operator --(int) {
		Iterator iterator = *this;
		--(*this);
		return *this;
	}

	friend bool operator ==(const Iterator& lhs, const Iterator& rhs) {
		return lhs.node == rhs.node && lhs.trx == rhs.trx;
	}

	friend bool operator !=(const Iterator& lhs, const Iterator& rhs) {
		return !(lhs.node == rhs.node || lhs.trx == rhs.trx);
	}

	T& operator *() {
		auto lock = std::shared_lock(node->mutex);
		return node->val;
	}

	void set(T val) {
		auto lock = std::unique_lock(node->mutex);
		node->val = val;
	}

	T get() {
		auto lock = std::shared_lock(node->mutex);
		return node->val;
	}

	Iterator<T>& operator=(Iterator<T>& other)
	{
		if (this == &other) {
			return *this;
		}
		node = other.node;
		return *this;
	}

	Iterator<T>& operator=(Iterator<T>&& other) noexcept
	{
		if (this == &other)
			return *this;

		node = std::exchange(other.node, nullptr);
		return *this;
	}

	bool isReadable() {
		return isReadable(node, trx);
	}

	static bool isReadable(Node<T>* node, Trx trx) {
		auto lock = std::shared_lock(node->mutex);

		if (node == nullptr) {
			return true;
		}
		if (trx.type == TrxType::write) {
			return node->endVersion == INT_MAX;
		}
		else {
			return node->startVersion <= trx.ver
				&& node->endVersion >= trx.ver;
		}

	}

	Trx getTrx() {
		return trx;
	}
	Trx trx;

	Node<T>* node;
	
};



template<typename T>
class List {
	public:
		explicit List(TrxManager& trxManager): head(nullptr), tail(nullptr), trxManager(trxManager) {

		}
		~List() {
			auto node = head->next;
			while (node) {
				auto prev = node;
				node = node->next;
				delete prev;
			}
		}

		Iterator<T> back(Trx trx) {
			auto node = head;
			Node<T>* result = nullptr;
			while (node != nullptr) {
				if (!Iterator<T>::isReadable(node, trx)) {
						result = node;
				}
				node = node->next;
			}
			return Iterator<T>(result, trx);
		}

		Iterator<T> insert(Iterator<T> pos, T val) {
			auto newNode = new Node<T>(val, trxManager.getVersion());
			auto node = pos.getNode();
			if (pos == end(pos.trx)) {
				auto backIt = back(pos.trx);
				if (backIt == end(pos.trx)) {
					node = head;
				}
				else {
					node = backIt.getNode();
				}
				newNode->prev = node;
				if (node) {
					newNode->next = node->next;
					node->next = newNode;
				}
				else {
					head = newNode;
				}
				return Iterator<T>(newNode, pos.trx);
			}
			newNode->next = node;
			newNode->prev = node->prev;
			node->prev = newNode;
			if (newNode->prev) {
				newNode->prev->next = newNode;
			}
			else {
				head = newNode;
			}
			return Iterator<T>(newNode, pos.trx);
		}

		Iterator<T> erase(Iterator<T> pos) {
			Iterator<T> newIt = pos;
			newIt++;
			pos.getNode()->endVersion = trxManager.getVersion();
			return newIt;
		}

		Iterator<T> begin(Trx trx) {
			auto node = head;
			while (node && !Iterator<T>::isReadable(node, trx)) {
				node = node->next;
			}
			return Iterator<T>(node, trx);
		}

		Iterator<T> end(Trx trx) {
			return Iterator<T>(nullptr, trx);
		}

		int size(Trx trx) {
			int size = 0;
			for (auto it = begin(trx); it != end(trx); ++it) {
				size++;
			}
			return size;
		}


		void gc(std::vector<Trx>& trxVec) {
			auto lock = std::unique_lock(mutex);
			auto node = head;
			while (node) {
				if (node->endVersion == INT_MAX) {
					node = node->next;
				}
				else {
					bool found = false;
					for (const auto& trx : trxVec)
					{
						if (Iterator<T>::isReadable(node, trx)) {
							found = true;
							break;
						}
					}
					if (found) {
						node = node->next;
					}
					else {
						if (node->next) {
							node->next->prev = node->prev;
						}
						if (node->prev) {
							node->prev->next = node->next;
						}
						else {
							head = node->next;
						}

						auto prev = node;
						node = node->next;
						delete prev;
					}
				}

			}
		}

		Node<T>* getHead() {
			return head;
		}

		std::shared_mutex mutex;

private:
	Node<T>* head;
	Node<T>* tail;

	TrxManager& trxManager;
};



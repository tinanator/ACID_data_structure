#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "List.hpp"
#include "Iterator.hpp"
#include <iostream>
#include <future>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <barrier>
#include <latch>
//
//TEST_CASE("Consistent 1 thread list") {
//
//	SECTION("Begin() End() front() back()") {
//		ConsistentList<int> list;
//		Iterator<int> it = list.begin();
//		REQUIRE(it == list.end());
//
//		list.push_back(1);
//		list.push_back(2);
//		list.push_back(3);
//		list.push_back(4);
//
//		it = list.begin();
//
//		REQUIRE(*it == 1);
//
//		auto val = list.front();
//
//		REQUIRE(val == 1);
//
//		val = list.back();
//
//		REQUIRE(val == 4);
//	}
//
//
//	SECTION("Initializing list") {
//		ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });
//
//		auto val = list.front();
//
//		REQUIRE(val == 1);
//
//		val = list.back();
//
//		REQUIRE(val == 6);
//	}
//
//	SECTION("erase") {
//		ConsistentList<int> list({ 1, 2, 3, 4 });
//
//		REQUIRE(list.size() == 4);
//
//		Iterator<int> it = list.begin();
//
//		list.erase(it);
//
//		REQUIRE(list.size() == 3);
//		REQUIRE(*it == 2);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it);
//
//		REQUIRE(list.size() == 2);
//		REQUIRE(*it == 3);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it);
//
//		REQUIRE(list.size() == 1);
//		REQUIRE(*it == 4);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it);
//
//		REQUIRE(list.size() == 0);
//		REQUIRE(it == list.end());
//	}
//
//	SECTION("inc dec") {
//		ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });
//
//		auto it = list.begin();
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->next->countRef == 2);
//
//		it++;
//		REQUIRE(*it == 2);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev->countRef == 2);
//
//		it++;
//		REQUIRE(*it == 3);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev->countRef == 2);
//
//		++it;
//		REQUIRE(*it == 4);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev->countRef == 2);
//
//		++it;
//		REQUIRE(*it == 5);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->prev->countRef == 2);
//
//		it--;
//		REQUIRE(*it == 4);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->next->countRef == 2);
//
//		it--;
//		REQUIRE(*it == 3);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->next->countRef == 2);
//
//		--it;
//		REQUIRE(*it == 2);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->next->countRef == 2);
//
//		--it;
//		REQUIRE(*it == 1);
//		REQUIRE(it.getPtr()->countRef == 3);
//		REQUIRE(it.getPtr()->next->countRef == 2);
//
//		REQUIRE((*it)++ == 1);
//		REQUIRE(*it == 2);
//
//		REQUIRE(++(*it) == 3);
//
//		REQUIRE((*it)-- == 3);
//		REQUIRE(*it == 2);
//
//		REQUIRE(--(*it) == 1);
//	}
//
//	SECTION("One more erase test") {
//		ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });
//
//		auto it1 = list.begin();
//
//		it1++;
//		it1++;
//
//		auto it2 = list.begin();
//
//		it2++;
//		it2++;
//
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it2.getPtr()->countRef == 4);
//		REQUIRE(it1 == it2);
//
//		list.erase(it1);
//		REQUIRE(*it1 == 4);
//		REQUIRE(it1.getPtr()->next->val == 5);
//		REQUIRE(it1.getPtr()->prev->val == 2);
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it1.getPtr()->prev->countRef == 3);
//
//		REQUIRE(it2.getPtr()->countRef == 1);
//
//		list.erase(it1);
//		REQUIRE(*it1 == 5);
//		REQUIRE(it1.getPtr()->next->val == 6);
//		REQUIRE(it1.getPtr()->prev->val == 2);
//		REQUIRE(it1.getPtr()->prev->next->val == 5);
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it1.getPtr()->prev->countRef == 4);
//
//		REQUIRE(it2.getPtr()->next->val == 4);
//		REQUIRE(it2.getPtr()->next->next->val == 5);
//
//		REQUIRE(it2.getPtr() != nullptr);
//		REQUIRE(it2.getPtr()->next != nullptr);
//
//		it1--;
//		REQUIRE(*it1 == 2);
//		REQUIRE(it1.getPtr()->countRef == 5);
//		REQUIRE(it1.getPtr()->next->countRef == 3);
//
//		list.erase(it1);
//		REQUIRE(*it1 == 5);
//		REQUIRE(it1.getPtr()->countRef == 5);
//		REQUIRE(it1.getPtr()->prev->val == 1);
//		REQUIRE(it1.getPtr()->prev->next->val == 5);
//		REQUIRE(it1.getPtr()->prev->countRef == 3);
//
//		REQUIRE(it2.getPtr()->next->val == 4);
//		REQUIRE(it2.getPtr()->next->countRef == 1);
//		REQUIRE(it2.getPtr()->deleted);
//		REQUIRE(it2.getPtr()->prev->val == 2);
//		REQUIRE(it2.getPtr()->prev->countRef == 2);
//		REQUIRE(it2.getPtr()->deleted);
//
//		list.erase(--it1);
//		REQUIRE(*it1 == 5);
//		REQUIRE(it1.getPtr()->countRef == 6);
//		REQUIRE(it1.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it1);
//		REQUIRE(*it1 == 6);
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it1.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it1);
//		REQUIRE(it1 == list.end());
//		REQUIRE(list.empty());
//
//
//		list.erase(it2);
//		REQUIRE(it2 == list.end());
//	}
//
//	SECTION("One more erase test") {
//		ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });
//
//		auto it1 = list.begin();
//
//		it1++;
//		it1++;
//
//		auto it2 = list.begin();
//
//		it2++;
//		it2++;
//
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it2.getPtr()->countRef == 4);
//		REQUIRE(it1 == it2);
//
//		list.erase(it1);
//		REQUIRE(*it1 == 4);
//		REQUIRE(it1.getPtr()->next->val == 5);
//		REQUIRE(it1.getPtr()->prev->val == 2);
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it1.getPtr()->prev->countRef == 3);
//
//		REQUIRE(it2.getPtr()->countRef == 1);
//
//		list.erase(it1);
//		REQUIRE(*it1 == 5);
//		REQUIRE(it1.getPtr()->next->val == 6);
//		REQUIRE(it1.getPtr()->prev->val == 2);
//		REQUIRE(it1.getPtr()->prev->next->val == 5);
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it1.getPtr()->prev->countRef == 4);
//
//		REQUIRE(it2.getPtr()->next->val == 4);
//		REQUIRE(it2.getPtr()->next->next->val == 5);
//
//		REQUIRE(it2.getPtr() != nullptr);
//		REQUIRE(it2.getPtr()->next != nullptr);
//
//		it1--;
//		REQUIRE(*it1 == 2);
//		REQUIRE(it1.getPtr()->countRef == 5);
//		REQUIRE(it1.getPtr()->next->countRef == 3);
//
//		list.erase(it1);
//		REQUIRE(*it1 == 5);
//		REQUIRE(it1.getPtr()->countRef == 5);
//		REQUIRE(it1.getPtr()->prev->val == 1);
//		REQUIRE(it1.getPtr()->prev->next->val == 5);
//		REQUIRE(it1.getPtr()->prev->countRef == 3);
//
//		REQUIRE(it2.getPtr()->next->val == 4);
//		REQUIRE(it2.getPtr()->next->countRef == 1);
//		REQUIRE(it2.getPtr()->deleted);
//		REQUIRE(it2.getPtr()->prev->val == 2);
//		REQUIRE(it2.getPtr()->prev->countRef == 2);
//		REQUIRE(it2.getPtr()->deleted);
//
//		list.erase(--it1);
//		REQUIRE(*it1 == 5);
//		REQUIRE(it1.getPtr()->countRef == 6);
//		REQUIRE(it1.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it1);
//		REQUIRE(*it1 == 6);
//		REQUIRE(it1.getPtr()->countRef == 4);
//		REQUIRE(it1.getPtr()->prev == list.getBeginNode());
//
//		list.erase(it1);
//		REQUIRE(it1 == list.end());
//		REQUIRE(list.empty());
//
//
//		list.erase(it2);
//		REQUIRE(it2 == list.end());
//		
//	}
//
//
//	SECTION("insert") {
//		ConsistentList<int> list;
//		Iterator<int> it = list.end();
//		list.insert(it, 1);
//		REQUIRE(it == list.end());
//		it--;
//		REQUIRE(*it == 1);
//		list.insert(it, 2);
//		REQUIRE(*it == 1);
//		it--;
//		REQUIRE(*it == 2);
//	}
//
//
//	SECTION("multi erase") {
//		ConsistentList<int> list;
//
//		list.push_back(1);
//		list.push_back(2);
//		list.push_back(3);
//		list.push_back(4);
//		list.push_back(5);
//		list.push_back(6);
//
//		Iterator<int> iter1 = list.begin();
//
//		iter1++;
//		iter1++;
//		iter1++;
//
//		Iterator<int> iter2 = iter1;
//
//		list.erase(iter1);
//
//		REQUIRE(*iter1 == 5);
//		REQUIRE(*iter2 == 4);
//		REQUIRE(iter1.getPtr()->countRef == 4);
//		REQUIRE(iter2.getPtr()->countRef == 1);
//		REQUIRE(iter1.getPtr()->prev->countRef == 3);
//
//
//		Iterator<int> iter3 = list.begin();
//
//		iter3++;
//		iter3++;
//
//		Iterator<int> iter4 = iter3;
//
//		list.erase(iter3);
//
//		REQUIRE(*iter3 == 5);
//		REQUIRE(*iter4 == 3);
//		REQUIRE(iter3.getPtr()->countRef == 6);
//		REQUIRE(iter4.getPtr()->countRef == 2);
//		REQUIRE(iter3.getPtr()->prev->countRef == 3);
//		REQUIRE(iter3.getPtr()->prev->val == 2);
//
//		list.erase(iter3);
//
//		list.erase(iter4);
//
//		list.erase(iter2);
//
//		Iterator<int> iter5 = list.begin();
//
//		Iterator<int> iter6 = list.begin();
//
//		list.erase(iter5);
//
//		list.erase(iter5);
//
//		list.erase(iter3);
//
//		list.erase(iter2);
//
//		list.erase(iter4);
//
//		list.erase(iter6);
//
//		list.erase(iter1);
//	}
//}


int i = 0;

void write(Iterator<int>& it, ConsistentList<int>& list, std::condition_variable_any& cv, int val) {
	std::unique_lock<std::shared_mutex> lock(list.getMutex());
	//std::cout << std::thread::id. << " thread wait" << std::endl;
	cv.wait(lock);
	i++;
	*it = val;
	//lock.unlock();
}

void read(Iterator<int>& it, ConsistentList<int>& list, std::condition_variable_any& cv, std::promise<int>&& p) {
	std::shared_lock<std::shared_mutex> lock(list.getMutex());
	cv.wait(lock);
	p.set_value(*it);
}

TEST_CASE("Multi thread") {

	//SECTION("push") {
	//	ConsistentList<std::pair<int, int>> list;

	//	std::vector<std::thread> threads;

	//	int threadCount = 10;

	//	int pushCount = 20;

	//	auto push = [&](int threadNum) {
	//		for (int i = 0; i < pushCount; i++) {
	//			list.push_back(std::pair<int, int>(threadNum, i));
	//		}
	//	};

	//	for (int i = 0; i < threadCount; i++) {
	//		threads.push_back(std::thread(push, i));
	//	}

	//	for (auto& th : threads) {
	//		th.join();
	//	}

	//	REQUIRE(list.size() == pushCount * threadCount);

	//	auto it = list.begin();
	//	for (int i = 0; i < threadCount; i++) {
	//		int el = 0;
	//		for (int j = 0; j < pushCount * threadCount; j++) {
	//			if (it.get().first == i) {
	//				REQUIRE(it.get().second == el);
	//				el++;
	//			}
	//			el = 0;
	//		}
	//	}

	//}

	SECTION("HZ") {
		//ConsistentList<int> list;

		//int listSize = 100;

		//int threadCount = 20;

		//for (int i = 0; i < listSize; i++) {
		//	list.push_back(i);
		//}

		//std::latch creation_iterators{ threadCount };
		//std::latch erasing_iterators{ threadCount };

		//auto createIterators = [&]() {
		//	std::vector<Iterator<int>> iterators;
		//	for (int i = 0; i < 10; i++) {
		//		auto it = list.begin();
		//		int offset = rand() % list.size();
		//		list.advance(it, offset);
		//		iterators.push_back(it);
		//	}
		//	creation_iterators.count_down();
		//	creation_iterators.wait();

		//	for (int i = 0; i < 10; i++) {
		//		if (i % 2 == 0) {
		//			list.erase(iterators[i]);
		//		}
		//	}
		//	erasing_iterators.count_down();
		//	erasing_iterators.wait();

		//	for (auto& it : iterators) {
		//		while (it != list.end()) {
		//			it++;
		//		}
		//	}

		//};


		//std::vector<std::thread> threads;

		//for (int i = 0; i < threadCount; i++) {
		//	threads.push_back(std::thread(createIterators));
		//}

		//for (auto& th : threads) {
		//	th.join();
		//}

		//REQUIRE(list.getRealSize() == list.size());

	}

	//SECTION("change elements and read") {
	//	ConsistentList<int> list;

	//	int listSize = 20;

	//	for (int i = 0; i < listSize; i++) {
	//		list.push_back(i);
	//	}


	//	std::condition_variable_any cv;

	//	std::shared_mutex mutex;

	//	std::atomic<bool> isDataReady{ false };

	//	int threadCounts = listSize;

	//	std::atomic<int> threadsRead{ threadCounts };

	//	std::vector<std::thread> threads;

	//	std::vector<int> savedValues(threadCounts, 0);

	//	std::atomic<int> threadNumber = 0;

	//	for (int i = 0; i < threadCounts; i++) {

	//		threads.push_back(std::thread([&, i] {

	//			std::shared_lock<std::shared_mutex> lock(mutex);
	//			Iterator<int> it = list.begin();
	//			std::cout << "Wait for getting list ready..." << std::endl;
	//			cv.wait(lock, [&] { return isDataReady.load(); });
	//			list.advance(it, i);
	//			savedValues[i] = *it;
	//			threadsRead++;
	//			cv.notify_all();


	//		}));
	//	}

	//	std::thread writer([&] {
	//					
	//		std::unique_lock<std::shared_mutex> lock(mutex);
	//			
	//		for (auto it = list.begin(); it != list.end(); it++) {
	//			it.set(it.get() * 10);
	//		}
	//		threadsRead = 0;

	//		isDataReady = true;
	//		std::cout << "List is ready..." << std::endl;
	//		cv.notify_all();
	//		std::cout << "Wait for all threads read the list..." << std::endl;
	//		cv.wait(lock, [&] {return threadsRead.load() == threadCounts; });
	//		threadsRead = 0;
	//		for (int k = 0; k < savedValues.size(); k++) {
	//			REQUIRE(savedValues[k] == k * 10);
	//		}

	//	});
	//	for (auto& t : threads) {
	//		t.join();
	//	}
	//	writer.join();

	//}
	
	SECTION("insert and erase") {

		std::cout << "insert and erase test..." << std::endl;

		ConsistentList<int> list;

		std::vector<std::thread> threads;

		std::condition_variable_any cv;

		std::shared_mutex mutex;

		int threadCount = 3;


		std::atomic<int> threadsFinished{0};

		auto insertFunc = [&](int id) {
			Iterator<int> it = list.begin();
			for (int i = 0; i < threadCount; i++) {
				list.insert(it, id);
			}

			threadsFinished++;

			cv.notify_all();
		};

		auto eraseFunc = [&](int id) {
			std::shared_lock<std::shared_mutex> lock(mutex);
			cv.wait(lock, [&] { return threadsFinished == threadCount; });
			Iterator<int> it = list.begin();
			for (int i = 0; i < threadCount; i++) {
				list.erase(it);
			}
		};

		for (int i = 0; i < threadCount; i++) {
			threads.push_back(std::thread(insertFunc, i));
		}

		for (int i = 0; i < threadCount; i++) {
			threads.push_back(std::thread(eraseFunc, i));
		}

		for (auto& th : threads) {
			th.join();
		}

		for (int i = 0; i < 4; i++) {
			auto it = list.begin();
			list.erase(it);
		}

		REQUIRE(list.size() == 0);
		REQUIRE(list.getRealSize() == 0);


	}

}

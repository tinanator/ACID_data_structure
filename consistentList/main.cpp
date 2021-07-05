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

TEST_CASE("Multi thread") {

	SECTION("1") {

		std::cout << "---------TEST 1----------" << std::endl;

		int count = 100;

		for (int tc = 2; tc <= 100; tc += 10) {
			ConsistentList<int> list;

			for (int i = 0; i < count; i++) {
				list.push_back(0);
			}

			auto push = [&](int threadNum) {
				int itersCount = 10;
				std::vector<Iterator<int>> invalidIterators;
				std::vector<Iterator<int>> iterators;
				for (int i = 0; i < itersCount; i++) {
					iterators.push_back(list.begin());
				}
				int val = 0;
				int insertedCount = 1000;
				bool toStop = false;
				while (insertedCount > 0) {
					for (int i = 0; i < iterators.size(); i++) {
						if (toStop) {
							break;
						}
						int r = rand() % 3;
						switch (r) {
						case 0:
							if (iterators[i] != list.end()) {
								iterators[i]++;
							}
							break;
						case 1:
							iterators[i] = list.insert(iterators[i], val + threadNum);
							insertedCount--;
							if (insertedCount <= 0) {
								toStop = true;
							}
							std::cout << std::to_string(insertedCount) + ' ' << std::endl;
							val++;
							break;
						case 2:
							invalidIterators.push_back(iterators[i]);
							iterators[i] = list.erase(iterators[i]);
							break;
						}
					}
				}
			};

			std::vector<std::thread> threads;

			int threadCount = tc;

			auto tstart = clock();

			for (int i = 0; i < threadCount; i++) {
				threads.push_back(std::thread(push, i));
			}

			for (auto& th : threads) {
				th.join();
			}


			auto tend = clock();

			auto time = tend - tstart;

			std::cout << std::to_string(tc) + ": " + std::to_string(time) << std::endl;
		}
	}

	SECTION("2") {

		std::cout << "---------TEST 2----------" << std::endl;

		int count = 200000;

		for (int tc = 2; tc <= 100; tc += 10) {
			int threadCount = tc;

			int elemCount = count / tc;

			ConsistentList<std::pair<int, int>> list;

			auto push = [&list, elemCount](int threadNum) {
				int itersCount = elemCount;
				std::vector<Iterator<std::pair<int, int>>> iterators;
				for (int i = 0; i < itersCount; i++) {
					iterators.push_back(list.begin());
				}
				int val = elemCount * threadNum;
				while (val < elemCount * (threadNum + 1)) {
					for (auto it : iterators) {
						int i = 0;
						int r = rand() % 2;
						switch (r) {
						case 0:
						{
							if (it != list.end()) {
								it++;
							}
							break;
						}

						case 1:
						{
							list.insert(it, std::pair<int, int>(val, threadNum));
							val++;
							break;
						}

						}
						if (val > elemCount * (threadNum + 1) - 1) {
							break;
						}
					}
				}

			};

			std::vector<std::thread> threads;

			auto tstart = clock();

			for (int i = 0; i < threadCount; i++) {
				threads.push_back(std::thread(push, i));
			}

			for (auto& th : threads) {
				th.join();
			}

			auto tend = clock();

			auto time = tend - tstart;

			std::cout << std::to_string(tc) + ": " + std::to_string(time) << std::endl;

			for (int t = 0; t < threadCount; t++) {
				int val = elemCount * t;
				auto m = std::map<int, int>();
				for (int i = 0 + t * elemCount; i < (t + 1) * elemCount; i++) {
					m[i] = 0;
				}
				for (auto l : list) {
					if (t == l.second) {
						m[l.first]++;
						REQUIRE(l.first >= elemCount * t);
						REQUIRE(l.first <= elemCount * (t + 1));
					}
				}

				for (auto [key, val] : m) {
					if ((val != 1)) {
						std::cout << "aaaaaa" + '\n';
					}
					REQUIRE(val == 1);
				}

			}
		}
		
	}


	SECTION("3") {
		std::cout << "---------TEST 3----------" << std::endl;

		for (int tc = 2; tc <= 100; tc += 10) {

			int elemToInsert = 1000;
			int threadCount = tc;
			int itersCount = elemToInsert / threadCount;
			ConsistentList<int> list;

			std::latch q{ threadCount };
			std::latch r{ threadCount };

			int val = 0;
			for (int i = 0; i < threadCount * itersCount; i++) {
				list.push_back(val);
				val++;
			}

			auto push = [&list, &elemToInsert, &q, &r, &threadCount, &itersCount](int threadNum) {

				std::vector<Iterator<int>> iterators;
				for (int i = 0; i < itersCount; i++) {
					auto it = list.begin();
					list.advance(it, threadNum + i * threadCount);
					iterators.push_back(it);
					//	std::cout << '-';
				}

				q.count_down();
				q.wait();

				int val = threadNum * elemToInsert;
				for (int l = 0; l < elemToInsert; l++) {
					for (int j = 0; j < iterators.size(); j++) {
						auto it = list.insert(iterators[j], val);
						iterators[j] = it;
						val++;
					}
				}

				//r.count_down();
				//r.wait();

				std::cout << list.size();

				for (int j = 0; j < iterators.size(); j++) {
					for (int l = 0; l < elemToInsert; l++) {
						auto it = list.erase(iterators[j]);
						iterators[j] = it;
					}
				}

			};

			std::vector<std::thread> threads;

			auto tstart = clock();

			for (int i = 0; i < threadCount; i++) {
				threads.push_back(std::thread(push, i));
			}

			for (auto& th : threads) {
				th.join();
			}


			auto tend = clock();

			auto time = tend - tstart;

			std::cout << std::to_string(tc) + ": " + std::to_string(time) << std::endl;


			std::cout << "check!" << std::endl;

			REQUIRE(list.size() == threadCount * itersCount);
		}
	}
}

//TEST_CASE("Multi thread") {
//
//	SECTION("push") {
//
//		std::cout << "push..." << std::endl;
//
//		ConsistentList<std::pair<int, int>> list;
//
//		std::vector<std::thread> threads;
//
//		int threadCount = 100;
//
//		int pushCount = 2;
//
//		auto push = [&](int threadNum) {
//			for (int i = 0; i < pushCount; i++) {
//				list.push_back(std::pair<int, int>(threadNum, i));
//			}
//		};
//
//		for (int i = 0; i < threadCount; i++) {
//			threads.push_back(std::thread(push, i));
//		}
//
//		for (auto& th : threads) {
//			th.join();
//		}
//
//		REQUIRE(list.size() == pushCount * threadCount);
//
//		auto it = list.begin();
//		for (int i = 0; i < threadCount; i++) {
//			int el = 0;
//			for (int j = 0; j < pushCount * threadCount; j++) {
//				if (it.get().first == i) {
//					REQUIRE(it.get().second == el);
//					el++;
//				}
//				el = 0;
//			}
//		}
//
//	}
//
//	SECTION("insert erase and move") {
//
//		std::cout << "insert erase and move..." << std::endl;
//
//		ConsistentList<int> list;
//
//		int listSize = 100;
//
//		int threadCount = 1000;
//
//		for (int i = 0; i < listSize; i++) {
//			list.push_back(i);
//		}
//
//		std::latch creation_iterators{ threadCount };
//		std::latch erasing_iterators{ threadCount };
//
//		auto createIterators = [&]() {
//			std::vector<Iterator<int>> iterators;
//			for (int i = 0; i < 10; i++) {
//				auto it = list.begin();
//				int offset = rand() % list.size();
//				list.advance(it, offset);
//				iterators.push_back(it);
//			}
//			creation_iterators.count_down();
//			creation_iterators.wait();
//
//			for (int i = 0; i < 10; i++) {
//				if (i % 2 == 0) {
//					list.erase(iterators[i]);
//				}
//			}
//
//			erasing_iterators.count_down();
//			erasing_iterators.wait();
//			auto end = list.end();
//			for (auto& it : iterators) {
//				while (it != end) {
//					it++;
//				}
//			}
//
//		};
//
//
//		std::vector<std::thread> threads;
//
//		for (int i = 0; i < threadCount; i++) {
//			threads.push_back(std::thread(createIterators));
//		}
//
//		for (auto& th : threads) {
//			th.join();
//		}
//
//		REQUIRE(list.getRealSize() == list.size());
//
//	}
//
//	SECTION("move iterators") {
//
//		std::cout << "move iterators..." << std::endl;
//
//		ConsistentList<int> list;
//
//		int listSize = 100;
//
//		int threadCount = 1000;
//
//		int iteratorsPerThread = 10;
//
//		for (int i = 0; i < listSize; i++) {
//			list.push_back(i);
//		}
//
//		std::vector<std::thread> threads;
//
//		auto moveForward = [&]() {
//			std::vector<Iterator<int>> iters;
//			for (int i = 0; i < iteratorsPerThread; i++) {
//				auto it = list.begin();
//				int pos = rand() % list.size();
//				list.advance(it, pos);
//				iters.push_back(it);
//			}
//
//			int count = iters.size();
//
//			for (auto& it : iters) {
//				while (it != list.end()) {
//					it++;
//				}
//			}
//
//		};
//
//		auto moveBack = [&]() {
//			std::vector<Iterator<int>> iters;
//			for (int i = 0; i < iteratorsPerThread; i++) {
//				auto it = list.begin();
//				int pos = rand() % list.size();
//				list.advance(it, pos);
//				iters.push_back(it);
//			}
//
//			int count = iters.size();
//
//			for (auto& it : iters) {
//				while (it != list.begin()) {
//					it--;
//				}
//			}
//
//		};
//
//		for (int i = 0; i < threadCount / 2; i++) {
//			threads.push_back(std::thread(moveForward));
//		}
//
//		for (int i = 0; i < threadCount / 2; i++) {
//			threads.push_back(std::thread(moveBack));
//		}
//
//		for (auto& th : threads) {
//			th.join();
//		}
//
//		Iterator<int> it = list.begin();
//		it++;
//
//		while (it != list.end()) {
//			REQUIRE(it.getPtr()->countRef == 3);
//			it++;
//		}
//	}
//
//	SECTION("insert and erase") {
//
//		std::cout << "insert and erase test..." << std::endl;
//
//		ConsistentList<int> list;
//
//		std::vector<std::thread> threads;
//
//		std::condition_variable_any cv;
//
//		std::shared_mutex mutex;
//
//		int threadCount = 1000;
//
//		int elementsCount = 100;
//
//		std::atomic<int> threadsFinished{ 0 };
//
//		auto insertFunc = [&](int id) {
//			Iterator<int> it = list.begin();
//			for (int i = 0; i < elementsCount; i++) {
//				list.insert(it, id);
//			}
//
//			threadsFinished++;
//
//			cv.notify_all();
//		};
//
//		auto eraseFunc = [&](int id) {
//			std::shared_lock<std::shared_mutex> lock(mutex);
//			cv.wait(lock, [&] { return threadsFinished == threadCount; });
//			Iterator<int> it = list.begin();
//			for (int i = 0; i < elementsCount; i++) {
//				list.erase(it);
//			}
//		};
//
//		for (int i = 0; i < threadCount; i++) {
//			threads.push_back(std::thread(insertFunc, i));
//		}
//
//		for (int i = 0; i < threadCount; i++) {
//			threads.push_back(std::thread(eraseFunc, i));
//		}
//
//		for (auto& th : threads) {
//			th.join();
//		}
//
//		REQUIRE(list.size() == list.getRealSize());
//	}
//}
//
//

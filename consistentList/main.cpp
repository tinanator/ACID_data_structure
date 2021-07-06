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


TEST_CASE("Begin() End() front() back()") {
	ConsistentList<int> list;

	Iterator<int> it = list.begin();

	REQUIRE(it == list.end());

	list.push_back(1);
	list.push_back(2);
	list.push_back(3);
	list.push_back(4);

	it = list.begin();

	REQUIRE(*it == 1);

	auto val = list.front();

	REQUIRE(val == 1);

	val = list.back();

	REQUIRE(val == 4);
}


TEST_CASE("Initializing list") {
	ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });

	auto val = list.front();

	REQUIRE(val == 1);

	val = list.back();

	REQUIRE(val == 6);
}

TEST_CASE("erase") {
	ConsistentList<int> list({ 1, 2, 3, 4 });

	REQUIRE(list.size() == 4);

	Iterator<int> it = list.begin();

	it = list.erase(it);

	REQUIRE(list.size() == 3);
	REQUIRE(*it == 2);
	REQUIRE(it.getPtr()->countRef == 4);
	REQUIRE(it.getPtr()->prev == list.getBeginNode());

	it = list.erase(it);

	REQUIRE(list.size() == 2);
	REQUIRE(*it == 3);
	REQUIRE(it.getPtr()->countRef == 4);
	REQUIRE(it.getPtr()->prev == list.getBeginNode());

	it = list.erase(it);

	REQUIRE(list.size() == 1);
	REQUIRE(*it == 4);
	REQUIRE(it.getPtr()->countRef == 4);
	REQUIRE(it.getPtr()->prev == list.getBeginNode());

	it = list.erase(it);

	REQUIRE(list.size() == 0);
	REQUIRE(it == list.end());
}

TEST_CASE("inc dec") {
	ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });

	auto it = list.begin();
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->next->countRef == 2);

	it++;
	REQUIRE(*it == 2);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev->countRef == 2);

	it++;
	REQUIRE(*it == 3);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev->countRef == 2);

	++it;
	REQUIRE(*it == 4);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev->countRef == 2);

	++it;
	REQUIRE(*it == 5);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev->countRef == 2);

	it--;
	REQUIRE(*it == 4);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->next->countRef == 2);

	it--;
	REQUIRE(*it == 3);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->next->countRef == 2);

	--it;
	REQUIRE(*it == 2);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->next->countRef == 2);

	--it;
	REQUIRE(*it == 1);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->next->countRef == 2);

	REQUIRE((*it)++ == 1);
	REQUIRE(*it == 2);

	REQUIRE(++(*it) == 3);

	REQUIRE((*it)-- == 3);
	REQUIRE(*it == 2);

	REQUIRE(--(*it) == 1);

}

TEST_CASE() {
	ConsistentList<int> list({ 1, 2, 3, 4, 5, 6 });

	auto it1 = list.begin();

	it1++;
	it1++;

	auto it2 = list.begin();

	it2++;
	it2++;

	REQUIRE(it1.getPtr()->countRef == 4);
	REQUIRE(it2.getPtr()->countRef == 4);
	REQUIRE(it1 == it2);

	it1 = list.erase(it1);
	REQUIRE(*it1 == 4);
	REQUIRE(it1.getPtr()->next->val == 5);
	REQUIRE(it1.getPtr()->prev->val == 2);
	REQUIRE(it1.getPtr()->countRef == 4);
	REQUIRE(it1.getPtr()->prev->countRef == 3);

	REQUIRE(it2.getPtr()->countRef == 2);

	it1 = list.erase(it1);
	REQUIRE(*it1 == 5);
	REQUIRE(it1.getPtr()->next->val == 6);
	REQUIRE(it1.getPtr()->prev->val == 2);
	REQUIRE(it1.getPtr()->prev->next->val == 5);
	REQUIRE(it1.getPtr()->countRef == 4);
	REQUIRE(it1.getPtr()->prev->countRef == 4);

	REQUIRE(it2.getPtr()->next->val == 4);
	REQUIRE(it2.getPtr()->next->next->val == 5);

	REQUIRE(it2.getPtr() != nullptr);
	REQUIRE(it2.getPtr()->next != nullptr);

	it1--;
	REQUIRE(*it1 == 2);
	REQUIRE(it1.getPtr()->countRef == 5);
	REQUIRE(it1.getPtr()->next->countRef == 3);

	it1 = list.erase(it1);
	REQUIRE(*it1 == 5);
	REQUIRE(it1.getPtr()->countRef == 5);
	REQUIRE(it1.getPtr()->prev->val == 1);
	REQUIRE(it1.getPtr()->prev->next->val == 5);
	REQUIRE(it1.getPtr()->prev->countRef == 3);

	REQUIRE(it2.getPtr()->next->val == 4);
	REQUIRE(it2.getPtr()->next->countRef == 2);
	REQUIRE(it2.getPtr()->deleted);
	REQUIRE(it2.getPtr()->prev->val == 2);
	REQUIRE(it2.getPtr()->prev->countRef == 3);
	REQUIRE(it2.getPtr()->deleted);

	it1 = list.erase(--it1);
	REQUIRE(*it1 == 5);
	REQUIRE(it1.getPtr()->countRef == 6);
	REQUIRE(it1.getPtr()->prev == list.getBeginNode());

	it1 = list.erase(it1);
	REQUIRE(*it1 == 6);
	REQUIRE(it1.getPtr()->countRef == 4);
	REQUIRE(it1.getPtr()->prev == list.getBeginNode());

	it1 = list.erase(it1);
	REQUIRE(it1 == list.end());
	REQUIRE(list.empty());
}

TEST_CASE("Insert") {
	ConsistentList<int> list;
	Iterator<int> it = list.end();
	list.insert(it, 1);
	REQUIRE(it == list.end());
	it--;
	REQUIRE(*it == 1);
	list.insert(it, 2);
	REQUIRE(*it == 1);
	it--;
	REQUIRE(*it == 2);
}

TEST_CASE() {

	ConsistentList<int> list;

	list.push_back(1);
	list.push_back(2);
	list.push_back(3);
	list.push_back(4);
	list.push_back(5);
	list.push_back(6);

	Iterator<int> iter1 = list.begin();

	iter1++;
	iter1++;
	iter1++;

	Iterator<int> iter2 = iter1;

	iter1 = list.erase(iter1);

	REQUIRE(*iter1 == 5);
	REQUIRE(*iter2 == 4);
	REQUIRE(iter1.getPtr()->countRef == 4);
	REQUIRE(iter2.getPtr()->countRef == 2);
	REQUIRE(iter1.getPtr()->prev->countRef == 3);


	Iterator<int> iter3 = list.begin();

	iter3++;
	iter3++;

	Iterator<int> iter4 = iter3;

	iter3 = list.erase(iter3);

	REQUIRE(*iter3 == 5);
	REQUIRE(*iter4 == 3);
	REQUIRE(iter3.getPtr()->countRef == 6);
	REQUIRE(iter4.getPtr()->countRef == 3);
	REQUIRE(iter3.getPtr()->prev->countRef == 3);
	REQUIRE(iter3.getPtr()->prev->val == 2);

	list.erase(iter3);

	list.erase(iter4);

	list.erase(iter2);

	Iterator<int> iter5 = list.begin();

	Iterator<int> iter6 = list.begin();

	list.erase(iter5);

	list.erase(iter5);

	list.erase(iter3);

	list.erase(iter2);

	list.erase(iter4);

	list.erase(iter6);

	list.erase(iter1);
}

TEST_CASE("Multi thread") {
//
//	SECTION("1") {
//		std::cout << "---------TEST----------" << std::endl;
//
//		for (int tc = 2; tc <= 10; tc++) {
//
//			int elemToInsert = 1000;
//			int threadCount = tc;
//			int itersCount = elemToInsert / threadCount;
//			ConsistentList<int> list;
//
//			std::latch q{ threadCount };
//			std::latch r{ threadCount };
//
//			int val = 0;
//			for (int i = 0; i < threadCount * itersCount; i++) {
//				list.push_back(val);
//				val++;
//			}
//
//			auto push = [&list, &elemToInsert, &q, &r, &threadCount, &itersCount](int threadNum) {
//
//				std::vector<Iterator<int>> iterators;
//				for (int i = 0; i < itersCount; i++) {
//					auto it = list.begin();
//					list.advance(it, threadNum + i * threadCount);
//					iterators.push_back(it);
//				}
//
//				q.count_down();
//				q.wait();
//
//				int val = threadNum * elemToInsert;
//				for (int l = 0; l < elemToInsert; l++) {
//					for (int j = 0; j < iterators.size(); j++) {
//						auto it = list.insert(iterators[j], val);
//						iterators[j] = it;
//						val++;
//					}
//				}
//
//			//	std::cout << list.size();
//
//				for (int j = 0; j < iterators.size(); j++) {
//					for (int l = 0; l < elemToInsert; l++) {
//						auto it = list.erase(iterators[j]);
//						iterators[j] = it;
//					}
//				}
//
//			};
//
//			std::vector<std::thread> threads;
//
//			auto tstart = clock();
//
//			for (int i = 0; i < threadCount; i++) {
//				threads.push_back(std::thread(push, i));
//			}
//
//			for (auto& th : threads) {
//				th.join();
//			}
//
//
//			auto tend = clock();
//
//			auto time = tend - tstart;
//
//			std::cout << std::to_string(tc) + ": " + std::to_string(time) << std::endl;
//
//			int checkVal = 0;
//
//			for (auto& el : list) {
//				REQUIRE(checkVal == el);
//				checkVal++;
//			}
//
//			REQUIRE(list.size() == threadCount * itersCount);
//		}
//	}

	SECTION("3") {

		std::cout << "---------TEST 3----------" << std::endl;

		int count = 100;

		for (int tc = 2; tc <= 10; tc++) {
			ConsistentList<int> list;

			for (int i = 0; i < count; i++) {
				list.push_back(0);
			}

			auto push = [&](int threadNum) {
				int itersCount = 100;
				std::vector<Iterator<int>> invalidIterators;
				std::vector<Iterator<int>> iterators;
				for (int i = 0; i < itersCount; i++) {
					Iterator<int> it = list.begin();
					list.advance(it, rand() % count);
					iterators.push_back(it);
				}
				int val = 0;
				int insertedCount = 100;
				bool toStop = false;
				while (insertedCount > 0) {
					for (int i = 0; i < iterators.size(); i++) {
						if (toStop) {
							break;
						}
						int r = rand() % 4;
						switch (r) {
						case 0:
							if (iterators[i] != list.end()) {
								iterators[i]++;
							}
							break;
						case 1:
							iterators[i] = list.insert(iterators[i], val + threadNum);
							insertedCount--;
							break;
						case 2:
							invalidIterators.push_back(iterators[i]);
							iterators[i] = list.erase(iterators[i]);
							break;
						case 3:
							if (iterators[i] != list.begin()) {
								iterators[i]--;
							}
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

	/*SECTION("2") {

		std::cout << "---------TEST 2----------" << std::endl;

		int count = 200000;

		for (int tc = 2; tc <= 10; tc++) {
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

	}*/
}

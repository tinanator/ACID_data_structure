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



TEST_CASE("Multi thread") {

	SECTION("1") {
		std::cout << "---------TEST----------" << std::endl;

		for (int tc = 2; tc <= 10; tc++) {

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

			//	std::cout << list.size();

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

			int checkVal = 0;

			for (auto& el : list) {
				REQUIRE(checkVal == el);
				checkVal++;
			}

			REQUIRE(list.size() == threadCount * itersCount);
		}
	}
}

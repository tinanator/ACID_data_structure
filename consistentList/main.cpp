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

	SECTION("0") {

		std::cout << "---------TEST 1----------" << std::endl;

		PurgedList<Node<int>>* gc = new PurgedList<Node<int>>();

		ConsistentList<int> list(*gc);

		int threadsNum = 100;

		std::atomic<int> globalCount = threadsNum;

		int elemCount = 100;

		auto push = [&list, &gc, &globalCount, &elemCount]() {
			for (int i = 0; i < elemCount; i++) {
				list.push_back(i);
			}

			std::vector<Iterator<int>> iters;

			for (auto it = list.begin(); it != list.end(); it++) {
				iters.push_back(list.erase(it));
			}

			for (auto& it : iters) {
				while (it != list.end()) {
					it++;
				}
			}

			globalCount--;
		};

		auto purgeStart = [&gc, &globalCount]() {
			while (globalCount > 0) {
				gc->purge();
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			gc->purge();
		};

		std::thread gcth(purgeStart);

		std::vector<std::thread> threads;


		for (int i = 0; i < threadsNum; i++) {
			threads.push_back(std::thread(push));
		}

		for (auto& th : threads) {
			th.join();
		}

		gcth.join();

		REQUIRE(gc->insertedCount == gc->erasedCount + 1);
	}

	SECTION("2") {

		std::cout << "---------TEST 2----------" << std::endl;

		for (int tc = 2; tc <= 10; tc++) {

			int elemToInsert = 1000;
			int threadCount = tc;
			int itersCount = elemToInsert / threadCount;

			PurgedList<Node<int>>* gc = new PurgedList<Node<int>>();

			ConsistentList<int> list(*gc);

			std::latch q{ threadCount };
			std::latch r{ threadCount };

			std::atomic<int> globalCount = threadCount;

			int val = 0;
			for (int i = 0; i < threadCount * itersCount; i++) {
				list.push_back(val);
				val++;
			}

			auto push = [&list, &elemToInsert, &q, &r, &threadCount, &itersCount, &globalCount](int threadNum) {

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

				//	std::cout << std::to_string(list.size()) + " " << std::endl;

				for (int j = 0; j < iterators.size(); j++) {
					for (int l = 0; l < elemToInsert; l++) {
						auto it = list.erase(iterators[j]);
						iterators[j] = it;
					}
				}

				globalCount--;
			};

			auto purgeStart = [&gc, &globalCount]() {
				while (globalCount > 0) {
					gc->purge();
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
				gc->purge();
			};

			std::vector<std::thread> threads;

			auto tstart = clock();

			std::thread gcth(purgeStart);

			for (int i = 0; i < threadCount; i++) {
				threads.push_back(std::thread(push, i));
			}



			for (auto& th : threads) {
				th.join();
			}

			gcth.join();


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
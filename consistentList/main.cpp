#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "List.hpp"
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

TEST_CASE("Transaction") {
	SECTION("1 thread") {
		TrxManager trxManager;
		List<int> list(trxManager);

		auto w1 = trxManager.beginWrite();

		auto it = list.begin(w1);

		auto it3 = list.insert(it, 1);

		auto it2 = list.end(w1);

		list.insert(it2, 2);

		list.insert(it3, 3);

		trxManager.commit(w1);

		auto r1 = trxManager.beginRead();


		std::vector<int> ans({3, 1, 2});
		int i = 0;
		for (auto it = list.begin(r1); it != list.end(r1); it++) {
			REQUIRE(*it == ans[i]);
			i++;
		}

		auto w2 = trxManager.beginWrite();

		auto it4 = list.begin(w2);

		list.insert(it4, 4);
		i = 0;
		for (auto it = list.begin(r1); it != list.end(r1); it++) {
			REQUIRE(*it == ans[i]);
			i++;
		}

		trxManager.commit(r1);

		auto r2 = trxManager.beginRead();

		list.erase(it4);

		
		ans = std::vector({4, 3, 1, 2});
		i = 0;
		for (auto it = list.begin(r2); it != list.end(r2); it++) {
			REQUIRE(*it == ans[i]);
			i++;
		}

		trxManager.commit(r2);
		auto r3 = trxManager.beginRead();

		ans = std::vector({ 4, 1, 2 });
		i = 0;
		for (auto it = list.begin(r3); it != list.end(r3); it++) {
			REQUIRE(*it == ans[i]);
			i++;
		}

		list.erase(list.begin(w2));

		list.gc(trxManager.getActiveTxr());
		

		ans = std::vector({ 4, 1, 2 });
		i = 0;

		auto node = list.getHead();
		while (node) {
			REQUIRE(node->val == ans[i]);
			node = node->next;
			i++;
		}

		trxManager.commit(r3);
		
		trxManager.commit(w2);

		list.gc(trxManager.getActiveTxr());

		ans = std::vector({ 1, 2 });
		i = 0;
		node = list.getHead();
		while (node) {
			REQUIRE(node->val == ans[i]);
			node = node->next;
			i++;
		}

	}

	SECTION("uniq writer") {
		TrxManager trxManager;
		List<int> list(trxManager);


		auto func = [&list, &trxManager](int tnum) {
			auto trx = trxManager.beginWrite();

			for (int i = 0; i < 3; i++) {
				list.insert(list.begin(trx), i + 3 * tnum);
			}

			auto lock = std::unique_lock(list.mutex);

			int i = 0;
			auto node = list.getHead();
			while (node) {
				std::cout << node->val << " ";
				node = node->next;
				i++;
			}
			std::cout << std::endl;
			lock.unlock();

			trxManager.commit(trx);
		};

		std::vector<std::thread> threads;
		for (int i = 0; i < 5; i++) {
			threads.push_back(std::thread(func, i));
		}

		for (auto& t : threads) {
			t.join();
		}
	}

	SECTION("multi readers") {
		std::cout << "-------------" << std::endl;
		TrxManager trxManager;
		List<int> list(trxManager);

		auto func = [&list, &trxManager](int tnum) {

			auto r = trxManager.beginRead();

			auto trx = trxManager.beginWrite();

			for (int i = 0; i < 3; i++) {
				list.insert(list.begin(trx), i + 3 * tnum);
			}

			auto lock = std::unique_lock(list.mutex);

			int i = 0;
			auto node = list.getHead();
			while (node) {
				std::cout << node->val << " ";
				node = node->next;
				i++;
			}
			std::cout << std::endl;
			lock.unlock();


			i = 0;
			for (auto it = list.begin(r); it != list.end(r); it++) {
				std::cout << *it << " ";
			}

			trxManager.commit(trx);

			i = 0;
			for (auto it = list.begin(r); it != list.end(r); it++) {
				std::cout << *it << " ";
			}
		};

		std::vector<std::thread> threads;
		for (int i = 0; i < 5; i++) {
			threads.push_back(std::thread(func, i));
		}

		for (auto& t : threads) {
			t.join();
		}

	}
}
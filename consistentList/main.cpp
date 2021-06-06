#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "List.hpp"
#include "Iterator.hpp"
#include <iostream>


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
	ConsistentList<int> list({1, 2, 3, 4, 5, 6});

	auto val = list.front();

	REQUIRE(val == 1);

	val = list.back();

	REQUIRE(val == 6);
}

TEST_CASE("erase") {
	ConsistentList<int> list({ 1, 2, 3, 4 });

	REQUIRE(list.size() == 4);

	Iterator<int> it = list.begin();

	list.erase(it);

	REQUIRE(list.size() == 3);
	REQUIRE(*it == 2);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev == list.getBeginNode());

	list.erase(it);

	REQUIRE(list.size() == 2);
	REQUIRE(*it == 3);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev == list.getBeginNode());

	list.erase(it);

	REQUIRE(list.size() == 1);
	REQUIRE(*it == 4);
	REQUIRE(it.getPtr()->countRef == 3);
	REQUIRE(it.getPtr()->prev == list.getBeginNode());

	list.erase(it);

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

	list.erase(it1);
	REQUIRE(*it1 == 4);
	REQUIRE(it1.getPtr()->next->val == 5);
	REQUIRE(it1.getPtr()->prev->val == 2);
	REQUIRE(it1.getPtr()->countRef == 4);
	REQUIRE(it1.getPtr()->prev->countRef == 3);

	REQUIRE(it2.getPtr()->countRef == 1);

	list.erase(it1);
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

	list.erase(it1);
	REQUIRE(*it1 == 5);
	REQUIRE(it1.getPtr()->countRef == 5);
	REQUIRE(it1.getPtr()->prev->val == 1);
	REQUIRE(it1.getPtr()->prev->next->val == 5);
	REQUIRE(it1.getPtr()->prev->countRef == 3);

	REQUIRE(it2.getPtr()->next->val == 4);
	REQUIRE(it2.getPtr()->next->countRef == 1);
	REQUIRE(it2.getPtr()->deleted);
	REQUIRE(it2.getPtr()->prev->val == 2);
	REQUIRE(it2.getPtr()->prev->countRef == 2);
	REQUIRE(it2.getPtr()->deleted);

	list.erase(--it1);
	REQUIRE(*it1 == 5);
	REQUIRE(it1.getPtr()->countRef == 6);
	REQUIRE(it1.getPtr()->prev == list.getBeginNode());

	list.erase(it1);
	REQUIRE(*it1 == 6);
	REQUIRE(it1.getPtr()->countRef == 4);
	REQUIRE(it1.getPtr()->prev == list.getBeginNode());

	list.erase(it1);
	REQUIRE(it1 == list.end());
	REQUIRE(list.empty());

	
	list.erase(it2);
	REQUIRE(it2 == list.end());
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

	list.erase(iter1);

	REQUIRE(*iter1 == 5);
	REQUIRE(*iter2 == 4);
	REQUIRE(iter1.getPtr()->countRef == 4);
	REQUIRE(iter2.getPtr()->countRef == 1);
	REQUIRE(iter1.getPtr()->prev->countRef == 3);


	Iterator<int> iter3 = list.begin();

	iter3++;
	iter3++;

	Iterator<int> iter4 = iter3;

	list.erase(iter3);

	REQUIRE(*iter3 == 5);
	REQUIRE(*iter4 == 3);
	REQUIRE(iter3.getPtr()->countRef == 6);
	REQUIRE(iter4.getPtr()->countRef == 2);
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

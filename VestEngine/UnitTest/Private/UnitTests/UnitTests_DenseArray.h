#pragma once

#include "Framework/Test.h"

#include "Utils/DenseArray.h"

DECLARE_TEST(DenseArray_Capacity0)
{
	DenseArray<int> ints;
	ints.initialize(0);

	TEST(ints.getCapacity() == 0);
	TEST(ints.size() == 0);
}

DECLARE_TEST(DenseArray_Capacity1)
{
	DenseArray<int> ints;
	ints.initialize(1);

	TEST(ints.getCapacity() == 1);
	TEST(ints.size() == 0);
}

DECLARE_TEST(DenseArray_Capacity100)
{
	DenseArray<int> ints;
	ints.initialize(100);

	TEST(ints.getCapacity() == 100);
	TEST(ints.size() == 0);
}

DECLARE_TEST(DenseArray_AddOneElem)
{
	DenseArray<int> ints;
	ints.initialize(1);
	TEST(ints.getCapacity() == 1);
	TEST(ints.size() == 0);

	int* elem = ints.add();
	TEST(ints.size() == 1);

	*elem = 10;
	TEST(ints.at(0) == 1);
}

DECLARE_TEST(DenseArray_AddTwoElems)
{
	DenseArray<int> ints;
	ints.initialize(2);
	TEST(ints.getCapacity() == 2);
	TEST(ints.size() == 0);

	int* elem = ints.add();
	TEST(ints.size() == 1);
	*elem = 10;
	TEST(ints.at(0) == 10);

	elem = ints.add();
	TEST(ints.size() == 2);
	*elem = 20;
	TEST(ints.at(1) == 20);
}

DECLARE_TEST(DenseArray_RemoveOneElem)
{
	DenseArray<int> ints;
	ints.initialize(1);
	TEST(ints.getCapacity() == 1);
	TEST(ints.size() == 0);

	ints.remove(nullptr);
	TEST(ints.size() == 0);
}

DECLARE_TEST(DenseArray_AddAndRemove)
{
	DenseArray<int> ints;
	ints.initialize(2);
	TEST(ints.getCapacity() == 2);
	TEST(ints.size() == 0);

	int* elem = ints.add();
	TEST(ints.size() == 1);
	*elem = 10;
	TEST(ints.at(0) == 10);

	ints.remove(elem);
	TEST(ints.size() == 0);
}


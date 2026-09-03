#pragma once

#include "Framework/Test.h"

#include "Utils/DenseArray.h"

// --- Capacity
DECLARE_TEST(DenseArray_Capacity0)
{
	DenseArray<int> ints;
	ints.initialize(0);

	EXPECT(ints.getCapacity() == 0);
	EXPECT(ints.size() == 0);
}

DECLARE_TEST(DenseArray_Capacity1)
{
	DenseArray<int> ints;
	ints.initialize(1);

	EXPECT(ints.getCapacity() == 1);
	EXPECT(ints.size() == 0);
}

DECLARE_TEST(DenseArray_Capacity100)
{
	DenseArray<int> ints;
	ints.initialize(100);

	EXPECT(ints.getCapacity() == 100);
	EXPECT(ints.size() == 0);
}

// --- Add
DECLARE_TEST(DenseArray_AddOneElem)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT(ints.getCapacity() == 1);
	EXPECT(ints.size() == 0);

	int* elem = ints.add();
	EXPECT(ints.size() == 1);

	*elem = 10;
	EXPECT(ints.at(0) == 1);
}

DECLARE_TEST(DenseArray_AddTwoElems)
{
	DenseArray<int> ints;
	ints.initialize(2);
	EXPECT(ints.getCapacity() == 2);
	EXPECT(ints.size() == 0);

	int* elem = ints.add();
	EXPECT(ints.size() == 1);
	*elem = 10;
	EXPECT(ints.at(0) == 10);

	elem = ints.add();
	EXPECT(ints.size() == 2);
	*elem = 20;
	EXPECT(ints.at(1) == 20);
}

// --- Remove
DECLARE_TEST(DenseArray_RemoveNullptrNoInit)
{
	DenseArray<int> ints;
	EXPECT_EXCEPTION(ints.remove(nullptr));
}

DECLARE_TEST(DenseArray_RemoveNullptr)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT_EXCEPTION(ints.remove(nullptr));
}

DECLARE_TEST(DenseArray_RemoveElemNoAdd)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int& elem = ints.at(0);
	EXPECT_EXCEPTION(ints.remove(&elem));
}

DECLARE_TEST(DenseArray_RemoveElem0)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* elem0 = ints.add();
	EXPECT_EXCEPTION(ints.remove(elem0));
}
#pragma once

#include "Framework/Test.h"

#include "Utils/DenseArray.h"

// --- Capacity
DECLARE_TEST(DenseArray, Capacity0)
{
	DenseArray<int> ints;
	ints.initialize(0);

	EXPECT(ints.getCapacity() == 0);
	EXPECT(ints.size() == 0);
}

DECLARE_TEST(DenseArray, Capacity1)
{
	DenseArray<int> ints;
	ints.initialize(1);

	EXPECT(ints.getCapacity() == 1);
	EXPECT(ints.size() == 0);
}

DECLARE_TEST(DenseArray, Clear0)
{
	DenseArray<int> ints;
	ints.initialize(0);
	EXPECT(ints.getCapacity() == 0);
	EXPECT(ints.data() == nullptr);

	ints.clear();
	EXPECT(ints.getCapacity() == 0);
	EXPECT(ints.data() == nullptr);
}

DECLARE_TEST(DenseArray, Clear1)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT(ints.getCapacity() == 1);
	EXPECT(ints.data() != nullptr);

	ints.clear();
	EXPECT(ints.getCapacity() == 0);
	EXPECT(ints.data() == nullptr);
}

// --- Add
DECLARE_TEST(DenseArray, AddOneElem)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT(ints.getCapacity() == 1);
	EXPECT(ints.size() == 0);

	int* elem = ints.add();
	EXPECT(ints.size() == 1);
}

DECLARE_TEST(DenseArray, AddTwoElems)
{
	DenseArray<int> ints;
	ints.initialize(2);
	EXPECT(ints.getCapacity() == 2);
	EXPECT(ints.size() == 0);

	int* elem = ints.add();
	EXPECT(ints.size() == 1);

	elem = ints.add();
	EXPECT(ints.size() == 2);
}

// --- Remove
DECLARE_TEST(DenseArray, RemoveNullptrNoInit)
{
	DenseArray<int> ints;
	EXPECT_EXCEPTION(ints.remove(nullptr));
}

DECLARE_TEST(DenseArray, RemoveNullptr)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT_EXCEPTION(ints.remove(nullptr));
}

DECLARE_TEST(DenseArray, RemoveElemNoAdd)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* buffer = ints.data();
	EXPECT_EXCEPTION(ints.remove(buffer));
}

DECLARE_TEST(DenseArray, RemoveElem0FromArray1)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* elem0 = ints.add();
	ints.remove(elem0);
}

DECLARE_TEST(DenseArray, RemoveElem1FromArray1)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* elem0 = ints.add();
	EXPECT_EXCEPTION(ints.remove(elem0 + 1));
}

DECLARE_TEST(DenseArray, RemoveElem0FromArray10)
{
	DenseArray<int> ints;
	ints.initialize(10);
	ints.add();
	ints.add();
	ints.add();
	int& elem0 = ints.at(0);
	ints.remove(&elem0);
}

// --- At
DECLARE_TEST(DenseArray, AtNeg1From0)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT_EXCEPTION(ints.at(-1));
}

DECLARE_TEST(DenseArray, At0From0)
{
	DenseArray<int> ints;
	ints.initialize(1);
	EXPECT_EXCEPTION(ints.at(0));
}

DECLARE_TEST(DenseArray, At0From1)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* elem0 = ints.add();
	*elem0 = 10;
	int& elem0Value = ints.at(0);
	EXPECT(elem0Value == 10);
}

DECLARE_TEST(DenseArray, At10From1)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* elem0 = ints.add();
	EXPECT_EXCEPTION(ints.at(10));
}

// --- Last
DECLARE_TEST(DenseArray, LastFrom0)
{
	DenseArray<int> ints;
	ints.initialize(0);
	EXPECT_EXCEPTION(ints.last());
}

DECLARE_TEST(DenseArray, LastFrom1)
{
	DenseArray<int> ints;
	ints.initialize(1);
	int* elem0 = ints.add();
	*elem0 = 55;
	EXPECT(ints.last() == 55);
}
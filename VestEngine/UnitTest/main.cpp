#include "Utils/DenseArray.h"

#include <iostream>

int main()
{
	DenseArray<int> lol;
	lol.initialize(5);
	int* lol1 = lol.add();
	*lol1 = 5;

	std::cout << *lol1 << std::endl;

	return 0;
}
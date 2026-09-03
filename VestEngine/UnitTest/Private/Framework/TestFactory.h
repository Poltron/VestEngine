#pragma once

#include <iostream>

class TestFactoryBase
{
public:
	virtual void runTest();
};

template <typename T>
class TestFactory : public TestFactoryBase
{
public:
	virtual void runTest() override;

private:
	T* create();
	void destroy(T* inInstance);
};


// .inl
template<typename T>
void TestFactory<T>::runTest()
{
	T* test = create();

	try
	{
		test->run();
		std::cout << "\t" << test->name << std::endl;
	}
	catch (const std::runtime_error& error)
	{
		std::cerr << "\t" << test->name << "failed with crash : " << std::endl << "\t" << error.what();
		return;
	}

	destroy(test);
}

template<typename T>
T* TestFactory<T>::create()
{
	return new T();
}

template<typename T>
void TestFactory<T>::destroy(T* inInstance)
{
	delete inInstance;
}
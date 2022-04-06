#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "gtest/gtest.h"

class MyTest : public ::testing::Test
{
public:
	MyTest()
	{
		std::cout << "MyTest" << std::endl;
	}
	~MyTest()
	{
		std::cout << "~MyTest" << std::endl;
	}

	void SetUp() override
	{
		std::cout << "SetUp" << std::endl;
	}

	void TearDown() override
	{
		std::cout << "TearDown" << std::endl;
	}

	void push_back(uint32_t data)
	{
		m_data.push_back(data);
	}

	uint32_t size()
	{
		return m_data.size();
	}

	std::vector<uint32_t>& data()
	{
		return m_data;
	}

private:
	std::vector<uint32_t> m_data;
};


TEST_F(MyTest, Sample1) {
	
	data().push_back(1);
	data().push_back(2);

	EXPECT_EQ(data().size(), 2);
}

TEST_F(MyTest, Sample2) {

	data().push_back(1);
	data().push_back(2);
	data().push_back(3);

	EXPECT_EQ(data().size(), 3);
}
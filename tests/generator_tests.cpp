#include <gtest/gtest.h>

#include <sstream>

#include <vypcomp/ir/ir.h>

using namespace ::testing;

using namespace vypcomp;

class GeneratorTests : public Test {};

TEST_F(GeneratorTests, firstTest)
{
    std::cout << "success?" << std::endl;
}

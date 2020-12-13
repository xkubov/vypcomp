#include <gtest/gtest.h>

#include <sstream>

#include <vypcomp/generator/generator.h>

using namespace ::testing;

using namespace vypcomp;

class GeneratorTests : public Test {};

TEST_F(GeneratorTests, firstTest)
{
    std::ostringstream out;
    Generator gen(true);
    ASSERT_NO_THROW(gen.generate(nullptr, out));
}

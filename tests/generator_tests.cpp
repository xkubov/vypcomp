/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

#include <gtest/gtest.h>

#include <sstream>

#include <vypcomp/generator/generator.h>

using namespace ::testing;

using namespace vypcomp;

class GeneratorTests : public Test {};

TEST_F(GeneratorTests, firstTest)
{
    Generator gen(std::make_unique<std::ostringstream>(), true);
    const auto& result_out = gen.get_output();
    ASSERT_NO_THROW(gen.generate(vypcomp::SymbolTable()));
}

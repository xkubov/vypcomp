#include <gtest/gtest.h>

#include <sstream>

#include "vypcomp/parser/parser.h"

using namespace ::testing;

using namespace vypcomp;

class ParserTests : public Test {};

TEST_F(ParserTests, emptyInput)
{
	std::stringstream input;
	std::ostringstream output;

	LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
	ASSERT_NO_THROW(parser.generateOutput(output));
	ASSERT_TRUE(output.str().empty());
}

TEST_F(ParserTests, invalidFile)
{
	LangParser parser;
	ASSERT_THROW(parser.parse("pls_dont_create_file_with_this_name"), std::runtime_error);
}

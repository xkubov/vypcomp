#include <gtest/gtest.h>

#include <sstream>

#include "vypcomp/parser/parser.h"

using namespace ::testing;

using namespace vypcomp;

class ScannerTests : public Test {
public:
	std::vector<Parser::token_type> scanInput(std::istream& input) {
		Scanner scanner(input);

		std::vector<Parser::token_type> tokens;
		Parser::token_type token;

		do {
			Parser::semantic_type type;
			Parser::location_type location;
			token = Parser::token_type(scanner.yylex(&type, &location));
			tokens.push_back(token);
		}
		while (token != Parser::token::END);

		tokens.pop_back();
		return tokens;
	}
};

TEST_F(ScannerTests, empty)
{
	std::stringstream in("");
	ASSERT_TRUE(scanInput(in).empty());
}

TEST_F(ScannerTests, blanks)
{
	std::stringstream in("\t\t   \n \n\t   \t\t\t  ");
	ASSERT_TRUE(scanInput(in).empty());
}

TEST_F(ScannerTests, keywords)
{
	std::map<std::string, Parser::token_type> keywords = {
		{"class", Parser::token::CLASS},
		{"else", Parser::token::ELSE},
		{"if", Parser::token::IF},
		{"int", Parser::token::INT},
		{"new", Parser::token::NEW},
		{"return", Parser::token::RETURN},
		{"string", Parser::token::STRING},
		{"super", Parser::token::SUPER},
		{"this", Parser::token::THIS},
		{"void", Parser::token::VOID},
		{"while", Parser::token::WHILE}
	};
	for (auto [kw, token]: keywords) {
		std::stringstream in(kw);
		std::vector<Parser::token_type> out{token};
		ASSERT_EQ(scanInput(in), out);
	}
}

TEST_F(ScannerTests, keywords_sequence)
{
	std::map<std::string, Parser::token_type> keywords = {
		{"class", Parser::token::CLASS},
		{"else", Parser::token::ELSE},
		{"if", Parser::token::IF},
		{"int", Parser::token::INT},
		{"new", Parser::token::NEW},
		{"return", Parser::token::RETURN},
		{"string", Parser::token::STRING},
		{"super", Parser::token::SUPER},
		{"this", Parser::token::THIS},
		{"void", Parser::token::VOID},
		{"while", Parser::token::WHILE}
	};

	std::vector<Parser::token_type> tokens;
	std::ostringstream out;

	for (auto [kw, token]: keywords) {
		out << kw << " ";
		tokens.push_back(token);
	}

	std::stringstream in(out.str());
	ASSERT_EQ(scanInput(in), tokens);
}

TEST_F(ScannerTests, lexicalError)
{
	std::vector<std::string> errors = {
		"0asdf"
	};

	for (auto str: errors) {
		std::stringstream in(str);
		ASSERT_THROW(scanInput(in), LexicalError);
	}
}

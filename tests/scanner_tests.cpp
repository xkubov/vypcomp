#include <gtest/gtest.h>

#include <sstream>

#include "vypcomp/parser/parser.h"

using namespace ::testing;

using namespace vypcomp;

class ScannerTests : public Test {
public:
	using Input2Token = std::map<std::string, Parser::token_type>;

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

	void expectValid(const Input2Token &inputs) {
		std::vector<Parser::token_type> tokens;
		std::ostringstream out;

		for (auto [kw, token]: inputs) {
			out << kw << " ";
			tokens.push_back(token);
		}

		std::stringstream in(out.str());
		std::vector<Parser::token_type> scanned;

		ASSERT_NO_THROW(scanned = scanInput(in));
		ASSERT_EQ(scanned, tokens);
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

TEST_F(ScannerTests, suppotComments)
{
	std::stringstream in("/*\n\n\nFine\n\n*/\n\n//Comment\n\n");
	ASSERT_TRUE(scanInput(in).empty());
}

TEST_F(ScannerTests, keywords)
{
	std::map<std::string, Parser::token_type> keywords = {
		{"class", Parser::token::CLASS},
		{"else", Parser::token::ELSE},
		{"if", Parser::token::IF},
		{"new", Parser::token::NEW},
		{"return", Parser::token::RETURN},
		{"super", Parser::token::SUPER},
		{"this", Parser::token::THIS},
		{"void", Parser::token::VOID},
		{"while", Parser::token::WHILE}
	};
	for (auto [kw, token]: keywords) {
		std::stringstream in(kw);
		std::vector<Parser::token_type> out{token};
		std::vector<Parser::token_type> scanned;
		ASSERT_NO_THROW(scanned = scanInput(in));
		ASSERT_EQ(scanned, out);
	}
}

TEST_F(ScannerTests, supportKeywordSequences)
{
	Input2Token keywords {
		{"class", Parser::token::CLASS},
		{"else", Parser::token::ELSE},
		{"if", Parser::token::IF},
		{"new", Parser::token::NEW},
		{"return", Parser::token::RETURN},
		{"super", Parser::token::SUPER},
		{"this", Parser::token::THIS},
		{"void", Parser::token::VOID},
		{"while", Parser::token::WHILE}
	};

	expectValid(keywords);
}

TEST_F(ScannerTests, suppotDatatypes)
{
	Input2Token dataTypes {
		{"int", Parser::token::INT},
		{"string", Parser::token::STRING},
		{"float", Parser::token::FLOAT}
	};

	expectValid(dataTypes);
}

TEST_F(ScannerTests, suppotIdentifiers)
{
	Input2Token identifiers {
		{"peto", Parser::token::IDENTIFIER},
		{"je", Parser::token::IDENTIFIER},
		{"borec", Parser::token::IDENTIFIER},
		{"_", Parser::token::IDENTIFIER},
		{"____", Parser::token::IDENTIFIER},
		{"_0", Parser::token::IDENTIFIER},
		{"_OkK", Parser::token::IDENTIFIER},
		{"Class", Parser::token::IDENTIFIER},
		{"Else", Parser::token::IDENTIFIER},
		{"If", Parser::token::IDENTIFIER},
		{"Int", Parser::token::IDENTIFIER},
		{"New", Parser::token::IDENTIFIER},
		{"Return", Parser::token::IDENTIFIER},
		{"String", Parser::token::IDENTIFIER},
		{"Super", Parser::token::IDENTIFIER},
		{"This", Parser::token::IDENTIFIER},
		{"Void", Parser::token::IDENTIFIER},
		{"While", Parser::token::IDENTIFIER},
		{"zjedolSomJedlo32123412okk", Parser::token::IDENTIFIER}
	};

	expectValid(identifiers);
}

TEST_F(ScannerTests, lexicalErrorInvalidIdentifier)
{
	std::vector<std::string> errors = {
		"0asdf"
	};

	for (auto str: errors) {
		std::stringstream in(str);
		ASSERT_THROW(scanInput(in), LexicalError);
	}
}

TEST_F(ScannerTests, suppotLiterals)
{
	Input2Token literals {
		{R"(Ez literal)", Parser::token::LITERAL},
		{R"(With new line \nliteral)", Parser::token::LITERAL},
		{R"(With tabs \t\tok)", Parser::token::LITERAL},
		{R"(With escaped escapes \\ \\ \\)", Parser::token::LITERAL},
		{R"(With escaped escaped \"quotes\")", Parser::token::LITERAL}
	};

	expectValid(literals);
}

// TODO:
//  - expressions (operators)
//  - brackets
//  - semicolons, colons, commas

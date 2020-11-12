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
		{"int", Parser::token::DATA_TYPE},
		{"string", Parser::token::DATA_TYPE},
		{"float", Parser::token::DATA_TYPE}
	};

	expectValid(dataTypes);
}

TEST_F(ScannerTests, suppotIdentifiers)
{
	std::vector<std::string> identifiers {
		"peto" ,"je" ,"borec" ,"_" ,"____" ,"_0"
		,"_OkK" ,"Class" ,"Else" ,"If" ,"Int" ,"New"
		,"Return" ,"String" ,"Super" ,"This" ,"Void"
		,"While" ,"zjedolSomJedlo32123412okk"
	};

	Parser::semantic_type type;
	Parser::location_type location;
	Parser::token_type token;

	for (auto id: identifiers) {
		std::stringstream str(id);
		Scanner scanner(str);

		ASSERT_NO_THROW(
			token = Parser::token_type(
				scanner.yylex(&type, &location)
			)
		);
		ASSERT_EQ(token, Parser::token::IDENTIFIER);
		ASSERT_TRUE(std::holds_alternative<std::string>(type.value));
		std::string holds = std::get<std::string>(type.value);
		ASSERT_EQ(id, holds);
	}
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

TEST_F(ScannerTests, suppotStringLiterals)
{
	std::map<std::string, std::string> stringLiteral {
		{R"("Ez literal")", "Ez literal"},
		{R"("With new line \nliteral")", "With new line \nliteral"},
		{R"("With tabs \t\tok")", "With tabs \t\tok"},
		{R"("With escaped escapes \\ \\ \\")", "With escaped escapes \\ \\ \\"},
		{R"("With escaped escaped \"quotes\"")", "With escaped escaped \"quotes\""}
	};

	Parser::semantic_type type;
	Parser::location_type location;
	Parser::token_type token;

	for (auto [in, exp]: stringLiteral) {
		std::stringstream str(in);
		Scanner scanner(str);

		ASSERT_NO_THROW(
			token = Parser::token_type(
				scanner.yylex(&type, &location)
			)
		);
		ASSERT_EQ(token, Parser::token::LITERAL);
		ASSERT_TRUE(std::holds_alternative<std::string>(type.value));
		std::string holds = std::get<std::string>(type.value);
		ASSERT_EQ(exp, holds);
	}
}

TEST_F(ScannerTests, suppotIntLiterals)
{
	std::map<std::string, unsigned long long> intLiteral {
		{"1", 1},
		{"123", 123},
		{"00213", 213}
	};

	Parser::semantic_type type;
	Parser::location_type location;
	Parser::token_type token;

	for (auto [in, exp]: intLiteral) {
		std::stringstream str(in);
		Scanner scanner(str);

		ASSERT_NO_THROW(
			token = Parser::token_type(
				scanner.yylex(&type, &location)
			)
		);
		ASSERT_EQ(token, Parser::token::LITERAL);
		ASSERT_TRUE(std::holds_alternative<decltype(exp)>(type.value));
		auto holds = std::get<decltype(exp)>(type.value);
		ASSERT_EQ(exp, holds);
	}
}

TEST_F(ScannerTests, lexicalErrorInvalidLiterals)
{
	std::map<std::string, unsigned long long> intLiteral {
		{"1a", 1},
		{"123q", 123},
		{"00213u ", 213}
	};

	Parser::semantic_type type;
	Parser::location_type location;
	Parser::token_type token;

	for (auto [in, exp]: intLiteral) {
		std::stringstream str(in);
		Scanner scanner(str);

		ASSERT_THROW(
			token = Parser::token_type(
				scanner.yylex(&type, &location)
			),
			LexicalError
		);
	}
}

// TODO:
//  - expressions (operators)
//  - brackets
//  - semicolons, colons, commas

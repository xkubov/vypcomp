#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "bison_parser.tab.hpp"
#include "location.hh"

namespace vypcomp {

class Scanner : public yyFlexLexer{
public:
	Scanner(std::istream &in) 
		: yyFlexLexer(&in)
	{};
	Scanner(std::istream &in, Parser::token::token_kind_type start_token)
		: yyFlexLexer(&in), start_token(start_token)
	{};
	virtual ~Scanner() {};

	// We want to use differeny yylex with yacc.
	using yyFlexLexer::yylex;
	virtual int yylex(
		Parser::semantic_type *lval,
		Parser::location_type *location
	);

private:
	Parser::semantic_type *yylval = nullptr;
	bool first_token = true;
	Parser::token::token_kind_type start_token = Parser::token::PROGRAM_START;
};

class LexicalError: public std::exception {
public:
	LexicalError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

}

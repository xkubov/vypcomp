#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "bison_parser.tab.hpp"
#include "location.hh"

namespace vypcomp {

class Scanner : public yyFlexLexer{
public:
	Scanner(std::istream *in) : yyFlexLexer(in) {};
	virtual ~Scanner() {};

	virtual int yylex(
		Parser::semantic_type *const lval,
		Parser::location_type *location
	);

private:
	Parser::semantic_type *yylval = nullptr;
};

}

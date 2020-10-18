%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 

%define api.namespace {vypcomp}
%define api.parser.class {Parser}

%code requires {
	namespace vypcomp {
		class Scanner;
		class LangParser;
	}

}

%parse-param {
	Scanner  &scanner
}
%parse-param {
	LangParser  &parser
}

%code {

#include "vypcomp/parser/parser.h"
#include "vypcomp/parser/scanner.h"

#undef yylex
#define yylex scanner.yylex

}

%define api.value.type variant
%define parse.assert

%token END 0 "end of file"

%locations

%%

start : END

%%


void vypcomp::Parser::error(const location_type &l, const std::string &err_message)
{
	std::cerr << "error: " << err_message << " (" << l << ")" << std::endl;
}

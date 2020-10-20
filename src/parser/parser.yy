%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines

%define api.namespace {vypcomp}
%define api.parser.class {Parser}

%code requires {

	#include <variant>

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

#include <sstream>

#include "vypcomp/parser/parser.h"
#include "vypcomp/parser/scanner.h"

#undef yylex
#define yylex scanner.yylex

}

%define api.value.type {std::variant<std::string, int>}
%define parse.assert

%token END 0 "The End"
%token <std::string> WORD
%token CLASS
%token ELSE
%token IF
%token INT
%token NEW
%token RETURN
%token STRING
%token SUPER
%token THIS
%token VOID
%token WHILE

%locations

%%

start : keyword start
      | END

keyword : CLASS
	| ELSE
	| IF
	| INT
	| NEW
	| RETURN
	| STRING
	| SUPER
	| THIS
	| VOID
	| WHILE

%%


void vypcomp::Parser::error(const location_type &l, const std::string &err_message)
{
	std::ostringstream err;
	err << err_message << " (" << l << ")";
	throw ParserError(err.str());
}

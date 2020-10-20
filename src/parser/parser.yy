%require  "3.0"

%skeleton "lalr1.cc"

// Enable tracing
// https://www.gnu.org/software/bison/manual/html_node/Tracing.html
%define parse.trace

// Define namespace for our parser.
%define api.namespace {vypcomp}

// Define class of our parser.
%define api.parser.class {Parser}

%code requires {
	// For our plymorfous token.
	#include <variant>

	// We want to use these classes during parsing.
	namespace vypcomp {
		class Scanner;
		class LangParser;
	}

}

// First constructor parameter.
%parse-param {
	Scanner  &scanner
}
// Second constructor parameter.
%parse-param {
	LangParser  &parser
}

%code {

#include <sstream>

#include "vypcomp/parser/parser.h"
#include "vypcomp/parser/scanner.h"

// We have custom yylex.
#undef yylex
#define yylex scanner.yylex

}

// Define token type
%define api.value.type {std::variant<std::string, int>}

// To correctly destroy symbols.
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

/**
 * @brief Provides custom error handling for syntax errors.
 */
void vypcomp::Parser::error(const location_type &l, const std::string &err_message)
{
	std::ostringstream err;
	err << err_message << " (" << l << ")";
	throw SyntaxError(err.str());
}

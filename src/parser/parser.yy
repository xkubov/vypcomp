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
	#include <vector>
	#include <optional>
	#include <tuple>
	#include <string>

	// We want to use these classes during parsing.
	namespace vypcomp {
		class Scanner;
		class LangParser;
	}

	/**
	 * Provides datatype options.
	 */
	enum class Datatype : int {
		String,
		Float,
		Int
	};
	using Declaration = std::pair<Datatype, std::string>;
	using Arglist = std::vector<Declaration>;
	using PossibleDatatype = std::optional<Datatype>;

	using TokenImpl = std::variant<
		std::string,
		unsigned long long,
		Datatype,
		Arglist,
		Declaration,
		PossibleDatatype
	>;

	struct Token
	{
		TokenImpl value;

		template<typename T>
		Token& operator=(const T& t) {
			value = t;
			return *this;
		}
		
		template<typename T>
		T& nonterminal() {
			if (!std::holds_alternative<T>(value)) {
				value = T{};
			}
			
			return std::get<T>(value);
		}

		template<typename T>
		T& terminal() {
			if (!std::holds_alternative<T>(value)) {
				throw std::runtime_error("Expected different type of token.");
			}
			
			return std::get<T>(value);
		}
	};
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
%define api.value.type {Token}

// To correctly destroy symbols.
%define parse.assert

%token END 0 "The End"
%token <std::string> WORD
%token CLASS
%token ELSE
%token IF
%token NEW
%token RETURN
%token SUPER
%token THIS
%token VOID
%token WHILE

%token <terminal<Datatype>()> DATA_TYPE

%token <terminal<std::string>()>IDENTIFIER

%token LITERAL

%token LPAR
%token RPAR
%token RBRA
%token LBRA
%token COMMA

%locations

%nterm <nonterminal<PossibleDatatype>()> decl_type
%nterm <nonterminal<Declaration>()> decl
%nterm <nonterminal<Arglist>()> args
%nterm <nonterminal<Arglist>()> more_args
%nterm <nonterminal<Arglist>()> arg_list

%%

start : function_definition start
      | class_declaration start
      | eof
      ;

eof : END { /*TODO: check defined main*/ (void)parser;}
    ;

function_definition : function_declaration LBRA function_body
		    ;

function_declaration : decl_type IDENTIFIER LPAR arg_list { }
		     ;

function_body : statement function_body
	      | RBRA
	      ;

statement : {}
	  ;

arg_list : VOID RPAR { $$ = {}; }
	 | args { $$ = $1; }
	 ;

args : decl more_args { $2.insert($2.begin(), $1); $$ = $2; }
     ;

more_args: COMMA decl more_args { $3.insert($3.begin(), $2); $$ = std::move($3); }
	 | RPAR {$$ = {};}
	 ;

decl : DATA_TYPE IDENTIFIER { $$ = {$1, $2}; }
     ;

class_declaration : CLASS
		  ;

decl_type : DATA_TYPE { $$ = $1; }
	  | VOID { $$ = PossibleDatatype(); }
	  ;

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

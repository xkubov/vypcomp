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
	#include <iostream>
	#include <variant>
	#include <string>

	#include "vypcomp/ir/instructions.h"

	namespace vypcomp {

	using namespace ir;

	// We want to use these classes during parsing.
	class Scanner;
	class LangParser;

	/**
	 * Token holds one of the following types:
	 */
	using TokenImpl = std::variant<
		std::string,
		unsigned long long,
		Datatype,
		Arglist,
		Declaration,
		PossibleDatatype,
		Instruction::Ptr,
		BasicBlock::Ptr,
		Function::Ptr
	>;

	/**
	 * Provides wrapper around token for eazy intergration
	 * with bison. As type of tokens/nonterminals
	 * you must specify function call (either nonterminal/terminal).
	 *
	 * Example:
	 * To create a nonterminal with type string:
	 * ```
	 *     %nterm <nonterminal<std::string>()> example
	 * ```
	 */
	struct Token
	{
		TokenImpl value;

		template<typename T>
		Token& operator=(const T& t) {
			value = t;
			return *this;
		}

		/**
		 * Provides "datatype" to be declared for nonterminals.
		 * Nonterminals are spectial in form that we do not want
		 * to exception happen if it does not holds correct type.
		 * Nonterminals are initialized in this module and only
		 * in this module.
		 * Each semantic rule recursively goes through derivation tree
		 * and at the leaf level constructs new nonterminal's value.
		 * During descending the nonterminal has initial - not initialized
		 * value.
		 */
		template<typename T>
		T& nonterminal() {
			if (!std::holds_alternative<T>(value)) {
				value = T{};
			}

			return std::get<T>(value);
		}

		/**
		 * Terminals are initialized by scanner. This means that if they do not hold
		 * required value it is really bad - scanner is malfunctioning.
		 */
		template<typename T>
		T& terminal() {
			if (!std::holds_alternative<T>(value)) {
				throw std::runtime_error("Expected different type of token.");
			}

			return std::get<T>(value);
		}
	};
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
%token FOR

%token <terminal<Datatype>()> DATA_TYPE

%token <terminal<std::string>()>IDENTIFIER

%token LITERAL

%token LPAR
%token RPAR
%token RBRA
%token LBRA
%token COMMA
%token ASSIGNMENT

%token SEMICOLON

%locations

%nterm <nonterminal<PossibleDatatype>()> decl_type
%nterm <nonterminal<Declaration>()> decl
%nterm <nonterminal<Arglist>()> args
%nterm <nonterminal<Arglist>()> more_args
%nterm <nonterminal<Arglist>()> arg_list
%nterm <nonterminal<Function::Ptr>()> function_declaration
%nterm <nonterminal<BasicBlock::Ptr>()> function_body
%nterm <nonterminal<BasicBlock::Ptr>()> basic_block
%nterm <nonterminal<BasicBlock::Ptr>()> new_block
%nterm <nonterminal<Instruction::Ptr>()> statement

%%

start : function_definition start
      | class_declaration start
      | eof
      ;

eof : END {
	parser.ensureMainDefined();
};

function_definition : function_declaration LBRA function_body {
	$1->setFirst($3);

	for (auto it = $1->first(); it != nullptr; it = it->next()) {
		std::cout << "basic block: " << it->name() << std::endl;
	}
};

function_declaration : decl_type IDENTIFIER LPAR arg_list {
	Function::Ptr fun(new Function({ $1, $2, $4 }));
	parser.addFunction(fun);
	$$ = fun;
};

function_body : basic_block function_body { $1->setNext($2); $$ = $1; }
	  | RBRA { $$ = nullptr; }
	  ;

basic_block : statement basic_block { $2->addFirst($1) ; $$ = $2; }
	    | new_block { auto bb = BasicBlock::create(); bb->setNext($1); $$ = bb;}
	    ;

new_block : RBRA { $$ = nullptr; }
	  | IF  { $$ = BasicBlock::create(); }
	  ;

statement : RETURN { $$ = nullptr; }
	  | assignment_or_function_call { $$ = nullptr; }
	  | declaration {$$ = nullptr; }
	  ;

assignment_or_function_call : IDENTIFIER
			    ;

declaration : decl IDENTIFIER optional_assignment
	    ;

optional_assignment : SEMICOLON
		    ;

arg_list : VOID RPAR { $$ = {}; }
	 | args { $$ = $1; }
	 ;

args : decl more_args { $2.insert($2.begin(), $1); $$ = std::move($2); }
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

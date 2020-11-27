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
	class ParserDriver;

	/**
	 * Token holds one of the following types:
	 */
	using TokenImpl = std::variant<
		std::string,
		unsigned long long,
		float,
		Datatype,
		Arglist,
		Declaration,
		PossibleDatatype,
		Instruction::Ptr,
		OptLiteral,
		std::vector<Instruction::Ptr>,
		std::vector<std::pair<std::string, OptLiteral>>,
		BasicBlock::Ptr,
		Function::Ptr,
		Class::Ptr
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
	ParserDriver* parser
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

%token <terminal<std::string>()>STRING_LITERAL
%token <terminal<unsigned long long>()>INT_LITERAL
%token <terminal<float>()>FLOAT_LITERAL

%token LPAR
%token RPAR
%token RBRA
%token LBRA
%token COMMA
%token ASSIGNMENT

%token SEMICOLON
%token COLON

%locations

%nterm <nonterminal<PossibleDatatype>()> optional_type
%nterm <nonterminal<Declaration>()> decl
%nterm <nonterminal<Arglist>()> args
%nterm <nonterminal<Arglist>()> more_args
%nterm <nonterminal<Arglist>()> arg_list
%nterm <nonterminal<Function::Ptr>()> function_declaration
%nterm <nonterminal<BasicBlock::Ptr>()> function_body
%nterm <nonterminal<BasicBlock::Ptr>()> basic_block
%nterm <nonterminal<BasicBlock::Ptr>()> else
%nterm <nonterminal<BasicBlock::Ptr>()> end_of_block
%nterm <nonterminal<std::vector<Instruction::Ptr>>()> statement
%nterm <nonterminal<std::vector<Instruction::Ptr>>()> expr
%nterm <nonterminal<OptLiteral>()> optional_assignment
%nterm <nonterminal<OptLiteral>()> literal
%nterm <nonterminal<std::vector<Instruction::Ptr>>()> declaration
%nterm <nonterminal<Instruction::Ptr>()> return
%nterm <nonterminal<Class::Ptr>()> class_declaration
%nterm <nonterminal<std::vector<std::pair<std::string, OptLiteral>>>()> id2init
%nterm <nonterminal<std::vector<std::pair<std::string, OptLiteral>>>()> at_least_one_id

%%

/**
 * @brief Parse start.
 *
 * On global scale module consists of function and classes.
 * Optionaly we can support global variables.
 */
start : function_definition start
      | class_definition start
      | eof
      ;

/**
 * Flex is programmed to return END token at the end of the
 * input.
 */
eof : END {
	parser->ensureMainDefined();
};

/**
 * @brief Parses function definition.

 * Function definition consists of function declaration (see below)
 * and function body. Function body is stream of basic blocks.
 * At the execution of the code of the function definition we
 * have function declaratino parsed and we can assign the body
 * to the function.
 */
function_definition : function_declaration function_body {
	// Sets body of the function. Body is just a pointer
	// To the first basic block.
	$1->setFirst($2);
	// Ends parsing of the function. This is needed because
	// The way parser works. We create new symbol table for
	// function scope and we need to get rid of it. We
	// might have as well delete the symbol table in this place
	// but I found it to be dubious and all the logic is in parseEnd.
	parser->parseEnd();
};

/**
 * @brief Function declaration.

 * Function has a type or is void, identifier and argument list.
 * Argument list is returned as array of declarations. We want
 * to transforme these declaration in instructions. This is done in
 * parseStart as well as other stuff that is needed on the start of the function.
 * -> like pushing new symbol table for local scope.
 */
function_declaration : optional_type IDENTIFIER LPAR arg_list {
	// Creates new funcion.
	$$ = parser->newFunction({ $1, $2, $4 });
	parser->parseStart($$);
};

/**
 * function body is defined as:
 * ```
 * {
 *     basic_block0,
 *     basic_block1,
 *     ...
 *     end_basic_bock
 * ```

 * All basic blocks are connected. You can see that there is not ending bracket.
 * That is parsed as end of basic block -> last one.
 */
function_body : LBRA basic_block { $$ = $2; }
	      ;

/**
 * @brief Parses basic block.
 *
 * Basic block consists of statements. As some statemetns might be translated into
 * more instructions we want to represetn statment as vector. That is why we must
 * iterate through statemetns and add them to block one by one. It might not be
 * clear at the first glance but parser firstly travels to the last statement. Then
 * it creates new basic block and all correct statements appends to the beginning. That
 * is why we append each statemetn to the beginning (in reverse).

 * On block end we create block and assign pointer to the next block.
 */
basic_block : statement basic_block {
	for (auto it = $1.rbegin(); it != $1.rend(); it++) {
		$2->addFirst(*it);
	}

	$$ = $2;
}
| end_of_block {
	$$ = BasicBlock::create();
	$$->setNext($1);
};


/**
 * Definition of end of the block.
 */
end_of_block : RBRA {
	$$ = nullptr;
}
| IF LPAR expr RPAR LBRA basic_block else basic_block {
	// TODO: process condition.
	auto br = BranchInstruction::Ptr(new BranchInstruction($6, $8));
	$8->addFirst(br);
	$$ = $8;
};

else : ELSE LBRA basic_block {
};

/**
 * Statemetn definition.
 */
statement : return {
	$$ = {$1};
}
| IDENTIFIER ASSIGNMENT expr SEMICOLON {
	throw std::runtime_error("Assignment not implemented.");
}
| IDENTIFIER LPAR expr RPAR SEMICOLON {
	throw std::runtime_error("Function calls not implemented.");
}
| declaration {
	$$ = $1;
};

expr : IDENTIFIER {std::cerr << "Expressions not implemented." << std::endl; };

/**
 * Parses return statement. If there is return statement without value we must ensure
 * that we are in void function.
 */
return : RETURN SEMICOLON { $$ = nullptr; }
       ;

/**
 * Parses declaration statement.
 * Declaration might be one or more declarations ( int a; int a,b,c;) and assignment
 * might be initialized (int a = 42;)
 */
declaration : DATA_TYPE at_least_one_id {
	std::vector<Instruction::Ptr> result;

	for (auto [id, init]: $2) {
		// TODO: remove init from alloca instruction. Check init with parser.
		// TODO: make init as method of declaration.
		auto decl = AllocaInstruction::Ptr(new AllocaInstruction({$1, id}, init));
		parser->add(decl);
		result.push_back(decl);
	}

	$$ = result;
}

/**
 * Parses at least one identifier.
 */
at_least_one_id : IDENTIFIER optional_assignment id2init {
	$3.insert($3.begin(), {$1, $2});
	$$ = $3;
};

/**
 * Parses identifier with optional assignment.
 */
id2init : SEMICOLON {
	// Init vector.
	$$ = {};
}
| COMMA IDENTIFIER optional_assignment id2init {
	$4.insert($4.begin(), {$2, $3});
	$$ = $4;
};

optional_assignment : {
	$$ = {};
}
| ASSIGNMENT literal {
	$$ = $2;
};

literal : STRING_LITERAL { $$ = Literal($1); }
	| INT_LITERAL { $$ = Literal($1); }
	| FLOAT_LITERAL { $$ = Literal($1); }
	;

/**
 * Parses list of arguments.
 */
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

class_definition : class_declaration LBRA class_body {
	parser->parseEnd();
};

class_declaration : CLASS IDENTIFIER COLON IDENTIFIER {
	$$ = parser->newClass($2, $4);
	parser->parseStart($$);
};

class_body : function_definition class_body
	   | RBRA;

optional_type : DATA_TYPE { $$ = $1; }
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

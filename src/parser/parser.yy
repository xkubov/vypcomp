%require  "3.0"

%skeleton "lalr1.cc"

// Enable tracing
// https://www.gnu.org/software/bison/manual/html_node/Tracing.html
%define parse.trace
%define parse.error detailed

// Define namespace for our parser.
%define api.namespace {vypcomp}

// Define class of our parser.
%define api.parser.class {Parser}

%code requires {
	// For our plymorfous token.
	#include <iostream>
	#include <variant>
	#include <string>
	#include <utility>

	#include "vypcomp/ir/expression.h"
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
		double,
		PrimitiveDatatype,
		Datatype,
		Arglist,
		Declaration,
		PossibleDatatype,
		Instruction::Ptr,
		OptLiteral,
		std::vector<Instruction::Ptr>,
		std::vector<std::pair<std::string, Expression::ValueType>>,
		BasicBlock::Ptr,
		Function::Ptr,
		Class::Ptr,
		Expression::ValueType,
		std::vector<std::shared_ptr<Expression>>,
		std::optional<std::pair<Datatype, std::string>>
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

%token PROGRAM_START
%token EXPR_PARSE_START
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
%token PUBLIC
%token PRIVATE
%token PROTECTED

%token <terminal<PrimitiveDatatype>()> PRIMITIVE_DATA_TYPE

%token <terminal<std::string>()>IDENTIFIER

%token <terminal<std::string>()>STRING_LITERAL
%token <terminal<unsigned long long>()>INT_LITERAL
%token <terminal<double>()>FLOAT_LITERAL

%token LPAR
%token RPAR
%token RBRA
%token LBRA
%token COMMA
%token ASSIGNMENT
%token SEMICOLON
%token COLON

//
// Operator precedence table in ascending order (later has higher precedence)
//
%left OR
%left AND
%left EQUALS NOTEQUALS
%left '<' LEQ '>' GEQ
%left '+' '-'
%left '*' '/'
%nonassoc EXCLAMATION
%left '.'
%nonassoc LPAR RPAR
%nonassoc NEW
%nonassoc SUBEXPR // ureal token for expr->(expr) rule precedence

%locations

%nterm <nonterminal<Datatype>()> datatype
%nterm <nonterminal<Declaration>()> decl
%nterm <nonterminal<Arglist>()> args
%nterm <nonterminal<Arglist>()> more_args
%nterm <nonterminal<Arglist>()> arg_list
%nterm <nonterminal<Function::Ptr>()> function_definition
%nterm <nonterminal<Function::Ptr>()> function_declaration
%nterm <nonterminal<BasicBlock::Ptr>()> function_body
%nterm <nonterminal<BasicBlock::Ptr>()> basic_block
%nterm <nonterminal<BasicBlock::Ptr>()> else
%nterm <nonterminal<BasicBlock::Ptr>()> if_body
%nterm <nonterminal<BasicBlock::Ptr>()> end_of_block
%nterm <nonterminal<std::vector<Instruction::Ptr>>()> statement
%nterm <nonterminal<Expression::ValueType>()> optional_assignment
%nterm <nonterminal<OptLiteral>()> literal
%nterm <nonterminal<std::vector<Instruction::Ptr>>()> declaration
%nterm <nonterminal<Instruction::Ptr>()> return
%nterm <nonterminal<Class::Ptr>()> class_declaration
%nterm <nonterminal<std::vector<std::pair<std::string, Expression::ValueType>>>()> id2init
%nterm <nonterminal<std::shared_ptr<Expression>>()> expr
%nterm <nonterminal<std::shared_ptr<Expression>>()> binary_operation
%nterm <nonterminal<std::vector<std::shared_ptr<Expression>>>()> func_call_args
%nterm <nonterminal<std::optional<std::pair<Datatype, std::string>>>()> named_data

%%

/**
 * Support multiple parsing approaches, differentiated by different first token.
 */
%start parser_start;
parser_start : PROGRAM_START start
             | EXPR_PARSE_START expr END { 
				/* TODO: condition this on debug parse only if (debug_level()) */ 
				if ($2)
					std::cout << "parsed expression: " << $2->to_string()
						<< ", type: " << $2->type().to_string() << std::endl; 
				else
					std::cout << "expression parse yielded nullptr" << std::endl;
};

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
	parser->parseFunctionEnd();
	$$ = $1;
};

/**
 * @brief Function declaration.

 * Function has a type or is void, identifier and argument list.
 * Argument list is returned as array of declarations. We want
 * to transforme these declaration in instructions. This is done in
 * parseStart as well as other stuff that is needed on the start of the function.
 * -> like pushing new symbol table for local scope.
 */
function_declaration : named_data LPAR arg_list {
	// Creates new funcion.
	auto [t, i] = *$1;
	$$ = parser->newFunction({ t, i, $3 });
	parser->parseStart($$);
};
function_declaration : VOID IDENTIFIER LPAR arg_list {
	// Creates new funcion.
	$$ = parser->newFunction({ {}, $2, $4 });
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
};

if_body : if_action basic_block {
     parser->popSymbolTable();
     $$ = $2;
}

if_action : LBRA {
	parser->pushSymbolTable();
}

else : else_action basic_block {
     parser->popSymbolTable();
     $$ = $2;
}

else_action : ELSE LBRA {
     parser->pushSymbolTable();
}

/**
 * Statemetn definition.
 */
statement : return {
	$$ = {$1};
}
| IDENTIFIER ASSIGNMENT expr SEMICOLON {
	$$ = {parser->assign($1, $3)};
}
| IDENTIFIER LPAR func_call_args SEMICOLON {
	$$ = parser->call_func($1, $3);
}
| declaration {
	$$ = $1;
}
| IF LPAR expr RPAR if_body else {
	if ($3->type() != Datatype(PrimitiveDatatype::Int) && !$3->type().is<Datatype::ClassName>())
		throw SemanticError("Expression in if statement has to be either int or object type.");
	$$ = {BranchInstruction::Ptr(new BranchInstruction($3, $5, $6))};
}
| WHILE LPAR expr RPAR LBRA basic_block {
	if ($3->type() != Datatype(PrimitiveDatatype::Int) && !$3->type().is<Datatype::ClassName>())
		throw SemanticError("Expression in if statement has to be either int or object type.");
	$$ = {LoopInstruction::Ptr(new LoopInstruction($3, $6))};
};

expr 
: LPAR expr RPAR %prec SUBEXPR {
	$$ = std::move($2);
}
| literal {
	$$ = std::make_shared<LiteralExpression>($1.value());
}
| expr LPAR func_call_args {
	auto exp = $1;
	auto args = $3;
	if (!exp->type().is<Datatype::FunctionType>())
	{
		throw SemanticError("Function call attempted on non-function expression.");
	}
	else
	{
		auto function_expr = dynamic_cast<FunctionExpression*>($1.get());
		// TODO: verify argument types and argument count, `print` will have a vararg flag
		// excluding it from the count check (just verify that all types are primitive)
		// probably use the same parser->call_func and take the first element of the vector
		$$ = std::make_shared<FunctionExpression>(function_expr->getFunction(), $3);
	}
}
| IDENTIFIER {
	$$ = parser->identifierExpr($1);
}
| binary_operation {
	$$ = std::move($1);
}
| EXCLAMATION expr {
	$$ = parser->notExpr($2);
}
| LPAR IDENTIFIER RPAR expr
{
	$$ = parser->createCastExpr($2, $4);
}
| THIS {
	$$ = parser->thisExpr();
}
| SUPER {
	$$ = parser->superExpr();
}
;

func_call_args 
: expr COMMA func_call_args { 
	$$.push_back($1);
	$$.insert($$.end(), $3.begin(), $3.end()); 
}
| expr RPAR { $$ = { $1 }; }
| RPAR { $$ = {}; };
// TODO function call here
// | expr LPAR args ...
// TODO cast
// TODO new

binary_operation 
: expr '+' expr {
	$$ = parser->addExpr($1, $3);
}
| expr '-' expr {
	$$ = parser->subExpr($1, $3);
}
| expr '*' expr {
	$$ = parser->mulExpr($1, $3);
}
| expr '/' expr {
	$$ = parser->divExpr($1, $3);
}
| expr GEQ expr {
	$$ = parser->geqExpr($1, $3);
}
| expr '>' expr {
	$$ = parser->gtExpr($1, $3);
}
| expr LEQ expr {
	$$ = parser->leqExpr($1, $3);
}
| expr '<' expr {
	$$ = parser->ltExpr($1, $3);
}
| expr EQUALS expr {
	$$ = parser->eqExpr($1, $3);
}
| expr NOTEQUALS expr {
	$$ = parser->neqExpr($1, $3);
}
| expr AND expr {
	$$ = parser->andExpr($1, $3);
}
| expr OR expr {
	$$ = parser->orExpr($1, $3);
}
| expr '.' IDENTIFIER {
	$$ = parser->dotExpr($1, $3);
};

/**
 * Parses return statement. If there is return statement without value we must ensure
 * that we are in void function.
 */
return : RETURN SEMICOLON { $$ = parser->createReturn(nullptr); }
       | RETURN expr SEMICOLON { $$ = parser->createReturn($2); }
       ;

/**
 * Parses declaration statement.
 * Declaration might be one or more declarations ( int a; int a,b,c;) and assignment
 * might be initialized (int a = 42;)
 */
declaration : named_data optional_assignment id2init {
	std::vector<Instruction::Ptr> result;
	auto [t, n] = *$1;
	$3.insert($3.begin(), {n, $2});

	for (auto [id, init]: $3) {
		// TODO: remove init from alloca instruction. Check init with parser.
		// TODO: make init as method of declaration.
		auto decl = AllocaInstruction::Ptr(new AllocaInstruction({t, id}));
		parser->add(decl);
		result.push_back(decl);
		if (init) {
			result.push_back(parser->assign(id, init));
		}
	}

	$$ = result;
}

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
	$$ = nullptr;
}
| ASSIGNMENT expr {
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

decl : datatype IDENTIFIER { $$ = {$1, $2}; }
     ;

class_definition : class_declaration LBRA class_body {
	if (auto c = $1->constructor()) {
		if (c->args().size())
			throw SemanticError(
				"Cunstructor of class "+$1->name()+
				" cannot have arguments."
			);
		if (c->type())
			throw SemanticError(
				"Cunstructor of class "+$1->name()+
				" is not void."
			);
	}
	parser->parseClassEnd();
};

class_declaration : CLASS IDENTIFIER COLON IDENTIFIER {
	$$ = parser->newClass($2, $4);
	parser->parseStart($$);
};

class_body : function_definition class_body { 
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	curr->add($1, ir::Class::Visibility::Public);
}
| PUBLIC function_definition class_body {
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	curr->add($2, ir::Class::Visibility::Public);
}
| PRIVATE function_definition class_body {
auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	curr->add($2, ir::Class::Visibility::Private);
}
| PROTECTED function_definition class_body {
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	curr->add($2, ir::Class::Visibility::Protected);
}
| declaration class_body {
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	bool first = true;
	for (auto i: $1) {
		if (auto var = std::dynamic_pointer_cast<AllocaInstruction>(i)) {
			if (first) {
				curr->add(var, ir::Class::Visibility::Public);
				first = false;
			} else {
				throw SyntaxError("Invalid syntax for attribute declaration.");
			}
		}
		else {
			//Initialization
			curr->addImplicit(i);
		}
	}
}
| PUBLIC declaration class_body {
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	bool first = true;
	for (auto i: $2) {
		if (auto var = std::dynamic_pointer_cast<AllocaInstruction>(i)) {
			if (first) {
				curr->add(var, ir::Class::Visibility::Public);
				first = false;
			} else {
				throw SyntaxError("Invalid syntax for attribute declaration.");
			}
		}
		else {
			//Initialization
			curr->addImplicit(i);
		}
	}
}
| PRIVATE declaration class_body {
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	bool first = true;
	for (auto i: $2) {
		if (auto var = std::dynamic_pointer_cast<AllocaInstruction>(i)) {
			if (first) {
				curr->add(var, ir::Class::Visibility::Public);
				first = false;
			} else {
				throw SyntaxError("Invalid syntax for attribute declaration.");
			}
		}
		else {
			//Initialization
			curr->addImplicit(i);
		}
	}
}
| PROTECTED declaration class_body {
	auto curr = parser->getCurrentClass();
	if (curr == nullptr)
		throw std::runtime_error("expected class to be parsed! "+std::to_string(__LINE__));
	bool first = true;
	for (auto i: $2) {
		if (auto var = std::dynamic_pointer_cast<AllocaInstruction>(i)) {
			if (first) {
				curr->add(var, ir::Class::Visibility::Public);
				first = false;
			} else {
				throw SyntaxError("Invalid syntax for attribute declaration.");
			}
		}
		else {
			//Initialization
			curr->addImplicit(i);
		}
	}
}
| RBRA;

datatype : PRIMITIVE_DATA_TYPE { $$ = ir::Datatype($1); }
	 | IDENTIFIER { $$ = parser->customDatatype($1); }
	 ;

named_data : datatype IDENTIFIER { $$ = {$1, $2}; };

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

%{
#include <string>
#include <sstream>

#include "vypcomp/parser/scanner.h"

// We are going to use custom yylex.
#undef  YY_DECL
#define YY_DECL int vypcomp::Scanner::yylex(vypcomp::Parser::semantic_type *lval, vypcomp::Parser::location_type *loc)

/* typedef to make the returns for the tokens shorter */
using token = vypcomp::Parser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return (token::END)

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

std::ostringstream buffer;

%}

%option debug
%option nodefault
%option yyclass="vypcomp::Scanner"
%option noyywrap
%option c++

%x BLOCK_COMMENT
%x STRING_PARSE

%%

%{
			yylval = lval;
			if (prepend_first_token)
			{
				prepend_first_token = false;
				return start_token;
			}
%}

[[:space:]]  ;

"/*"                 { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"  { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n    { }
<BLOCK_COMMENT>.     { }
\/\/.*$   ;

\"                      { buffer.str(""); buffer.clear(); BEGIN(STRING_PARSE); }
<STRING_PARSE>\\n       { buffer << "\n"; }
<STRING_PARSE>\\t       { buffer << "\t"; }
<STRING_PARSE>\\\"      { buffer << "\""; }
<STRING_PARSE>\\\\      { buffer << "\\"; }
<STRING_PARSE>\\.       { throw LexicalError("Invalid escape: "+std::string(yytext)); }
<STRING_PARSE>\"        {
	BEGIN(INITIAL);
	*yylval = buffer.str();
	buffer.str(""); buffer.clear();
	return token::STRING_LITERAL;
}
<STRING_PARSE>.         {
	if (*yytext < ' ')
		throw LexicalError(
			"Invalid string character: \'"
			+ std::string(yytext)+"\'"
		);
	buffer << *yytext;
}

class   { return token::CLASS; }
void    { return token::VOID; }
else    { return token::ELSE; }
if      { return token::IF; }
new     { return token::NEW; }
return  { return token::RETURN; }
super   { return token::SUPER; }
this    { return token::THIS; }
while   { return token::WHILE; }
for     { return token::FOR; }
public  { return token::PUBLIC; }
private  { return token::PRIVATE; }
protected  { return token::PROTECTED; }

int     { *yylval = PrimitiveDatatype::Int; return token::PRIMITIVE_DATA_TYPE; }
float   { *yylval = PrimitiveDatatype::Float; return token::PRIMITIVE_DATA_TYPE; }
string  { *yylval = PrimitiveDatatype::String; return token::PRIMITIVE_DATA_TYPE; }

[_a-zA-Z][_a-zA-Z0-9]*  { *yylval = yytext; return token::IDENTIFIER; }

[0-9]*\.([0-9]+f?|f) { *yylval = std::stod(yytext, nullptr); return token::FLOAT_LITERAL; }
[0-9]+([a-zA-Z])  {
	throw LexicalError("invalid number: "+std::string(yytext));
}

[0-9]+  {
	// string validation should not be necessary.
	*yylval = strtoull(yytext, nullptr, 10);
	return token::INT_LITERAL;
}

[(]  {return token::LPAR;}
[)]  {return token::RPAR;}
[,]  {return token::COMMA;}
[{]  {return token::LBRA;}
[}]  {return token::RBRA;}
[:]  {return token::COLON;}
[;]  {return token::SEMICOLON;}
[=]  {return token::ASSIGNMENT;}
"+"  { return '+'; }
"-"  { return '-'; }
"*"  { return '*'; }
"/"  { return '/'; }
"."  { return '.'; }
"=="  { return token::EQUALS; }
"!="  { return token::NOTEQUALS; }
">"  { return '>'; }
">="  { return token::GEQ; }
"<"  { return '<'; }
"<="  { return token::LEQ; }
"||"  { return token::OR; }
"&&"  { return token::AND; }

.  {
	// TODO: parse word and print it.
	std::ostringstream err;
	err << "invalid lexeme: " << yytext << " (" << *loc << ")";
	throw LexicalError(err.str());
}

%%

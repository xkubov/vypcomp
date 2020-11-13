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
	return token::LITERAL;
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

int     { *yylval = Datatype::Int; return token::DATA_TYPE; }
float   { *yylval = Datatype::Float; return token::DATA_TYPE; }
string  { *yylval = Datatype::String; return token::DATA_TYPE; }

[_a-zA-Z][_a-zA-Z0-9]*  { *yylval = yytext; return token::IDENTIFIER; }

[0-9]+([a-zA-Z])  {
	throw LexicalError("invalid number: "+std::string(yytext));
}

[0-9]+  {
	// string validation should not be necessary.
	*yylval = strtoull(yytext, nullptr, 10);
	return token::LITERAL;
}

[(]  {return token::LPAR;}
[)]  {return token::RPAR;}
[,]  {return token::COMMA;}
[{]  {return token::LBRA;}
[}]  {return token::RBRA;}
[;]  {return token::SEMICOLON;}
[=]  {return token::ASSIGNMENT;}

.  {
	// TODO: parse word and print it.
	std::ostringstream err;
	err << "invalid lexeme: " << yytext << " (" << *loc << ")";
	throw LexicalError(err.str());
}

%%

namespace vypcomp {

vypcomp::LexicalError::LexicalError(const std::string& msg):
	msg(msg)
{
};

const char * vypcomp::LexicalError::what() const throw()
{
	return msg.c_str();
};

}

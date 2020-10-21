%{
#include <string>

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

%}

%option debug
%option nodefault
%option yyclass="vypcomp::Scanner"
%option noyywrap
%option c++

%%

%{
            yylval = lval;
%}

[[:space:]]	;
class	{ return token::CLASS; }
else	{ return token::ELSE; }
if	{ return token::IF; }
int	{ return token::INT; }
new	{ return token::NEW; }
return	{ return token::RETURN; }
string	{ return token::STRING; }
super	{ return token::SUPER; }
this	{ return token::THIS; }
void	{ return token::VOID; }
while	{ return token::WHILE; }
.	{ throw LexicalError("unknown type name '"+std::string(yytext)+"'"); }

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

%{
#include <string>

#include "flex_scanner.hpp"

#undef  YY_DECL
#define YY_DECL int vypcomp::Scanner::yylex(vypcomp::Parser::semantic_type * const lval, vypcomp::Parser::location_type *loc)

/* typedef to make the returns for the tokens shorter */
using token = vypcomp::Parser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return( token::END )

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
%option nodefault
%option yyclass="vypcomp::Scanner"
%option noyywrap
%option c++

%%

%%

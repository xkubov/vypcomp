#include "vypcomp/errors/errors.h"

using namespace vypcomp;

vypcomp::LexicalError::LexicalError(const std::string& msg):
	msg(msg)
{
};

const char * vypcomp::LexicalError::what() const throw()
{
	return msg.c_str();
};

SyntaxError::SyntaxError(const std::string& msg):
	msg(msg)
{
}

const char* SyntaxError::what() const throw()
{
	return msg.c_str();
}


SemanticError::SemanticError(const std::string& msg):
	msg(msg)
{
}

const char* SemanticError::what() const throw()
{
	return msg.c_str();
}

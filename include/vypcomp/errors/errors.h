#pragma once

#include <exception>
#include <string>

namespace vypcomp {

class LexicalError: public std::exception {
public:
	LexicalError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

class SyntaxError : public std::exception {
public:
	SyntaxError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

class SemanticError: public std::exception {
public:
	SemanticError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

class IncompabilityError: public std::exception {
public:
	IncompabilityError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

}

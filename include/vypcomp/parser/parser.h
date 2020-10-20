#pragma once

#include <memory>
#include <string>

#include "vypcomp/parser/scanner.h"
#include "bison_parser.tab.hpp"

namespace vypcomp {

/**
 * Provides wrapper around Bison Parser that implements interface
 * for pasing.
 */
class LangParser {
public:
	LangParser() = default;

	virtual ~LangParser();

public:
	/**
	 * @brief Parse file provided by path as argument.
	 */
	void parse(const std::string &filename);
	void parse(std::istream &file);

	void generateOutput(const std::string &output) const;
	void generateOutput(std::ostream &output) const;

private:
	std::unique_ptr<vypcomp::Parser> _parser;
	std::unique_ptr<vypcomp::Scanner> _scanner;
};

class SyntaxError: public std::exception {
public:
	SyntaxError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

}

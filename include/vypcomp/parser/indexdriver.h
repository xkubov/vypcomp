#pragma once

#include "parser.h"

namespace vypcomp {

/**
 * Provides first index run pareser driver.
 */
class IndexParserDriver : public ParserDriver {
public:
	IndexParserDriver();
	IndexParserDriver(const SymbolTable &global);

public:
	virtual Class::Ptr newClass(const std::string& name, const std::string& base) const override;
	virtual Function::Ptr newFunction(const ir::Function::Signature& sig) const override;

	virtual void parseStart(ir::Function::Ptr fun) override;
	virtual void parseStart(ir::Class::Ptr fun) override;

private:
	std::vector<vypcomp::SymbolTable> _tables;
};

} // namespace vypcomp

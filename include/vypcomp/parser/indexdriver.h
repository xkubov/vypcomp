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
	virtual Datatype customDatatype(const std::string& dt) const override;
	virtual Instruction::Ptr assign(const std::string& ptr, const ir::Expression::ValueType& val) const override;
	virtual std::shared_ptr<CastExpression> createCastExpr(std::string class_name, Expression::ValueType expr) const override;

private:
	std::vector<vypcomp::SymbolTable> _tables;
};

} // namespace vypcomp

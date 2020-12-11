#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "vypcomp/ir/ir.h"

namespace vypcomp {

namespace ir {

/**
 * Represents instruction block that can be named.
 */
class BasicBlock {
public:
	using Ptr = std::shared_ptr<BasicBlock>;

	/// Name if not specified set randomly.
	BasicBlock(
		const std::string& name = {},
		const std::string& suffix = {}
	);

	static BasicBlock::Ptr create();

	void setNext(BasicBlock::Ptr instr);
	BasicBlock::Ptr next() const;

	void addFirst(Instruction::Ptr first);
	Instruction::Ptr first() const;
	Instruction::Ptr last() const;

	std::string name() const;

private:
	BasicBlock::Ptr _next;
	Instruction::Ptr _first = nullptr;
	std::string _name = "";
};

class DummyInstruction : public Instruction {
public:
	using Ptr = std::shared_ptr<DummyInstruction>;
	virtual std::string str() const override;
};

class AllocaInstruction : public Instruction {
public:
	using Ptr = std::shared_ptr<AllocaInstruction>;

	/**
	 * Initial value is stored in form of string.
	 */
	AllocaInstruction(const Declaration& decl);

	void addPrefix(const std::string& prefix);
	virtual std::string str() const override;

	PrimitiveDatatype type() const;
	std::string name() const;

private:
	std::string _varName;
	std::string _prefix;
	PrimitiveDatatype _type;
};

class Assignment: public Instruction {
public:
	using Ptr = std::shared_ptr<Assignment>;

	/**
	 * Initial value is stored in form of string.
	 */
	Assignment(
		AllocaInstruction::Ptr ptr,
		Expression::ValueType expr
	);
	virtual std::string str() const override;

private:
	AllocaInstruction::Ptr _ptr;
	Expression::ValueType _expr;
};

class Function: public Instruction {
public:
	using Ptr = std::shared_ptr<Function>;
	using Signature = std::tuple<PossibleDatatype, std::string, Arglist>;

	Function(const Signature& sig);

	void addPrefix(const std::string& prefix);
	void setFirst(const BasicBlock::Ptr body);

	BasicBlock::Ptr first() const;
	BasicBlock::Ptr last() const;
	virtual std::string str() const override;

	bool isVoid() const;

	std::string name() const;
	PossibleDatatype type() const;
	const std::vector<AllocaInstruction::Ptr> args() const;
	std::vector<PrimitiveDatatype> argTypes() const;

private:
	PossibleDatatype _type;
	std::string _name;
	std::string _prefix;
	std::vector<AllocaInstruction::Ptr> _args;

	BasicBlock::Ptr _first = nullptr;
};

class BranchInstruction: public Instruction {
public:
	using Ptr = std::shared_ptr<BranchInstruction>;

	BranchInstruction(
		Expression::ValueType expr,
		BasicBlock::Ptr ifBlock,
		BasicBlock::Ptr elseBlock
	);
	virtual std::string str() const override;

private:
	Expression::ValueType _expr = nullptr;
	BasicBlock::Ptr _if = nullptr;
	BasicBlock::Ptr _else = nullptr;
};

class LoopInstruction: public Instruction {
public:
	using Ptr = std::shared_ptr<LoopInstruction>;

	LoopInstruction(
		Expression::ValueType expr,
		BasicBlock::Ptr loop
	);
	virtual std::string str() const override;

private:
	Expression::ValueType _expr = nullptr;
	BasicBlock::Ptr _body = nullptr;
};

class Class: public Instruction {
public:
	using Ptr = std::shared_ptr<Class>;
	Class(const std::string& name, Class::Ptr base);

	void setBase(Class::Ptr base);
	Class::Ptr getBase() const;
	void add(Function::Ptr methods, bool isPublic = true);
	void add(AllocaInstruction::Ptr attr, bool isPublic = true);

	Function::Ptr getPublicMethod(
		const std::string& name,
		const std::vector<PrimitiveDatatype>& argtypes) const;
	Function::Ptr getPublicMethodByName(const std::string& name) const;
	AllocaInstruction::Ptr getPublicAttribute(const std::string& name) const;

	const std::vector<Function::Ptr> publicMethods() const;
	const std::vector<Function::Ptr> privateMethods() const;

	const std::vector<AllocaInstruction::Ptr> publicAttributes() const;
	const std::vector<AllocaInstruction::Ptr> privateAttributes() const;

	std::string name() const;
	virtual std::string str() const override;

private:
	std::string _name;
	Class::Ptr _parent;
	std::vector<Function::Ptr> _publicMethods;
	std::vector<Function::Ptr> _privateMethods;
	std::vector<AllocaInstruction::Ptr> _publicAttrs;
	std::vector<AllocaInstruction::Ptr> _privateAttrs;
};

}
}

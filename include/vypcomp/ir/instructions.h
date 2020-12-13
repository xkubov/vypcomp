#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "vypcomp/ir/ir.h"

namespace vypcomp {

class Generator;

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
	std::string str(const std::string& prefix) const;

	std::string name() const;

private:
	BasicBlock::Ptr _next;
	Instruction::Ptr _first = nullptr;
	std::string _name = "";
};

class DummyInstruction : public Instruction {
public:
	using Ptr = std::shared_ptr<DummyInstruction>;
	virtual std::string str(const std::string& prefix) const override;
};

class AllocaInstruction : public Instruction {
public:
	using Ptr = std::shared_ptr<AllocaInstruction>;

	/**
	 * Initial value is stored in form of string.
	 */
	AllocaInstruction(const Declaration& decl);

	void addPrefix(const std::string& prefix);
	virtual std::string str(const std::string& prefix) const override;

	Datatype type() const;
	std::string name() const;

private:
	std::string _varName;
	std::string _prefix;
	Datatype _type;
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
	virtual std::string str(const std::string& prefix) const override;
	AllocaInstruction::Ptr getAlloca() const;
	Expression::ValueType getExpr() const;
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
	virtual std::string str(const std::string& prefix) const override;

	bool isVoid() const;

	std::string name() const;
	PossibleDatatype type() const;
	const std::vector<AllocaInstruction::Ptr> args() const;
	std::vector<Datatype> argTypes() const;

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
	virtual std::string str(const std::string& prefix) const override;
	BasicBlock::Ptr getIf() const;
	BasicBlock::Ptr getElse() const;
private:
	Expression::ValueType _expr = nullptr;
	BasicBlock::Ptr _if = nullptr;
	BasicBlock::Ptr _else = nullptr;
};

class Return: public Instruction {
public:
	using Ptr = std::shared_ptr<Return>;

	Return(Expression::ValueType expr = nullptr);
	bool isVoid() const;

	virtual std::string str(const std::string& prefix) const override;

private:
	Expression::ValueType _expr;
};

class LoopInstruction: public Instruction {
public:
	using Ptr = std::shared_ptr<LoopInstruction>;

	LoopInstruction(
		Expression::ValueType expr,
		BasicBlock::Ptr loop
	);
	virtual std::string str(const std::string& prefix) const override;
	BasicBlock::Ptr getBody() const;
private:
	Expression::ValueType _expr = nullptr;
	BasicBlock::Ptr _body = nullptr;
};

class Class: public Instruction {
public:
	enum class Visibility {
		Public,
		Private,
		Protected
	};
public:
	using Ptr = std::shared_ptr<Class>;
	Class(const std::string& name, Class::Ptr base);

	void setBase(Class::Ptr base);
	Class::Ptr getBase() const;
	void add(Function::Ptr methods, const Visibility& v = Visibility::Public);
	void add(AllocaInstruction::Ptr attr, const Visibility& v = Visibility::Public);

	// Visibility:
	//  - Public: method that is available to public
	//  - Protected/Private: method that is available to the class too.
	Function::Ptr getMethod(
		const std::string& name,
		const std::vector<Datatype>& argtypes,
		const Visibility& v = Visibility::Public
	) const;
	Function::Ptr getMethod(
		const std::string& name,
		const Visibility& v = Visibility::Public
	) const;

	//
	// Visibility:
	//  - Public: attribute that is available to public
	//  - Protected/Private: attribute that is available to the class too.
	AllocaInstruction::Ptr getAttribute(const std::string& name, const Visibility& v = Visibility::Public) const;

	const std::vector<Function::Ptr> publicMethods() const;
	const std::vector<Function::Ptr> privateMethods() const;
	const std::vector<Function::Ptr> protectedMethods() const;

	const std::vector<AllocaInstruction::Ptr> publicAttributes() const;
	const std::vector<AllocaInstruction::Ptr> privateAttributes() const;
	const std::vector<AllocaInstruction::Ptr> protectedAttributes() const;

	std::string name() const;
	virtual std::string str(const std::string& prefix) const override;

private:
	std::string _name;
	Class::Ptr _parent;
	std::vector<Function::Ptr> _publicMethods;
	std::vector<Function::Ptr> _privateMethods;
	std::vector<Function::Ptr> _protectedMethods;
	std::vector<AllocaInstruction::Ptr> _publicAttrs;
	std::vector<AllocaInstruction::Ptr> _privateAttrs;
	std::vector<AllocaInstruction::Ptr> _protectedAttrs;
};

}
}

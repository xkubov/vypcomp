#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace vypcomp {

namespace ir {

/**
 * Provides datatype options.
 */
enum class PrimitiveDatatype : int {
	String,
	Float,
	Int
};

using Declaration = std::pair<PrimitiveDatatype, std::string>;
using Arglist = std::vector<Declaration>;
using PossibleDatatype = std::optional<PrimitiveDatatype>;


struct Literal {
public:
	using Impl = std::variant<std::string, unsigned long long, float>;
	Literal(const Impl& val);

	template<PrimitiveDatatype = PrimitiveDatatype::String>
	std::string get() const;

	template<PrimitiveDatatype = PrimitiveDatatype::Int>
	int get() const;

	template<PrimitiveDatatype = PrimitiveDatatype::Float>
	float get() const;

private:
	Impl _val;
};

using OptLiteral = std::optional<Literal>;

/**
 * Represents abstraction over instruction.
 *
 * Instruction is basically every statement:
 *  - function call,
 *  - variable declaration
 *  - if statement
 *  - loops
 *  - assignmnent
 */
class Instruction {
public:
	using Ptr = std::shared_ptr<Instruction>;

	Instruction();

	void setNext(Instruction::Ptr next);
	Instruction::Ptr next() const;

protected:
	Instruction::Ptr _next = nullptr;
};

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

class AllocaInstruction: public Instruction {
public:
	using Ptr = std::shared_ptr<AllocaInstruction>;

	/**
	 * Initial value is stored in form of string.
	 */
	AllocaInstruction(
		const Declaration& decl,
		const OptLiteral& init = {}
	);

	void addPrefix(const std::string& prefix);

	PrimitiveDatatype type() const;
	std::string name() const;
	OptLiteral init() const;

private:
	std::string _varName;
	std::string _prefix;
	PrimitiveDatatype _type;
	OptLiteral _init;
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
		BasicBlock::Ptr ifBlock,
		BasicBlock::Ptr elseBlock
	);

private:
	BasicBlock::Ptr _if = nullptr;
	BasicBlock::Ptr _else = nullptr;
};

class LoopInstruction: public Instruction {
public:
	using Ptr = std::shared_ptr<LoopInstruction>;

	LoopInstruction(
		BasicBlock::Ptr loop
	);

private:
	BasicBlock::Ptr _body = nullptr;
};

class Class: public Instruction {
public:
	using Ptr = std::shared_ptr<Class>;
	Class(const std::string& name, Class::Ptr parent);

	void add(Function::Ptr methods, bool isPublic = true);
	void add(AllocaInstruction::Ptr attr, bool isPublic = true);

	Function::Ptr getPublicMethod(
		const std::string& name,
		const std::vector<PrimitiveDatatype>& argtypes) const;
	AllocaInstruction::Ptr getPublicAttribute(const std::string& name) const;

	const std::vector<Function::Ptr> publicMethods() const;
	const std::vector<Function::Ptr> privateMethods() const;

	const std::vector<AllocaInstruction::Ptr> publicAttributes() const;
	const std::vector<AllocaInstruction::Ptr> privateAttributes() const;

	std::string name() const;

private:
	const std::string _name;
	Class::Ptr _parent;
	std::vector<Function::Ptr> _publicMethods;
	std::vector<Function::Ptr> _privateMethods;
	std::vector<AllocaInstruction::Ptr> _publicAttrs;
	std::vector<AllocaInstruction::Ptr> _privateAttrs;
};

}
}

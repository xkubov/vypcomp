#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace vypcomp {

namespace ir {

/**
 * Provides datatype options.
 */
enum class Datatype : int {
	String,
	Float,
	Int
};

using Declaration = std::pair<Datatype, std::string>;
using Arglist = std::vector<Declaration>;
using PossibleDatatype = std::optional<Datatype>;

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
		const std::string& init = {}
	);

	Datatype type() const;
	std::string name() const;
	std::string init() const;

private:
	std::string _varName;
	Datatype _type;
	std::string _init;
};

class Function: public Instruction {
public:
	using Ptr = std::shared_ptr<Function>;
	using Signature = std::tuple<PossibleDatatype, std::string, Arglist>;

	Function(const Signature& sig);

	void setFirst(const BasicBlock::Ptr body);
	BasicBlock::Ptr first() const;
	BasicBlock::Ptr last() const;

	bool isVoid() const;

	std::string name() const;
	PossibleDatatype type() const;
	const std::vector<AllocaInstruction::Ptr> args() const;

private:
	PossibleDatatype _type;
	std::string _name;
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

}
}

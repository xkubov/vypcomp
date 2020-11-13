#include "vypcomp/ir/instructions.h"

using namespace vypcomp;
using namespace vypcomp::ir;

// ------------------------------
// Instruction
// ------------------------------

Instruction::Instruction()
{
}

void Instruction::setNext(Instruction::Ptr next)
{
	_next = next;
}

Instruction::Ptr Instruction::next() const
{
	return _next;
}

// ------------------------------
// InstructionBlock
// ------------------------------

InstructionBlock::InstructionBlock(const std::string& name, const std::string& suf)
{
	static uint64_t ID = 0;

	std::string suffix(suf);
	if (suffix.empty()) {
		suffix = "_"+std::to_string(ID);
		ID++;
	}

	name = name+suffix;
}

void InstructionBlock::setNext(InstructionBlock::Ptr instr)
{
	_next = next;
}

InstructionBlock::Ptr InstructionBlock::next() const
{
	return _next;
}

Instruction::Ptr InstructionBlock::first() const
{
	return _first;
}

Instruction::Ptr InstructionBlock::last() const
{
	Instruction::Ptr prev = nullptr;
	for (auto it = _first; it != nullptr; it = it->next()) {
		prev = it;
	}

	return prev;
}

// ------------------------------
// Function
// ------------------------------

Function::Function(const Function::Signature& sig)
{
	std::tie(_type, _name, _args) = sig;
}

void Function::setFirst(const InstructionBlock::Ptr body)
{
	_first = body;
}

InstructionBlock::Ptr Function::first() const
{
	return _first;
}

InstructionBlock::Ptr Function::last() const
{
	auto prev = _first;
	for (auto it = _first; it != nullptr; it+=it->next()) {
		prev = it;
	}

	return prev;
}

bool Function::isVoid() const
{
	return _type;
}

// ------------------------------
// BranchInstruction
// ------------------------------

BranchInstruction::BranchInstruction(
		InstructionBlock::Ptr ifBlock,
		InstructionBlock::Ptr elseBlock):
	_if(ifBlock),
	_else(elseBlock)
{
}

// ------------------------------
// LoopInstruction
// ------------------------------

LoopInstruction::LoopInstruction(InstructionBlock::Ptr body):
	_body(body)
{
}

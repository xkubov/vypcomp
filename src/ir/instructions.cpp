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
// BasicBlock
// ------------------------------

BasicBlock::BasicBlock(const std::string& name, const std::string& suf)
{
	static uint64_t ID = 0;

	std::string suffix(suf);
	if (suffix.empty()) {
		suffix = "_"+std::to_string(ID);
		ID++;
	}

	_name = name+suffix;
}

BasicBlock::Ptr BasicBlock::create()
{
	return BasicBlock::Ptr(new BasicBlock("label", ""));
}

void BasicBlock::setNext(BasicBlock::Ptr instr)
{
	_next = instr;
}

void BasicBlock::addFirst(Instruction::Ptr first)
{
	if (_first) {
		first->setNext(first);
	}

	_first = first;
}

BasicBlock::Ptr BasicBlock::next() const
{
	return _next;
}

Instruction::Ptr BasicBlock::first() const
{
	return _first;
}

Instruction::Ptr BasicBlock::last() const
{
	Instruction::Ptr prev = nullptr;
	for (auto it = _first; it != nullptr; it = it->next()) {
		prev = it;
	}

	return prev;
}

std::string BasicBlock::name() const
{
	return _name;
}

// ------------------------------
// Function
// ------------------------------

Function::Function(const Function::Signature& sig)
{
	std::tie(_type, _name, _args) = sig;
}

void Function::setFirst(const BasicBlock::Ptr body)
{
	_first = body;
}

BasicBlock::Ptr Function::first() const
{
	return _first;
}

BasicBlock::Ptr Function::last() const
{
	auto prev = _first;
	for (auto it = _first; it != nullptr; it = it->next()) {
		prev = it;
	}

	return prev;
}

bool Function::isVoid() const
{
	return _type.has_value();
}

// ------------------------------
// BranchInstruction
// ------------------------------

BranchInstruction::BranchInstruction(
		BasicBlock::Ptr ifBlock,
		BasicBlock::Ptr elseBlock):
	_if(ifBlock),
	_else(elseBlock)
{
}

// ------------------------------
// LoopInstruction
// ------------------------------

LoopInstruction::LoopInstruction(BasicBlock::Ptr body):
	_body(body)
{
}

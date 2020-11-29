#include "vypcomp/ir/instructions.h"

using namespace vypcomp;
using namespace vypcomp::ir;

Literal::Literal(const Literal::Impl &val):
	_val(val)
{
}

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
		first->setNext(_first);
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
	Arglist args;
	std::tie(_type, _name, args) = sig;
	for (auto decl: args) {
		_args.push_back(
			AllocaInstruction::Ptr(new AllocaInstruction(decl))
		);
	}
}

void Function::addPrefix(const std::string& prefix)
{
	_prefix += prefix;
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

std::string Function::name() const
{
	return _name;
}

PossibleDatatype Function::type() const
{
	return _type;
}

const std::vector<AllocaInstruction::Ptr> Function::args() const
{
	return _args;
}

std::vector<PrimitiveDatatype> Function::argTypes() const
{
	std::vector<PrimitiveDatatype> types;
	for (auto arg: _args) {
		types.push_back(arg->type());
	}

	return types;
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

// ------------------------------
// AllocaInstruction
// ------------------------------

AllocaInstruction::AllocaInstruction(const Declaration& decl, const OptLiteral& init):
	_init(init)
{
	std::tie(_type, _varName) = decl;
}

void AllocaInstruction::addPrefix(const std::string& prefix)
{
	_prefix += prefix;
}

PrimitiveDatatype AllocaInstruction::type() const
{
	return _type;
}

std::string AllocaInstruction::name() const
{
	return _varName;
}

OptLiteral AllocaInstruction::init() const
{
	return _init;
}

// ------------------------------
// Class
// ------------------------------

Class::Class(const std::string& name, Class::Ptr parent):
	_name(name),
	_parent(parent)
{
}

void Class::add(Function::Ptr method, bool isPublic)
{
	method->addPrefix(_name);

	if (isPublic)
		_publicMethods.push_back(method);
	else
		_privateMethods.push_back(method);
}

void Class::add(AllocaInstruction::Ptr attr, bool isPublic)
{
	attr->addPrefix(_name);

	if (isPublic)
		_publicAttrs.push_back(attr);
	else
		_privateAttrs.push_back(attr);
}

Function::Ptr Class::getPublicMethod(const std::string& name, const std::vector<PrimitiveDatatype>& argtypes) const
{
	auto it = std::find_if(_publicMethods.begin(), _publicMethods.end(), [name, argtypes](const auto& method) {
		return method->name() == name && method->argTypes() == argtypes;
	});

	if (it != _publicMethods.end())
		return *it;

	if (_parent)
		return _parent->getPublicMethod(name, argtypes);

	return nullptr;
}

AllocaInstruction::Ptr Class::getPublicAttribute(const std::string& name) const
{
	auto it = std::find_if(_publicAttrs.begin(), _publicAttrs.end(), [name](const auto& attr) {
		return attr->name() == name;
	});

	if (it != _publicAttrs.end())
		return *it;

	if (_parent)
		return _parent->getPublicAttribute(name);

	return nullptr;
}

std::string Class::name() const
{
	return _name;
}

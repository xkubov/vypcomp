#pragma once
#include <string>
#include <sstream>

#include <vypcomp/ir/instructions.h>

namespace vypcomp
{
    class Generator
    {
    public:
        using OutputStream = std::ostream;
        using RegisterName = std::string;
    public:
        Generator() = default;
        Generator(bool verbose) : verbose(verbose) {}

        void generate(vypcomp::ir::BasicBlock::Ptr input, OutputStream& out);
    private:
        void generate_instruction(vypcomp::ir::Instruction::Ptr input, OutputStream& out);
        vypcomp::Generator::RegisterName generate_expression(ir::Expression::ValueType input, OutputStream& out);
        bool is_alloca(vypcomp::ir::Instruction::Ptr instr);
        std::vector<ir::AllocaInstruction::Ptr> get_alloca_instructions(vypcomp::ir::Instruction::Ptr block);

    private:
        bool verbose = false;
    };
}

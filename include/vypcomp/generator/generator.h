#pragma once
#include <string>
#include <sstream>
#include <unordered_map>

#include <vypcomp/ir/instructions.h>
#include <vypcomp/parser/symbol_table.h>
#include <vypcomp/ir/expression.h>

namespace vypcomp
{
    class Generator
    {
    public:
        using OutputStream = std::ostream;
        using RegisterName = std::string;
        using AllocaRawPtr = vypcomp::ir::AllocaInstruction*;
        using OffsetMap = std::unordered_map<AllocaRawPtr, std::int64_t>;
    public:
        Generator(std::string out_filename, bool verbose);
        Generator(std::unique_ptr<std::ostream> out, bool verbose);

        void generate(const SymbolTable& symbol_table);

        const OutputStream& get_output() const;
    private:
        void generate(vypcomp::ir::Function::Ptr input);
        void generate_instruction(vypcomp::ir::Instruction::Ptr input, OffsetMap& variable_offsets);
        vypcomp::Generator::RegisterName generate_expression(ir::Expression::ValueType input);
        bool is_alloca(vypcomp::ir::Instruction::Ptr instr);
        std::vector<ir::AllocaInstruction::Ptr> get_alloca_instructions(vypcomp::ir::Instruction::Ptr block);

    private:
        std::unique_ptr<std::ostream> out;
        bool verbose = false;
    };
}

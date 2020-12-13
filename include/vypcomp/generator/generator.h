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
        void generate_expression(ir::Expression::ValueType input, RegisterName destination, OffsetMap& variable_offsets);
        void generate_return();
        std::vector<ir::AllocaInstruction::Ptr> get_alloca_instructions(vypcomp::ir::Instruction::Ptr block);

        bool is_alloca(vypcomp::ir::Instruction::Ptr instr) const;
        bool is_return(vypcomp::ir::Instruction::Ptr instr) const;
    private:
        std::unique_ptr<std::ostream> out;
        bool verbose = false;
        // These variables are needed when the generator jumps into an instruction stream from generate(function)
        // so that it can properly generate return statements
        std::size_t arg_count = 0;
        std::size_t variable_count = 0;
    };
}

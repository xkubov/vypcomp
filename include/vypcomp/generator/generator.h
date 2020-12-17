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
        using DestinationName = std::string;
        using AllocaVector = std::vector<ir::AllocaInstruction::Ptr>;
        using AllocaRawPtr = vypcomp::ir::AllocaInstruction*;
        using ExprRawPtr = vypcomp::ir::Expression*;
        using OffsetMap = std::unordered_map<AllocaRawPtr, std::int64_t>;
        using TempVarMap = std::unordered_map<ExprRawPtr, AllocaRawPtr>;
    public:
        Generator(std::string out_filename, bool verbose);
        Generator(std::unique_ptr<std::ostream> out, bool verbose);

        void generate(const SymbolTable& symbol_table);

        const OutputStream& get_output() const;
    private:
        void generate(vypcomp::ir::Function::Ptr input, OutputStream& out);
        void generate_block(vypcomp::ir::BasicBlock::Ptr in_block, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_instruction(vypcomp::ir::Instruction::Ptr input, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_expression(ir::Expression::ValueType input, DestinationName destination, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_binaryop(ir::BinaryOpExpression::Ptr input, DestinationName destination, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_return(OutputStream& out);
        void generate_builtin_functions(OutputStream& out);

        // aggregates all alloca instructions from the whole function, these alloca locations are then assigned stack positions in variable_offsets mapping
        AllocaVector get_alloca_instructions(vypcomp::ir::Instruction::Ptr block, TempVarMap& exp_temporary_mapping);
        std::vector<ir::AllocaInstruction::Ptr> get_temporary_allocas(vypcomp::ir::Expression::ValueType expr, TempVarMap& exp_temporary_mapping);
        std::vector<ir::AllocaInstruction::Ptr> get_required_temporaries(ir::Expression::ValueType expr, TempVarMap& exp_temporary_mapping);

        bool is_alloca(vypcomp::ir::Instruction::Ptr instr) const;
        bool is_return(vypcomp::ir::Instruction::Ptr instr) const;
        bool is_builtin_func(std::string func_name) const;

        std::optional<std::size_t> find_offset(AllocaRawPtr alloca_ptr, OffsetMap& variable_offsets) const;
        std::optional<AllocaRawPtr> find_expr_destination(ExprRawPtr expr, TempVarMap& temporary_variables_mapping) const;
        DestinationName get_expr_destination(ExprRawPtr expr, TempVarMap& temporary_variables_mapping, OffsetMap& variable_offsets) const;
    private:
        std::unique_ptr<std::ostream> _main_out;
        bool verbose = false;
        // These variables are needed when the generator jumps into an instruction stream from generate(function)
        // so that it can properly generate return statements
        std::size_t arg_count = 0;
        std::size_t variable_count = 0;
    };
}

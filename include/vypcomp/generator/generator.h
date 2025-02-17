/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

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
        using ClassName = std::string;
        using MethodName = std::string;
        using LabelName = std::string;
        using VtableMapping = std::unordered_map<MethodName, LabelName>;
        using MethodVector = std::vector<MethodName>;
        using VtableIndexLookup = std::unordered_map<MethodName, std::size_t>;
        using VtableIndexLookupPtr = std::shared_ptr<VtableIndexLookup>;
        using ClassVtableLookup = std::unordered_map<ClassName, VtableIndexLookupPtr>;
        using VtableAddressMapping = std::unordered_map<ClassName, std::uint64_t>;
    public:
        Generator(std::string out_filename, bool verbose);
        Generator(std::unique_ptr<std::ostream> out, bool verbose);

        void generate(const SymbolTable& symbol_table);

        const OutputStream& get_output() const;
    private:
        void generate_function(vypcomp::ir::Function::Ptr input, std::string label_name, OutputStream& out);
        void generate_function_body(vypcomp::ir::Function::Ptr input, OutputStream& out, const AllocaVector& args, const AllocaVector& local_variables, TempVarMap& temporary_variables_mapping);
        void generate_constructor_body(vypcomp::ir::Function::Ptr input, std::string label_name, OutputStream& out);
        void generate_block(vypcomp::ir::BasicBlock::Ptr in_block, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_instruction(vypcomp::ir::Instruction::Ptr input, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_expression(ir::Expression::ValueType input, DestinationName destination, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_binaryop(ir::BinaryOpExpression::Ptr input, DestinationName destination, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out);
        void generate_return(OutputStream& out);
        void generate_builtin_functions(OutputStream& out);
        void generate_vtables(const SymbolTable& symbol_table, OutputStream& out);
        std::string generate_method_label(const ir::Function::Ptr& method);
        void generate_class(vypcomp::ir::Class::Ptr input, OutputStream& out);
        void generate_constructor(vypcomp::ir::Class::Ptr input, OutputStream& out);
        void generate_constructor_chain_invocation(vypcomp::ir::Class::Ptr input, OutputStream& out);
        std::size_t get_object_size(vypcomp::ir::Class::Ptr input);
        std::size_t get_object_attribute_offset(vypcomp::ir::Class::Ptr class_ptr, const std::string& attribute_name);

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
        // assigns vtable id for each class
        VtableAddressMapping class_vtable_addr_mapping;
        // maps class name to a table that maps methods to method ids
        ClassVtableLookup class_method_vtable_mapping;
        // These variables are needed when the generator jumps into an instruction stream from generate(function)
        // so that it can properly generate return statements
        std::size_t arg_count = 0;
        std::size_t variable_count = 0;
    };
}

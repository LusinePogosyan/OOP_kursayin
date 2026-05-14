#include "BackendPipeline.h"

#include "../../Compiler/CodeGenerator.h"

namespace backend {

void AstOptimizer::optimize(std::vector<std::unique_ptr<ASTNode>>&) const {
    // Placeholder for AST-level optimizations (constant folding, dead branch removal, etc.).
}

void IrOptimizer::optimize(LogicalIR&) const {
    // Placeholder for IR-level peephole and control-flow optimizations.
}

LogicalIR BackendPipeline::lowerToLogicalIr(const std::vector<std::unique_ptr<ASTNode>>& ast, SymbolTable& symbols) const {
    CodeGenerator generator(symbols);
    for (const auto& node : ast) {
        generator.generate(node.get());
    }

    LogicalIR ir;
    ir.instructions = generator.getResult();
    ir.dataWords = generator.getDataSection();
    ir.dataBaseAddress = generator.getDataBaseAddress();
    return ir;
}

}  // namespace backend

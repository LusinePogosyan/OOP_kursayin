#ifndef BACKEND_PIPELINE_H
#define BACKEND_PIPELINE_H

#include <memory>
#include <vector>

#include "../../Common/Instruction.h"
#include "../../Compiler/ASTNodes.h"
#include "../../Compiler/SymbolTable.h"

namespace backend {

struct LogicalIR {
    std::vector<Instruction> instructions;
    std::vector<int32_t> dataWords;
    int32_t dataBaseAddress = 4096;
};

class AstOptimizer {
public:
    void optimize(std::vector<std::unique_ptr<ASTNode>>& ast) const;
};

class IrOptimizer {
public:
    void optimize(LogicalIR& ir) const;
};

class BackendPipeline {
public:
    LogicalIR lowerToLogicalIr(const std::vector<std::unique_ptr<ASTNode>>& ast, SymbolTable& symbols) const;
};

}  // namespace backend

#endif
#ifndef FRONTEND_PIPELINE_H
#define FRONTEND_PIPELINE_H

#include <memory>
#include <string>
#include <vector>

#include "../../Compiler/ASTNodes.h"

namespace frontend {

struct FrontendResult {
    std::vector<std::unique_ptr<ASTNode>> ast;
};

class FrontendPipeline {
public:
    FrontendResult compileToAst(const std::string& source) const;
};

}  // namespace frontend

#endif
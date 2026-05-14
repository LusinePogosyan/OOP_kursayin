#include "FrontendPipeline.h"

#include "../../Compiler/Lexer.h"
#include "../../Compiler/Parser.h"

namespace frontend {

FrontendResult FrontendPipeline::compileToAst(const std::string& source) const {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    FrontendResult result;
    result.ast = parser.parse();
    return result;
}

}  // namespace frontend

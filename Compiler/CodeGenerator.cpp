#include "CodeGenerator.h"
#include <stdexcept>

void CodeGenerator::generate(ASTNode* node) {
    if (node == nullptr) {
        return;
    }
    if (auto func = dynamic_cast<FunctionNode*>(node)) {
        for (auto& stmt : func->body) generate(stmt.get());
    } else if (auto decl = dynamic_cast<DeclarationNode*>(node)) {
        if (!symbolTable.existsInCurrentScope(decl->name)) {
            SymbolType st = SymbolType::Local;
            if (decl->isGlobal) {
                st = SymbolType::Global;
            } else if (decl->isStatic) {
                st = SymbolType::Static;
            }
            symbolTable.addSymbol(decl->name, decl->type, st);
        }
        Symbol sym = symbolTable.lookup(decl->name);
        if (sym.stype == SymbolType::Global || sym.stype == SymbolType::Static) {
            size_t dataIndex = static_cast<size_t>(sym.offset / 4);
            if (dataSection.size() <= dataIndex) {
                dataSection.resize(dataIndex + 1, 0);
            }

            const bool isLocalStatic = (sym.stype == SymbolType::Static && !decl->isGlobal);
            if (isLocalStatic) {
                if (decl->initializer) {
                    int32_t guardAddress = 0;
                    auto it = staticInitGuardAddressByDecl.find(decl);
                    if (it == staticInitGuardAddressByDecl.end()) {
                        guardAddress = appendDataWords({0}); // 0 -> not initialized
                        staticInitGuardAddressByDecl[decl] = guardAddress;
                    } else {
                        guardAddress = it->second;
                    }

                    // if (guard != 0) skip initialization
                    instructions.push_back({OpCode::LW, 30, 0, 0, guardAddress});
                    size_t skipInitBranchIdx = instructions.size();
                    instructions.push_back({OpCode::BNE, 0, 30, 0, 0});

                    generateExpression(decl->initializer.get(), 5);
                    instructions.push_back({OpCode::SW, 0, 0, 5, dataBaseAddress + sym.offset});
                    instructions.push_back({OpCode::ADDI, 30, 0, 0, 1});
                    instructions.push_back({OpCode::SW, 0, 0, 30, guardAddress});

                    int32_t skipInitLabel = static_cast<int32_t>(instructions.size());
                    patchBranch(skipInitBranchIdx, skipInitLabel);
                }
            } else if (auto lit = dynamic_cast<IntLiteralNode*>(decl->initializer.get())) {
                dataSection[dataIndex] = lit->value;
            } else if (decl->initializer) {
                generateExpression(decl->initializer.get(), 5);
                instructions.push_back({OpCode::SW, 0, 0, 5, dataBaseAddress + sym.offset});
            }
        } else if (decl->initializer) {
            generateExpression(decl->initializer.get(), 5);
            instructions.push_back({OpCode::SW, 0, 2, 5, sym.offset});
        }
    } else if (auto ret = dynamic_cast<ReturnNode*>(node)) {
        if (ret->expression) {
            generateExpression(ret->expression.get(), 10);  // a0
        }
        instructions.push_back({OpCode::HALT, 0, 0, 0, 0});
    } else if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        generateExpression(ifNode->condition.get(), 5);
        size_t falseBranchIdx = instructions.size();
        instructions.push_back({OpCode::BEQ, 0, 5, 0, 0});  // if cond == 0 jump to else/end

        for (const auto& stmt : ifNode->thenBlock) {
            generate(stmt.get());
        }

        if (!ifNode->elseBlock.empty()) {
            size_t jumpOverElseIdx = instructions.size();
            instructions.push_back({OpCode::JAL, 0, 0, 0, 0});

            int32_t elseLabel = static_cast<int32_t>(instructions.size());
            patchBranch(falseBranchIdx, elseLabel);

            for (const auto& stmt : ifNode->elseBlock) {
                generate(stmt.get());
            }
            int32_t endLabel = static_cast<int32_t>(instructions.size());
            patchJump(jumpOverElseIdx, endLabel);
        } else {
            int32_t endLabel = static_cast<int32_t>(instructions.size());
            patchBranch(falseBranchIdx, endLabel);
        }
    } else if (auto loop = dynamic_cast<WhileNode*>(node)) {
        int32_t conditionLabel = static_cast<int32_t>(instructions.size());
        generateExpression(loop->condition.get(), 5);
        size_t exitBranchIdx = instructions.size();
        instructions.push_back({OpCode::BEQ, 0, 5, 0, 0});  // if cond == 0 -> exit

        breakPatchStack.push_back({});
        continueTargetStack.push_back(conditionLabel);

        for (const auto& stmt : loop->body) {
            generate(stmt.get());
        }
        instructions.push_back({OpCode::JAL, 0, 0, 0, conditionLabel});

        int32_t exitLabel = static_cast<int32_t>(instructions.size());
        patchBranch(exitBranchIdx, exitLabel);
        for (size_t patchIdx : breakPatchStack.back()) {
            patchJump(patchIdx, exitLabel);
        }
        breakPatchStack.pop_back();
        continueTargetStack.pop_back();
    } else if (auto doWhileNode = dynamic_cast<DoWhileNode*>(node)) {
        int32_t bodyLabel = static_cast<int32_t>(instructions.size());
        breakPatchStack.push_back({});
        continueTargetStack.push_back(bodyLabel);

        for (const auto& stmt : doWhileNode->body) {
            generate(stmt.get());
        }
        int32_t conditionLabel = static_cast<int32_t>(instructions.size());
        continueTargetStack.back() = conditionLabel;
        generateExpression(doWhileNode->condition.get(), 5);
        instructions.push_back({OpCode::BNE, 0, 5, 0, bodyLabel - static_cast<int32_t>(instructions.size())});

        int32_t exitLabel = static_cast<int32_t>(instructions.size());
        for (size_t patchIdx : breakPatchStack.back()) {
            patchJump(patchIdx, exitLabel);
        }
        breakPatchStack.pop_back();
        continueTargetStack.pop_back();
    } else if (auto forNode = dynamic_cast<ForNode*>(node)) {
        if (forNode->init) {
            generate(forNode->init.get());
        }

        size_t jumpToConditionIdx = instructions.size();
        instructions.push_back({OpCode::JAL, 0, 0, 0, 0});

        int32_t incrementLabel = static_cast<int32_t>(instructions.size());
        if (forNode->increment) {
            generateExpression(forNode->increment.get(), 5);
        }

        int32_t conditionLabel = static_cast<int32_t>(instructions.size());
        patchJump(jumpToConditionIdx, conditionLabel);

        size_t exitBranchIdx = static_cast<size_t>(-1);
        if (forNode->condition) {
            generateExpression(forNode->condition.get(), 5);
            exitBranchIdx = instructions.size();
            instructions.push_back({OpCode::BEQ, 0, 5, 0, 0});  // if cond == 0 -> exit
        }

        breakPatchStack.push_back({});
        continueTargetStack.push_back(incrementLabel);

        for (const auto& stmt : forNode->body) {
            generate(stmt.get());
        }
        instructions.push_back({OpCode::JAL, 0, 0, 0, incrementLabel});

        int32_t exitLabel = static_cast<int32_t>(instructions.size());
        if (exitBranchIdx != static_cast<size_t>(-1)) {
            patchBranch(exitBranchIdx, exitLabel);
        }
        for (size_t patchIdx : breakPatchStack.back()) {
            patchJump(patchIdx, exitLabel);
        }
        breakPatchStack.pop_back();
        continueTargetStack.pop_back();
    } else if (dynamic_cast<BreakNode*>(node)) {
        if (breakPatchStack.empty()) {
            throw std::runtime_error("Break used outside breakable construct");
        }
        breakPatchStack.back().push_back(instructions.size());
        instructions.push_back({OpCode::JAL, 0, 0, 0, 0});
    } else if (dynamic_cast<ContinueNode*>(node)) {
        if (continueTargetStack.empty()) {
            throw std::runtime_error("Continue used outside loop");
        }
        instructions.push_back({OpCode::JAL, 0, 0, 0, continueTargetStack.back()});
    } else if (auto printNode = dynamic_cast<PrintNode*>(node)) {
        generateExpression(printNode->expression.get(), 10);
        instructions.push_back({OpCode::PRINT, 0, 10, 0, 0});
    }
    else if (auto sw = dynamic_cast<SwitchNode*>(node)) {
        struct SectionRef {
            bool isDefault;
            int caseValue;
            const std::vector<std::unique_ptr<ASTNode>>* statements;
        };

        std::vector<SectionRef> sections;
        if (!sw->orderedSections.empty()) {
            for (const auto& section : sw->orderedSections) {
                sections.push_back({section.isDefault, section.caseValue, &section.statements});
            }
        } else {
            for (const auto& [caseValue, stmts] : sw->cases) {
                sections.push_back({false, caseValue, &stmts});
            }
            if (!sw->defaultCase.empty()) {
                sections.push_back({true, 0, &sw->defaultCase});
            }
        }

        if (sections.empty()) {
            return;
        }

        bool hasCase = false;
        int minCase = 0;
        int maxCase = 0;
        for (const auto& section : sections) {
            if (!section.isDefault) {
                if (!hasCase) {
                    minCase = maxCase = section.caseValue;
                    hasCase = true;
                } else {
                    if (section.caseValue < minCase) minCase = section.caseValue;
                    if (section.caseValue > maxCase) maxCase = section.caseValue;
                }
            }
        }

        if (!hasCase) {
            for (const auto& section : sections) {
                if (section.isDefault) {
                    for (const auto& stmt : *section.statements) {
                        if (dynamic_cast<BreakNode*>(stmt.get()) != nullptr) {
                            return;
                        }
                        generate(stmt.get());
                    }
                    return;
                }
            }
            return;
        }

        const int range = maxCase - minCase + 1;
        std::map<int, int32_t> caseToLabel;
        int32_t defaultLabel = -1;

        generateExpression(sw->condition.get(), 5);
        instructions.push_back({OpCode::ADDI, 6, 0, 0, minCase});
        instructions.push_back({OpCode::SUB, 5, 5, 6, 0});
        size_t lowBoundIdx = instructions.size();
        instructions.push_back({OpCode::BLT, 0, 5, 0, 0});
        instructions.push_back({OpCode::ADDI, 6, 0, 0, range});
        size_t hiBoundIdx = instructions.size();
        instructions.push_back({OpCode::BGE, 0, 5, 6, 0});
        instructions.push_back({OpCode::SLL, 7, 5, 0, 2});
        size_t tableBaseIdx = instructions.size();
        instructions.push_back({OpCode::ADDI, 28, 0, 0, 0});
        instructions.push_back({OpCode::ADD, 28, 28, 7, 0});
        instructions.push_back({OpCode::LW, 29, 28, 0, 0});
        instructions.push_back({OpCode::JALR, 0, 29, 0, 0});

        std::vector<size_t> breakJumps;
        for (const auto& section : sections) {
            int32_t label = static_cast<int32_t>(instructions.size());
            if (section.isDefault) {
                defaultLabel = label;
            } else {
                caseToLabel[section.caseValue] = label;
            }

            for (const auto& stmt : *section.statements) {
                if (dynamic_cast<BreakNode*>(stmt.get()) != nullptr) {
                    breakJumps.push_back(instructions.size());
                    instructions.push_back({OpCode::JAL, 0, 0, 0, 0});
                    break;
                }
                generate(stmt.get());
            }
        }

        int32_t endLabel = static_cast<int32_t>(instructions.size());
        if (defaultLabel < 0) {
            defaultLabel = endLabel;
        }
        for (size_t jumpIdx : breakJumps) {
            patchJump(jumpIdx, endLabel);
        }
        patchBranch(lowBoundIdx, defaultLabel);
        patchBranch(hiBoundIdx, defaultLabel);

        std::vector<int32_t> jumpTable(range, defaultLabel);
        for (const auto& [caseValue, label] : caseToLabel) {
            jumpTable[caseValue - minCase] = label;
        }
        instructions[tableBaseIdx].imm = appendDataWords(jumpTable);
    } else if (auto expr = dynamic_cast<ExprNode*>(node)) {
        generateExpression(expr, 5);
    }
}

void CodeGenerator::generateExpression(ExprNode* expr, int rd) {
    if (auto lit = dynamic_cast<IntLiteralNode*>(expr)) {
        instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, lit->value});
    } else if (auto bin = dynamic_cast<BinaryOpNode*>(expr)) {
        if (bin->op == "=") {
            auto* lhsVar = dynamic_cast<VariableNode*>(bin->left.get());
            if (!lhsVar) {
                throw std::runtime_error("Assignment left side must be variable");
            }
            Symbol sym = symbolTable.lookup(lhsVar->name);
            generateExpression(bin->right.get(), rd);
            if (sym.stype == SymbolType::Global || sym.stype == SymbolType::Static) {
                instructions.push_back({OpCode::SW, 0, 0, (uint8_t)rd, dataBaseAddress + sym.offset});
            } else {
                instructions.push_back({OpCode::SW, 0, 2, (uint8_t)rd, sym.offset});
            }
            return;
        }
        generateExpression(bin->left.get(), rd);
        generateExpression(bin->right.get(), rd + 1);
        if (bin->op == "+") {
            instructions.push_back({OpCode::ADD, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "-") {
            instructions.push_back({OpCode::SUB, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "*") {
            instructions.push_back({OpCode::MUL, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "/") {
            instructions.push_back({OpCode::DIV, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "%") {
            instructions.push_back({OpCode::MOD, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "&") {
            instructions.push_back({OpCode::AND, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "|") {
            instructions.push_back({OpCode::OR, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "^") {
            instructions.push_back({OpCode::XOR, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "<<") {
            instructions.push_back({OpCode::SLL, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == ">>") {
            instructions.push_back({OpCode::SRL, (uint8_t)rd, (uint8_t)rd, (uint8_t)(rd + 1), 0});
        } else if (bin->op == "&&" || bin->op == "||") {
            size_t branchIdx = instructions.size();
            if (bin->op == "&&") {
                instructions.push_back({OpCode::BEQ, 0, (uint8_t)rd, 0, 0});
            } else {
                instructions.push_back({OpCode::BNE, 0, (uint8_t)rd, 0, 0});
            }
            instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, 0});
            size_t jumpIdx = instructions.size();
            instructions.push_back({OpCode::JAL, 0, 0, 0, 0});
            int32_t trueLabel = static_cast<int32_t>(instructions.size());
            patchBranch(branchIdx, trueLabel);
            instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, 1});
            int32_t endLabel = static_cast<int32_t>(instructions.size());
            patchJump(jumpIdx, endLabel);
        } else if (bin->op == "<" || bin->op == ">" || bin->op == "<=" ||
                   bin->op == ">=" || bin->op == "==" || bin->op == "!=") {
            size_t branchIdx = instructions.size();
            if (bin->op == "<") {
                instructions.push_back({OpCode::BLT, 0, (uint8_t)rd, (uint8_t)(rd + 1), 0});
            } else if (bin->op == ">") {
                instructions.push_back({OpCode::BLT, 0, (uint8_t)(rd + 1), (uint8_t)rd, 0});
            } else if (bin->op == "<=") {
                instructions.push_back({OpCode::BGE, 0, (uint8_t)(rd + 1), (uint8_t)rd, 0});
            } else if (bin->op == ">=") {
                instructions.push_back({OpCode::BGE, 0, (uint8_t)rd, (uint8_t)(rd + 1), 0});
            } else if (bin->op == "==") {
                instructions.push_back({OpCode::BEQ, 0, (uint8_t)rd, (uint8_t)(rd + 1), 0});
            } else {
                instructions.push_back({OpCode::BNE, 0, (uint8_t)rd, (uint8_t)(rd + 1), 0});
            }

            instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, 0});
            size_t jumpOverTrueIdx = instructions.size();
            instructions.push_back({OpCode::JAL, 0, 0, 0, 0});
            int32_t trueLabel = static_cast<int32_t>(instructions.size());
            patchBranch(branchIdx, trueLabel);
            instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, 1});
            int32_t endLabel = static_cast<int32_t>(instructions.size());
            patchJump(jumpOverTrueIdx, endLabel);
        } else {
            throw std::runtime_error("Unsupported binary operator: " + bin->op);
        }
    } else if (auto unary = dynamic_cast<UnaryOpNode*>(expr)) {
        generateExpression(unary->operand.get(), rd);
        if (unary->op == "-") {
            instructions.push_back({OpCode::SUB, (uint8_t)rd, 0, (uint8_t)rd, 0});
        } else if (unary->op == "!") {
            size_t branchIdx = instructions.size();
            instructions.push_back({OpCode::BEQ, 0, (uint8_t)rd, 0, 0});
            instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, 0});
            size_t jumpIdx = instructions.size();
            instructions.push_back({OpCode::JAL, 0, 0, 0, 0});
            int32_t trueLabel = static_cast<int32_t>(instructions.size());
            patchBranch(branchIdx, trueLabel);
            instructions.push_back({OpCode::ADDI, (uint8_t)rd, 0, 0, 1});
            int32_t endLabel = static_cast<int32_t>(instructions.size());
            patchJump(jumpIdx, endLabel);
        } else if (unary->op == "~") {
            instructions.push_back({OpCode::SUB, (uint8_t)rd, 0, (uint8_t)rd, 0});
            instructions.push_back({OpCode::ADDI, (uint8_t)rd, (uint8_t)rd, 0, -1});
        }
    } else if (auto ternary = dynamic_cast<TernaryNode*>(expr)) {
        generateExpression(ternary->condition.get(), rd);
        size_t falseBranchIdx = instructions.size();
        instructions.push_back({OpCode::BEQ, 0, (uint8_t)rd, 0, 0});
        generateExpression(ternary->trueExpr.get(), rd);
        size_t jumpEndIdx = instructions.size();
        instructions.push_back({OpCode::JAL, 0, 0, 0, 0});
        int32_t falseLabel = static_cast<int32_t>(instructions.size());
        patchBranch(falseBranchIdx, falseLabel);
        generateExpression(ternary->falseExpr.get(), rd);
        int32_t endLabel = static_cast<int32_t>(instructions.size());
        patchJump(jumpEndIdx, endLabel);
    } else if (auto cast = dynamic_cast<CastNode*>(expr)) {
        generateExpression(cast->expression.get(), rd);
    } else if (auto var = dynamic_cast<VariableNode*>(expr)) {
        Symbol s = symbolTable.lookup(var->name);
        if (s.stype == SymbolType::Global || s.stype == SymbolType::Static) {
            instructions.push_back({OpCode::LW, (uint8_t)rd, 0, 0, dataBaseAddress + s.offset});
        } else {
            instructions.push_back({OpCode::LW, (uint8_t)rd, 2, 0, s.offset}); // 2 = sp
        }
    } else {
        throw std::runtime_error("Unsupported expression node");
    }
}

int CodeGenerator::appendDataWords(const std::vector<int32_t>& words) {
    int32_t base = dataBaseAddress + static_cast<int32_t>(dataSection.size() * sizeof(int32_t));
    dataSection.insert(dataSection.end(), words.begin(), words.end());
    return base;
}

void CodeGenerator::patchJump(size_t index, int32_t target) {
    instructions[index].imm = target;
}

void CodeGenerator::patchBranch(size_t index, int32_t target) {
    instructions[index].imm = target - static_cast<int32_t>(index);
}
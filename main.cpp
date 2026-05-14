#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Compiler/SymbolTable.h"
#include "src/backend/BackendPipeline.h"
#include "src/frontend/FrontendPipeline.h"
#include "src/linker/ExecutableFormat.h"
#include "src/runtime/VmMonitor.h"

namespace {
struct CliOptions {
    bool quiet = false;
    bool testMode = false;
    int expectedReturn = 0;
    std::string sourcePath;
    std::string sourceList;
};

std::string readFileText(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open source file: " + path);
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::string joinSources(const std::string& csvPaths) {
    std::stringstream ss(csvPaths);
    std::string item;
    std::string merged;
    while (std::getline(ss, item, ',')) {
        if (item.empty()) continue;
        if (!merged.empty()) merged += "\n";
        merged += readFileText(item);
        merged += "\n";
    }
    return merged;
}

CliOptions parseCli(int argc, char** argv) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--source" && i + 1 < argc) {
            opts.sourcePath = argv[++i];
        } else if (arg == "--sources" && i + 1 < argc) {
            opts.sourceList = argv[++i];
        } else if (arg == "--quiet") {
            opts.quiet = true;
        } else if (arg == "--test" && i + 2 < argc) {
            opts.testMode = true;
            opts.sourcePath = argv[++i];
            opts.expectedReturn = std::stoi(argv[++i]);
            opts.quiet = true;
        }
    }
    return opts;
}
}

int main(int argc, char** argv) {
    try {
        std::string source;
        CliOptions opts = parseCli(argc, argv);

        if (!opts.sourceList.empty()) {
            source = joinSources(opts.sourceList);
        } else if (!opts.sourcePath.empty()) {
            if (opts.sourcePath.find(',') != std::string::npos) {
                source = joinSources(opts.sourcePath);
            } else {
                source = readFileText(opts.sourcePath);
            }
        } else {
            source =
            "int g = 5; "
            "int main() { "
            "int acc = 0; "
            "for(int i = 0; i < 4; i = i + 1) { "
            "static int s = i + 1; "
            "acc = acc + s; "
            "} "
            "g = g + 1; "
            "acc = acc + g; "
            "return acc; "
            "}";
        }

        if (!opts.quiet) {
            std::cout << "Compiling source: " << source << "\n";
        }

        // 1) Frontend: Parser -> AST
        frontend::FrontendPipeline frontend;
        auto frontendResult = frontend.compileToAst(source);

        // 2) AST optimizer stage (placeholder hook for project requirements)
        backend::AstOptimizer astOptimizer;
        astOptimizer.optimize(frontendResult.ast);

        // 3) IR translation
        SymbolTable st;

        backend::BackendPipeline backend;
        auto ir = backend.lowerToLogicalIr(frontendResult.ast, st);

        // 4) IR optimizer stage (placeholder hook for project requirements)
        backend::IrOptimizer irOptimizer;
        irOptimizer.optimize(ir);

        // 5) Final program image
        std::vector<Instruction> program = ir.instructions;
        program.push_back({OpCode::HALT, 0, 0, 0, 0});

        linker_stage::ToolchainLinker linker;
        auto image = linker.linkToImage(program, ir.dataWords);
        linker.writeExecutable(image, "a.out.exe");

        // 6) VM Monitor runtime
        runtime::VmMonitor monitor(65536);
        if (!opts.quiet) {
            std::cout << "--- Executing Generated Code ---\n";
        }
        monitor.run(program, ir.dataWords, static_cast<uint32_t>(ir.dataBaseAddress));
        int result = monitor.readRegister(10);

        if (opts.testMode) {
            if (result != opts.expectedReturn) {
                std::cerr << "Test failed for " << opts.sourcePath << ": got " << result
                          << ", expected " << opts.expectedReturn << "\n";
                return 1;
            }
            return 0;
        }

        if (!opts.quiet) {
            monitor.dumpRegisters();
            std::cout << "return value (a0): " << result << "\n";
        } else {
            std::cout << result << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error during compilation/execution: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
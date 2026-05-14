#include "SymbolTable.h"

void SymbolTable::enterScope() {
    scopes.push_back({});
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
        // Նշում՝ իրական կոմպիլյատորում stack offset-ը նույնպես պետք է հետ բերել, 
        // բայց պարզության համար մենք շարունակում ենք աճեցնել այն։
    }
}

void SymbolTable::addSymbol(const std::string& name, const std::string& type, SymbolType stype) {
    if (existsInCurrentScope(name)) {
        throw std::runtime_error("Symbol already declared in this scope: " + name);
    }

    Symbol sym;
    sym.name = name;
    sym.dataType = type;
    sym.stype = stype;

    if (stype == SymbolType::Local) {
        // RISC-V-ում stack-ը սովորաբար աճում է ներքև, բայց մենք offset-ը կպահենք դրական
        sym.offset = currentStackOffset;
        currentStackOffset += 4; // Յուրաքանչյուր int/ptr զբաղեցնում է 4 բայթ
    } else {
        sym.offset = currentGlobalOffset;
        currentGlobalOffset += 4;
    }

    scopes.back()[name] = sym;
}

Symbol SymbolTable::lookup(const std::string& name) {
    // Փնտրում ենք վերջին ավելացված scope-ից դեպի սկիզբ (LEDA սկզբունքով)
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return (*it)[name];
        }
    }
    throw std::runtime_error("Undefined symbol: " + name);
}

bool SymbolTable::existsInCurrentScope(const std::string& name) {
    return scopes.back().find(name) != scopes.back().end();
}

void SymbolTable::resetLocalOffsets() {
    currentStackOffset = 8; // reserve 0: saved ra, 4: saved fp
}
#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <stdexcept>

enum class SymbolType { Local, Global, Static };

struct Symbol {
    std::string name;
    std::string dataType;
    SymbolType stype;
    int offset; // Հեռավորությունը Stack Pointer-ից (sp) կամ Global Pointer-ից (gp)
};

class SymbolTable {
private:
    // Յուրաքանչյուր map ներկայացնում է մեկ Scope (օր.՝ ֆունկցիայի ներսում կամ if-ի բլոկում)
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    int currentStackOffset;
    int currentGlobalOffset;

public:
    SymbolTable() : currentStackOffset(0), currentGlobalOffset(0) {
        // Սկզբնական գլոբալ scope
        scopes.push_back({});
    }

    void enterScope();
    void exitScope();
    
    // Ավելացնում է նոր փոփոխական
    void addSymbol(const std::string& name, const std::string& type, SymbolType stype);
    
    // Փնտրում է փոփոխականը բոլոր հասանելի scope-երում (սկսած ամենաներքինից)
    Symbol lookup(const std::string& name);
    
    bool existsInCurrentScope(const std::string& name);
    void resetLocalOffsets();
    int getRegister(const std::string& name) {
        if (table.find(name) != table.end()) {
            return table[name].regIndex; // Ենթադրենք Symbol-ը ունի regIndex դաշտ
        }
        return -1; // Կամ գցիր runtime_error
    }

private:
    struct SymbolInfo {
        std::string type;
        int regIndex;
    };
    std::map<std::string, SymbolInfo> table;
};

#endif
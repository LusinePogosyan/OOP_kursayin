#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>

enum class TokenType {
    // Keywords
    Int, Float, Double, Char, Void,
    If, Else, While, For, Do, Switch, Case, Default,
    Return, Break, Continue, Goto,
    Static, Extern, Const,
    Inline,
    True, False,
    Class, Struct, Union, Enum,
    Public, Private, Protected,
    StaticCast, DynamicCast, ReinterpretCast, ConstCast,
    Print,
    
    // Literals & Identifiers
    Identifier, Number, StringLiteral, CharLiteral,
    
    // Operators
    Assign,       // =
    Plus, Minus, Star, Slash, Percent, // + - * / %
    Eq, Neq, Lt, Gt, Leq, Geq,         // == != < > <= >=
    And, Or, Bang,                     // && || !
    BitAnd, BitOr, BitXor, BitNot,     // & | ^ ~
    ShiftLeft, ShiftRight,             // << >>
    PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign,
    XorAssign, AndAssign, OrAssign, ShiftLeftAssign, ShiftRightAssign,
    Question,
    
    // Punctuation
    LParen, RParen,   // ( )
    LBrace, RBrace,   // { }
    LBracket, RBracket, // [ ]
    Semicolon, Comma, Dot, Arrow, Colon, DoubleColon, // ; , . -> : ::
    
    EndOfFile,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t = TokenType::Unknown, std::string v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

#endif
#ifndef PARS_H
#define PARS_H

#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <unordered_map>
#include "Scanner.h"
#include "HashTable.h"

class Parser {
public:
    struct Action {
        enum Type { SHIFT, REDUCE, ACCEPT, ERROR, GOTO } type;
        int value;
        Action() = default;
        Action(Type t, int v) : type(t), value(v) {}
    };

    enum NonTerminal {
        PROGRAM, STMT_LIST, STMT, EXPR, EXPR_PRIME,
        TERM, TERM_PRIME, FACTOR, WHILE_COND,
        IF_STMT, WHILE_STMT, BLOCK, CONDITION
    };

    struct Production {
        NonTerminal lhs;
        int rhsSize;
        Production(NonTerminal l, int r) : lhs(l), rhsSize(r) {}
    };

    struct Symbol {
        enum SymbolType { TERMINAL, NON_TERMINAL } type;
        union {
            Token token;
            NonTerminal nt;
        };

        Symbol() : type(TERMINAL) { new (&token) Token(); }
        Symbol(const Token& t) : type(TERMINAL) { new (&token) Token(t); }
        Symbol(NonTerminal n) : type(NON_TERMINAL) { nt = n; }

        // Добавляем конструктор копирования
        Symbol(const Symbol& other) {
            type = other.type;
            if (type == TERMINAL) {
                new (&token) Token(other.token);
            }
            else {
                nt = other.nt;
            }
        }

        ~Symbol() {
            if (type == TERMINAL) token.~Token();
        }
    };
    enum TokenCode {
        T_BREAK = 100,
        T_CONTINUE = 101,
        T_DO = 102,
        T_ELSE = 103,
        T_IF = 104,
        T_WHILE = 105,
        T_EQ_EQ = 219,
        T_QUOTE = 200,      // "
        T_AND = 201,         // &
        T_LPAREN = 202,      // (
        T_RPAREN = 203,      // )
        T_MUL = 204,         // *
        T_PLUS = 205,        // +
        T_MINUS = 207,       // -
        T_DIV_EQ = 208,      // /=
        T_COLON = 209,       // :
        T_SEMICOLON = 210,   // ;
        T_LSHIFT = 211,      // <<
        T_EQ = 212,          // =
        T_RSHIFT = 213,      // >>
        T_LBRACKET = 214,    // [
        T_RBRACKET = 215,    // ]
        T_LBRACE = 216,      // {
        T_OR = 217,          // |
        T_OR_OR = 218,       // ||
        T_RBRACE = 219,      // }
        T_pit = 206,      // }
        T_ID = 30,           // Идентификаторы
        T_NUM = 40,           // Числовые константы
        
        T_EOF = 0
    };

    Parser(const std::vector<Token>& tokens, HashTable& hashTable);
    bool parse();
    void executeAction(const Action& action,
        std::stack<int>& states,
        std::stack<Symbol>& symbols,
        size_t& pos);
    const std::vector<std::string>& getErrors() const;

private:
    const std::vector<Token>& tokens;
    HashTable& hashTable;
    std::vector<std::string> errors;
    std::vector<Production> productions;
    std::unordered_map<int, std::unordered_map<int, Action>> actionTable;
    std::unordered_map<int, std::unordered_map<int, Action>> gotoTable;
    void writeErrorToFile(const std::string& errorMessage);
    void initialize();
    void init_Product();
    void init_actionTable();
    void init_gotoTable();
    int getTerminalCode(const Token& token) const;
    std::string getTerminalName(int term) const;
    void handleError(const Token& token, int state);
};

#endif
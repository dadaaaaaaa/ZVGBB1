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
        T_WHILE = 107,
        T_main = 106,
        T_int = 105,

        T_EQ_EQ = 214,//==
        T_ne_EQ = 200,//!=

        T_QUOTE = 201,      // "
        T_AND = 202,         // &
        T_LPAREN = 203,      // (
        T_RPAREN = 204,      // )
        T_MUL = 205,         // *
        T_PLUS = 206,        // +
        T_MINUS = 208,       // -
        T_DIV_EQ = 209,      // /=
        T_COLON = 210,       // :
        T_SEMICOLON = 211,   // ;
        T_LSHIFT = 212,      // <<
        T_EQ = 213,          // =
        T_RSHIFT = 215,      // >>
        T_LBRACKET = 216,    // [
        T_RBRACKET = 217,    // ]
        T_LBRACE = 218,      // {
        T_OR = 219,          // |
        T_OR_OR = 220,       // ||
        T_RBRACE = 221,      // }
        T_COMMA = 207,      // ,
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
    std::string getNonTerminalName(int nt) const;
void generateFormattedLALRTableToFile(const std::string& filename);
private:
    void writePostfixToFile(const std::string& postfixExpr, std::ofstream& postfixFile);

    std::string toPostfix(const std::vector<Token>& exprTokens);
    int precedence(const Token& token);

    std::string join(const std::vector<std::string>& vec, const std::string& delim);
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
    bool lookahead(int offset);
    bool isOperator(int terminalCode);
};

#endif
#ifndef PARSER_H
#define PARSERR_H
#include <vector>
#include <stack>
#include <string>
#include <unordered_map>
#include <functional>
#include "Scanner.h"
#include "HashTable.h"


class Parser {
public:
    struct Action {
        enum Type { SHIFT, REDUCE, ACCEPT, ERROR } type;
        int value;
    };
    enum NonTerminal {
        PROGRAM, STMT_LIST, STMT, EXPR, EXPR_PRIME,
        TERM, TERM_PRIME, FACTOR, WHILE_COND,
        IF_STMT, WHILE_STMT, BLOCK, CONDITION
    };
    struct Production {
        NonTerminal lhs;
        int rhsSize;
    };
    struct Symbol {
        enum { TERMINAL, NON_TERMINAL } type;
        union {
            Token token;
            NonTerminal nt;
        };
    };
    enum TokenCode {
        T_BREAK = 100,
        T_CONTINUE = 101,
        T_DO = 102,
        T_ELSE = 103,
        T_IF = 104,
        T_WHILE = 105,

        T_COM = 200,       // "
        T_AND = 201,       // &
        T_LPAREN = 202, // (
        T_RPAREN = 203, // ) 
        T_MUL = 204,       // *
        T_PLUS = 205,   // +
        T_MINUS = 206,   // +
        T_DIV_EQ = 207,    // /=
        T_COLON = 208,     // :
        T_SEMICOLON = 209, // ; 
        T_LSHIFT = 120,   // <<
        T_EQ = 211,    // = 
        T_RSHIFT = 212,   // >>
        T_LBRACKET = 213, // [
        T_RBRACKET = 214, // ]
        T_LBRACE = 215,// { 
        T_OR = 216,       // |
        T_OR_OR = 217,     // ||
        T_RBRACE = 218,// } 

        T_ID = 30,          // (30,*)
        T_NUM = 40          // (40,*)
    };

    Parser(const std::vector<Token>& tokens, HashTable& hashTable);
    bool parse();
    const std::vector<std::string>& getErrors() const;
private:
    

    const std::vector<Token>& tokens;
    HashTable& hashTable;
    std::vector<std::string> errors;

    std::vector<Production> productions;
    std::unordered_map<int, std::unordered_map<int, Action>> actionTable;
    std::unordered_map<int, std::unordered_map<int, int>> gotoTable;

    void initialize();
    void initializeProductions();
    void initializeActionTable();
    void initializeGotoTable();
    int getTerminalCode(const Token& token) const;
    std::string getTerminalName(int term) const;
    void handleError(const Token& token, int state);
};
#endif
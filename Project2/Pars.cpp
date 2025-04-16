#include "Pars.h"
#include <iostream>
#include <fstream>

Parser::Parser(const std::vector<Token>& tokens, HashTable& hashTable)
    : tokens(tokens), hashTable(hashTable) {
    initialize();
}

void Parser::initialize() {
    init_Product();
    init_actionTable();
    init_gotoTable();
}

void Parser::init_Product() {
    productions = {
        // Основные правила
        {PROGRAM, 1},          // program → stmt_list
        {STMT_LIST, 3},        // stmt_list → stmt ; stmt_list
        {STMT_LIST, 0},        // stmt_list → ε

        // Операторы
        {STMT, 4},             // stmt → id = expr ;
        {STMT, 5},             // stmt → while ( expr ) block
        {STMT, 3},             // stmt → block

        // Блоки
        {BLOCK, 3},            // block → { stmt_list }

        // Выражения
        {EXPR, 3},             // expr → expr | expr
        {EXPR, 1},             // expr → term
        {TERM, 1},             // term → factor
        {FACTOR, 1},           // factor → id
        {FACTOR, 1}            // factor → num
    };
}

void Parser::init_actionTable() {
    // Начальные состояния
    actionTable[0][T_ID] = { Action::SHIFT, 10 };      // a
    actionTable[0][T_WHILE] = { Action::SHIFT, 20 };  // while

    // Присваивание
    actionTable[10][T_EQ] = { Action::SHIFT, 11 };     // =
    actionTable[11][T_ID] = { Action::SHIFT, 10 };     // b
    actionTable[11][T_NUM] = { Action::SHIFT, 40 };    // 4

    // После числа в присваивании
    actionTable[40][T_SEMICOLON] = { Action::SHIFT, 41 }; // ;

    // После точки с запятой (новое состояние)
    actionTable[41][T_ID] = { Action::SHIFT, 10 };     // b после a=4;
    actionTable[41][T_WHILE] = { Action::SHIFT, 20 };  // while после a=4;
    actionTable[41][T_EOF] = { Action::REDUCE, 1 };    // конец программы

    // While
    actionTable[20][T_LPAREN] = { Action::SHIFT, 21 }; // (
    actionTable[21][T_ID] = { Action::SHIFT, 10 };     // a
    actionTable[21][T_NUM] = { Action::SHIFT, 40 };    // 2
    // Исправленная обработка условия while
    actionTable[50][T_RPAREN] = { Action::REDUCE, 7 };  // expr → expr | expr
    actionTable[51][T_RPAREN] = { Action::SHIFT, 52 };  // закрывающая скобка условия

    // После закрытия условия while
    actionTable[52][T_LBRACE] = { Action::SHIFT, 60 };  // {

    // Обработка идентификатора после |
    actionTable[50][T_ID] = { Action::SHIFT, 10 };
    actionTable[50][T_NUM] = { Action::SHIFT, 40 };
    // Побитовое ИЛИ
    actionTable[10][T_OR] = { Action::SHIFT, 50 };     // |
    actionTable[40][T_OR] = { Action::SHIFT, 50 };     // |
    actionTable[50][T_ID] = { Action::SHIFT, 10 };     // b
    actionTable[50][T_NUM] = { Action::SHIFT, 40 };    // 2

    // Закрывающая скобка
    actionTable[51][T_RPAREN] = { Action::SHIFT, 52 }; // )

    // Блоки
    actionTable[52][T_LBRACE] = { Action::SHIFT, 60 }; // {
    actionTable[60][T_ID] = { Action::SHIFT, 10 };     // a
    actionTable[60][T_RBRACE] = { Action::SHIFT, 61 }; // }
    // Полностью переработанная секция для условий while
    actionTable[21][T_ID] = { Action::SHIFT, 100 };  // Начало выражения
    actionTable[21][T_NUM] = { Action::SHIFT, 101 };

    // Обработка битового ИЛИ в условиях
    actionTable[100][T_OR] = { Action::SHIFT, 102 };
    actionTable[101][T_OR] = { Action::SHIFT, 102 };

    // Правый операнд после |
    actionTable[102][T_ID] = { Action::SHIFT, 100 };
    actionTable[102][T_NUM] = { Action::SHIFT, 101 };
    actionTable[203][T_RPAREN] = { Action::SHIFT, 204 }; // Принять )

    // Обработка закрывающей фигурной скобки
    actionTable[10][T_RBRACE] = { Action::REDUCE, 4 };   // stmt → expr;
    actionTable[221][T_RBRACE] = { Action::SHIFT, 222 }; // Завершение блока

    // Запрещаем повторную обработку
    actionTable[204][T_RPAREN] = { Action::ERROR, 0 };
    actionTable[222][T_RBRACE] = { Action::ERROR, 0 };
    // Закрывающая скобка условия
    actionTable[100][T_RPAREN] = { Action::REDUCE, 8 };  // expr → term
    actionTable[101][T_RPAREN] = { Action::REDUCE, 8 };  // expr → term
    actionTable[103][T_RPAREN] = { Action::SHIFT, 104 }; // expr → expr | expr
    actionTable[101][T_RPAREN] = { Action::REDUCE, 8 };  // expr → term (4)
    actionTable[103][T_RPAREN] = { Action::REDUCE, 7 };  // expr → expr | expr
    actionTable[203][T_RPAREN] = { Action::SHIFT, 104 }; // Принять закрывающую скобку
    actionTable[105][T_ID] = { Action::SHIFT, 10 };  // Разрешаем идентификаторы в теле
    actionTable[105][T_RBRACE] = { Action::SHIFT, 106 }; // Закрытие блока
    actionTable[10][T_RBRACE] = { Action::REDUCE, 8 };  // stmt → expr;
    actionTable[10][T_RBRACE] = { Action::REDUCE, 4 };   // stmt → expr;
    actionTable[203][T_RBRACE] = { Action::SHIFT, 300 }; // Завершение блока

    // Явное правило для выхода из блока
    actionTable[300][T_EOF] = { Action::REDUCE, 5 };     // while_stmt → while (expr) block
    actionTable[300][T_ID] = { Action::SHIFT, 10 };      // Разрешаем операторы после блока

    // Удаляем ошибочные переходы
    actionTable[11][T_RBRACE] = { Action::ERROR, 0 };    // Запрещаем } после =
    // Явное правило для блока
    actionTable[222][T_EOF] = { Action::REDUCE, 5 };    // while_stmt → while (expr) block
    // Обработка присваивания в теле цикла
    actionTable[10][T_EOF] = { Action::REDUCE, 4 };      // stmt → expr;
    actionTable[300][T_EOF] = { Action::REDUCE, 5 };     // while_stmt → while (expr) block
    actionTable[1][T_EOF] = { Action::ACCEPT, 0 };       // Успешное завершение

    // Явные переходы для завершающих состояний
    actionTable[40][T_EOF] = { Action::REDUCE, 8 };      // term → factor
    actionTable[203][T_EOF] = { Action::REDUCE, 7 };     // expr → expr | expr
    // Явное правило для выхода из блока
    actionTable[222][T_EOF] = { Action::REDUCE, 5 };     // while_stmt → while (expr) block
    actionTable[222][T_ID] = { Action::SHIFT, 10 };      // Разрешаем операторы после блока

    // Защита от повторной обработки
    actionTable[11][T_RBRACE] = { Action::ERROR, 0 };    // Запрещаем } после =
    // Завершение блока
    actionTable[106][T_EOF] = { Action::REDUCE, 5 }; // while_stmt → while (expr) block
    // Явное правило для while с условием
    actionTable[104][T_LBRACE] = { Action::SHIFT, 105 }; // {
    // Тело while
    actionTable[104][T_LBRACE] = { Action::SHIFT, 105 };
    // Конец программы
    actionTable[1][T_EOF] = { Action::ACCEPT, 0 };
}

void Parser::init_gotoTable() {
    // Основные переходы
    gotoTable[0][PROGRAM] = { Action::GOTO, 1 };
    gotoTable[0][STMT_LIST] = { Action::GOTO, 2 };
    gotoTable[0][STMT] = { Action::GOTO, 3 };
    gotoTable[21][EXPR] = { Action::GOTO, 200 };
    gotoTable[100][EXPR] = { Action::GOTO, 200 };
    gotoTable[101][EXPR] = { Action::GOTO, 200 };
    gotoTable[102][EXPR] = { Action::GOTO, 203 };
    gotoTable[21][CONDITION] = { Action::GOTO, 210 }; // Условие while
    gotoTable[203][CONDITION] = { Action::GOTO, 210 }; // После полного выражения
    // Переход к телу while
    gotoTable[104][STMT] = { Action::GOTO, 300 };
    // После завершения оператора
    gotoTable[41][STMT_LIST] = { Action::GOTO, 42 };
    gotoTable[42][STMT] = { Action::GOTO, 3 };
    gotoTable[21][EXPR] = { Action::GOTO, 51 };  // переход к обработке expr в условии
    gotoTable[50][EXPR] = { Action::GOTO, 51 };  // после |
    gotoTable[105][STMT_LIST] = { Action::GOTO, 220 };
    gotoTable[220][STMT] = { Action::GOTO, 221 };
    gotoTable[105][BLOCK] = { Action::GOTO, 230 };
    gotoTable[222][PROGRAM] = { Action::GOTO, 1 }; // Возврат к корню
    // Завершение блока
    gotoTable[106][STMT] = { Action::GOTO, 300 }; // Возврат из блока
    // Переход после закрытия условия
    gotoTable[52][STMT] = { Action::GOTO, 53 };  // к телу while
    // Для выражений
    gotoTable[10][EXPR] = { Action::GOTO, 12 };
    gotoTable[40][EXPR] = { Action::GOTO, 12 };
    gotoTable[50][EXPR] = { Action::GOTO, 51 };
    gotoTable[204][WHILE_COND] = { Action::GOTO, 205 };
    gotoTable[222][BLOCK] = { Action::GOTO, 223 };
    gotoTable[222][STMT_LIST] = { Action::GOTO, 2 };     // Возврат к списку операторов
    gotoTable[222][STMT] = { Action::GOTO, 3 };          // Для следующего оператора
    // Удаляем циклические переходы
    gotoTable.erase(203);
    gotoTable.erase(221);
    // Для while
    gotoTable[20][STMT] = { Action::GOTO, 22 };
    gotoTable[21][EXPR] = { Action::GOTO, 23 };
    gotoTable[23][STMT] = { Action::GOTO, 24 };
    gotoTable[203][BLOCK] = { Action::GOTO, 301 };   // Специальное состояние для }
    gotoTable[300][PROGRAM] = { Action::GOTO, 1 };       // Возврат к корню
    gotoTable[300][STMT_LIST] = { Action::GOTO, 2 };     // Возврат к списку операторов
    gotoTable[10][PROGRAM] = { Action::GOTO, 1 };        // Завершение программы
    gotoTable[300][PROGRAM] = { Action::GOTO, 1 };
    gotoTable[40][PROGRAM] = { Action::GOTO, 1 };

    // Удаляем все лишние переходы
    gotoTable.erase(203);
    gotoTable.erase(11);
    // Удаляем проблемные переходы
    gotoTable.erase(10);
    gotoTable.erase(221);
    // Для блоков
    gotoTable[60][STMT_LIST] = { Action::GOTO, 62 };
}

const std::vector<std::string>& Parser::getErrors() const {
    return errors;
}


void Parser::executeAction(const Action& action, std::stack<int>& states, std::stack<Symbol>& symbols, size_t& pos) {
    switch (action.type) {
    case Action::SHIFT:
        states.push(action.value);
        symbols.emplace(tokens[pos]);
        pos++;
        break;

    case Action::REDUCE: {
        Production& prod = productions[action.value];

        // Проверка переменных (только для идентификаторов)
        if (prod.lhs == STMT && prod.rhsSize == 3) { // Правило id = expr
            if (symbols.top().type == Symbol::TERMINAL) {
                Token id_token = symbols.top().token;
                if (id_token.tableType == 30 && !hashTable.isLexemeExists(id_token.value, 30)) {
                    errors.push_back("Line " + std::to_string(id_token.line) +
                        ": Undefined variable '" + id_token.value + "'");
                }
            }
        }

        // Удаляем rhsSize элементов из стеков
        for (int i = 0; i < prod.rhsSize; i++) {
            if (symbols.empty()) break;
            symbols.pop();
            states.pop();
        }

        // Переход по gotoTable
        if (!states.empty()) {
            auto gotoIt = gotoTable.find(states.top());
            if (gotoIt != gotoTable.end()) {
                auto gotoStateIt = gotoIt->second.find(prod.lhs);
                if (gotoStateIt != gotoIt->second.end()) {
                    states.push(gotoStateIt->second.value); // Используем .value
                    symbols.emplace(prod.lhs);
                }
                else {
                    errors.push_back("No goto state for non-terminal");
                }
            }
            else {
                errors.push_back("Invalid goto state");
            }
        }
        break;
    }

    case Action::GOTO:
        states.push(action.value);
        break;

    case Action::ACCEPT:
        // Обработка в основном цикле
        break;

    case Action::ERROR:
        handleError(tokens[pos - 1], states.top());
        break;
    }
}

bool Parser::parse() {
    std::stack<int> states;
    std::stack<Symbol> symbols;
    states.push(0);

    size_t pos = 0;
    while (pos <= tokens.size()) {
        int state = states.top();
        const Token token = pos < tokens.size() ? tokens[pos] : Token{ "$", T_EOF, -1, -1, -1 };
        int term = getTerminalCode(token);

        // Отладочный вывод
        std::cout << "State: " << state << ", Token: " << token.value
            << " (type: " << token.tableType << ", index: " << token.index << ")\n";

        if (term == T_SEMICOLON) {
            // Если в текущем состоянии есть reduce по stmt → id = expr
            if (state == 12) {
                executeAction({ Action::REDUCE, 3 }, states, symbols, pos);
                continue;
            }
            // Если в стеке есть stmt, переходим к stmt_list
            else if (!symbols.empty() && symbols.top().type == Symbol::NON_TERMINAL && symbols.top().nt == STMT) {
                executeAction({ Action::REDUCE, 1 }, states, symbols, pos);  // stmt_list → stmt ; stmt_list
                continue;
            }
        }

        auto stateActions = actionTable.find(state);
        if (stateActions == actionTable.end()) {
            handleError(token, state);
            return false;
        }

        auto actionIt = stateActions->second.find(term);
        if (actionIt == stateActions->second.end()) {
            handleError(token, state);
            return false;
        }

        executeAction(actionIt->second, states, symbols, pos);
    }
    return true;
}

void Parser::handleError(const Token& token, int state) {
    std::vector<std::string> expected;
    auto stateActions = actionTable.find(state);
    if (stateActions != actionTable.end()) {
        for (const auto& actionPair : stateActions->second) {
            expected.push_back(getTerminalName(actionPair.first));
        }
    }
    errors.push_back("Unexpected " + token.value + ", expected: ");
    if (!expected.empty()) {
        errors.back() += expected[0];
        for (size_t i = 1; i < expected.size(); i++) {
            errors.back() += ", " + expected[i];
        }
    }
}

std::string join(const std::vector<std::string>& vec, const std::string& delim) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delim;
        result += vec[i];
    }
    return result;
}

int Parser::getTerminalCode(const Token& token) const {
    if (token.tableType == 10) {
        return 100 + token.index;
    }
    else if (token.tableType == 20) {
        return 200 + token.index;
    }
    return token.tableType;
}

std::string Parser::getTerminalName(int term) const {
    switch (term) {
    case T_BREAK: return "break";
    case T_CONTINUE: return "continue";
    case T_DO: return "do";
    case T_ELSE: return "else";
    case T_IF: return "if";
    case T_WHILE: return "while";
    case T_AND: return "&";
    case T_LPAREN: return "(";
    case T_RPAREN: return ")";
    case T_MUL: return "*";
    case T_PLUS: return "+";
    case T_MINUS: return "-";
    case T_DIV_EQ: return "/=";
    case T_COLON: return ":";
    case T_SEMICOLON: return ";";
    case T_LSHIFT: return "<<";
    case T_EQ: return "=";
    case T_RSHIFT: return ">>";
    case T_LBRACKET: return "[";
    case T_RBRACKET: return "]";
    case T_LBRACE: return "{";
    case T_OR: return "|";
    case T_OR_OR: return "||";
    case T_RBRACE: return "}";
    case T_ID: return "identifier";
    case T_NUM: return "number";
    default: return "unknown";
    }
}
#include "Pars.h"
#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <unordered_map>
#include <string>

// Обновленный метод Parser::toPostfix
// Обновленный метод Parser::toPostfix
int Parser::precedence(const Token& token) {
    switch (getTerminalCode(token)) {
    case T_OR_OR: return 1;
    case T_AND: return 2;
    case T_EQ_EQ: case T_ne_EQ: return 3;
    case T_PLUS: case T_MINUS: return 4;
    case T_MUL: return 5;
    case T_EQ: return 6; // Присваивание имеет самый низкий приоритет
    default: return 0;
    }
}
bool Parser::isOperator(int terminalCode) {
    return terminalCode == T_PLUS || terminalCode == T_MINUS ||
        terminalCode == T_MUL || terminalCode == T_EQ ||
        terminalCode == T_EQ_EQ || terminalCode == T_ne_EQ ||
        terminalCode == T_OR_OR || terminalCode == T_AND;
}
std::string Parser::toPostfix(const std::vector<Token>& exprTokens) {
    std::stack<Token> operators;
    std::vector<std::string> output; // Будем хранить только значения
    std::string conditionKeyword; // Для хранения if/while

    for (const Token& token : exprTokens) {
        int terminalCode = getTerminalCode(token);

        // Сохраняем if/while для добавления в конце
        if (terminalCode == T_IF || terminalCode == T_WHILE) {
            conditionKeyword = token.value;
            continue;
        }

        if (terminalCode == T_NUM || terminalCode == T_ID) {
            output.push_back(token.value);
        }
        else if (isOperator(terminalCode)) {
            while (!operators.empty() &&
                getTerminalCode(operators.top()) != T_LPAREN &&
                precedence(operators.top()) >= precedence(token)) {
                output.push_back(operators.top().value);
                operators.pop();
            }
            operators.push(token);
        }
        else if (terminalCode == T_LPAREN) {
            operators.push(token);
        }
        else if (terminalCode == T_RPAREN) {
            while (!operators.empty() &&
                getTerminalCode(operators.top()) != T_LPAREN) {
                output.push_back(operators.top().value);
                operators.pop();
            }
            if (!operators.empty()) operators.pop();
        }
    }

    // Добавляем оставшиеся операторы
    while (!operators.empty()) {
        output.push_back(operators.top().value);
        operators.pop();
    }

    // Добавляем if/while в конец, если они были
    if (!conditionKeyword.empty()) {
        output.push_back(conditionKeyword);
    }

    // Объединяем результат
    std::string result;
    for (const auto& val : output) {
        result += val + " ";
    }
    if (!result.empty()) result.pop_back();

    return result;
}

void Parser::writePostfixToFile(const std::string& postfixExpr) {
    std::ofstream postfixFile("postfix.txt", std::ios::app);
    if (postfixFile.is_open()) {
        postfixFile << postfixExpr << std::endl; // Записываем постфиксное выражение на новой строке
        postfixFile.close(); // Закрываем файл
    }
    else {
        std::cerr << "Unable to open postfix file!" << std::endl; // Если не удалось открыть файл
    }
}

Parser::Parser(const std::vector<Token>& tokens, HashTable& hashTable)
    : tokens(tokens), hashTable(hashTable) {
    initialize();
}

void Parser::initialize() {
    init_Product();
    init_actionTable();
    init_gotoTable();
}
//правила для сверки 
void Parser::init_Product() {
    productions = {
        // Основные правила
        {PROGRAM, 1},          //0 program → stmt_list
        {STMT_LIST, 2},        // 1stmt_list → stmt stmt_list
        {STMT_LIST, 0},        // 2stmt_list → ε
        {STMT, 4},             // 3stmt → id = expr ;
        {STMT, 3},             // 4stmt → block
        {BLOCK, 3},            // 5block → { stmt_list }
        {EXPR, 3},             // 6expr → expr + term
        {EXPR, 1},             // 7expr → term
        {TERM, 1},             // 8term → factor
        {FACTOR, 3},           // 9factor → id
          {FACTOR, 3},           //10 factor → id
            {FACTOR, 3},           // 11factor → id
              {FACTOR, 3},           // 12factor → id
                {FACTOR, 3},           // 13factor → id
                  {FACTOR, 3},           // 14factor → id
                    {FACTOR, 3},           // 15factor → id
                      {FACTOR, 5},           // 16factor → id
                        {FACTOR, 3},           // factor → id
                          {FACTOR, 3},           // factor → id
                            {FACTOR, 3},           // factor → id
                              {FACTOR, 3},           // factor → id
                            {FACTOR, 3},           // factor → id
                            {FACTOR, 3},           // factor → id
                    {FACTOR, 3},           // factor → id
              {FACTOR, 3},           // factor → id
              {FACTOR, 3},           // factor → id
        {FACTOR, 3}            // factor → num
    };
}
//таблица с тем что может стоять и где

void Parser::init_actionTable() {
    //main 
    actionTable[1][T_int] = { Action::SHIFT, 2 };      // a
    actionTable[2][T_main] = { Action::SHIFT, 3 };      // a
    actionTable[3][T_LPAREN] = { Action::SHIFT, 4 };      // (
    actionTable[4][T_RPAREN] = { Action::SHIFT, 5 };      // )
    actionTable[5][T_LBRACE] = { Action::SHIFT, 0 };      // {
    actionTable[6][T_RBRACE] = { Action::ACCEPT, 0 };      // }

    // Начальные состояния
    actionTable[0][T_ID] = { Action::SHIFT, 10 };      // a
    actionTable[0][T_IF] = { Action::SHIFT, 20 };      // if
    actionTable[0][T_WHILE] = { Action::SHIFT, 40 };  //while
    actionTable[0][T_EOF] = { Action::ACCEPT, 0 };
    actionTable[0][T_RBRACE] = { Action::SHIFT, 6 };      // }

    //while 
    // Условный оператор
    actionTable[40][T_LPAREN] = { Action::SHIFT, 41 };   // (
    actionTable[41][T_ID] = { Action::SHIFT, 42 };       // a

    actionTable[42][T_ne_EQ] = { Action::SHIFT, 49 };      // a !=
    actionTable[42][T_EQ_EQ] = { Action::SHIFT, 49 };// a= =

    actionTable[49][T_ID] = { Action::SHIFT, 50 };      // b
    actionTable[49][T_NUM] = { Action::SHIFT, 50 };      // 4

    actionTable[50][T_RPAREN] = { Action::SHIFT, 45 };    // )

    actionTable[50][T_OR] = { Action::SHIFT,41 };      // |
    actionTable[50][T_OR_OR] = { Action::SHIFT,41 };      // ||
    actionTable[50][T_AND] = { Action::SHIFT,41 };      // &
    actionTable[50][T_PLUS] = { Action::SHIFT, 51 };      // 4
    actionTable[50][T_MINUS] = { Action::SHIFT, 51 };      // 4
    actionTable[50][T_MUL] = { Action::SHIFT, 51 };      // 4
    actionTable[50][T_RSHIFT] = { Action::SHIFT, 51 };      // 4
    actionTable[50][T_LSHIFT] = { Action::SHIFT, 51 };      // 4

    actionTable[51][T_NUM] = { Action::SHIFT, 42 };      // 4
    actionTable[51][T_ID] = { Action::SHIFT, 42 };      // b

    actionTable[42][T_OR] = { Action::SHIFT,41 };      // |
    actionTable[42][T_OR_OR] = { Action::SHIFT,41 };      // ||
    actionTable[42][T_AND] = { Action::SHIFT,41 };      // &

    actionTable[42][T_RPAREN] = { Action::SHIFT, 45 };    // )

    actionTable[45][T_LBRACE] = { Action::SHIFT, 46 };    // {

    // Добавляем ожидание stmt_list после {
    actionTable[46][T_ID] = { Action::SHIFT, 10 };      // a
    actionTable[46][T_IF] = { Action::SHIFT, 20 };      // if
    actionTable[46][T_WHILE] = { Action::SHIFT, 40 };  //while
    actionTable[46][T_BREAK] = { Action::SHIFT, 48 };  //break
    actionTable[46][T_CONTINUE] = { Action::SHIFT, 48 };//cont

    actionTable[48][T_SEMICOLON] = { Action::REDUCE, 1 };//cont;
    // Теперь, когда мы видим stmt внутри блока
    actionTable[47][T_EQ] = { Action::SHIFT, 11 };        // a =
    actionTable[47][T_DIV_EQ] = { Action::SHIFT, 11 };      // a /=

    // Закрытие блока
    actionTable[46][T_RBRACE] = { Action::SHIFT, 0 };    // Закрываем блок }

    //stroki 
    // Присваивание
    actionTable[10][T_EQ] = { Action::SHIFT, 11 };      // a =
    actionTable[10][T_DIV_EQ] = { Action::SHIFT, 11 };      //  a /=


    actionTable[11][T_ID] = { Action::SHIFT, 12 };      // b
    actionTable[11][T_NUM] = { Action::SHIFT, 12 };      // 4
    actionTable[11][T_SEMICOLON] = { Action::REDUCE, 10 };      // 4
    actionTable[11][T_PLUS] = { Action::SHIFT, 13 };//a=+

    actionTable[12][T_SEMICOLON] = { Action::REDUCE, 16 }; // a = b;

    actionTable[13][T_NUM] = { Action::REDUCE, 6 };      // 4
    actionTable[13][T_ID] = { Action::REDUCE, 6 };      // b

    actionTable[12][T_PLUS] = { Action::SHIFT, 13 };    // b +
    actionTable[12][T_MINUS] = { Action::SHIFT, 13 };    // b -
    actionTable[12][T_MUL] = { Action::SHIFT, 13 };    // b *
    actionTable[12][T_RSHIFT] = { Action::SHIFT, 13 };    // b >>
    actionTable[12][T_LSHIFT] = { Action::SHIFT, 13 };    // b <<


    //if 
    // Условный оператор

    actionTable[20][T_LPAREN] = { Action::SHIFT,21 };   // (
    actionTable[21][T_ID] = { Action::SHIFT, 22 };       // a

    actionTable[22][T_ne_EQ] = { Action::SHIFT, 29 };      // a !=
    actionTable[22][T_EQ_EQ] = { Action::SHIFT, 29 };// a= =

    actionTable[29][T_ID] = { Action::SHIFT, 60 };      // b
    actionTable[29][T_NUM] = { Action::SHIFT, 60 };      // 4

    actionTable[60][T_RPAREN] = { Action::SHIFT, 25 };    // )
    actionTable[60][T_OR] = { Action::SHIFT,21 };      // |
    actionTable[60][T_OR_OR] = { Action::SHIFT,21 };      // ||
    actionTable[60][T_AND] = { Action::SHIFT,21 };      // &
    actionTable[60][T_PLUS] = { Action::SHIFT, 61 };      // 4
    actionTable[60][T_MINUS] = { Action::SHIFT, 61 };      // 4
    actionTable[60][T_MUL] = { Action::SHIFT, 61 };      // 4
    actionTable[60][T_RSHIFT] = { Action::SHIFT, 61 };      // 4
    actionTable[60][T_LSHIFT] = { Action::SHIFT, 61 };      // 4

    actionTable[61][T_NUM] = { Action::SHIFT, 22 };      // 4
    actionTable[61][T_ID] = { Action::SHIFT, 22 };      // b

    actionTable[22][T_OR] = { Action::SHIFT,21 };      // |
    actionTable[22][T_OR_OR] = { Action::SHIFT,21 };      // ||
    actionTable[22][T_AND] = { Action::SHIFT,21 };      // &

    actionTable[22][T_RPAREN] = { Action::SHIFT, 25 };    // )

    actionTable[25][T_LBRACE] = { Action::SHIFT, 26 };    // {


    // Добавляем ожидание stmt_list после {
    actionTable[26][T_ID] = { Action::SHIFT, 27 };        // Ожидаем stmt внутри блока
    actionTable[26][T_IF] = { Action::SHIFT, 20 };        // Ожидаем if внутри блока

    // Теперь, когда мы видим stmt внутри блока
    actionTable[27][T_EQ] = { Action::SHIFT, 11 };        // stmt → id = expr;
    actionTable[27][T_DIV_EQ] = { Action::SHIFT, 11 };        // stmt → id = expr;

    // Закрытие блока
    actionTable[26][T_RBRACE] = { Action::SHIFT, 28 };    // Закрываем блок }
    //else
    actionTable[28][T_ELSE] = { Action::SHIFT, 25 };

    actionTable[28][!T_ELSE] = { Action::SHIFT, 0 };

}

//куда что за правило из 1-ой таблицы 
void Parser::init_gotoTable() {
    gotoTable[10][PROGRAM] = { Action::GOTO, 0 };    // После stmt
    gotoTable[0][BLOCK] = { Action::GOTO, 0 };      // Переход на блок
    gotoTable[6][EXPR] = { Action::GOTO, 6 };  // ф+и
    gotoTable[16][STMT] = { Action::GOTO, 3 };  // а=и
    gotoTable[1][STMT] = { Action::GOTO, 3 };  // a;

}





void Parser::executeAction(const Action& action, std::stack<int>& states, std::stack<Symbol>& symbols, size_t& pos) {
    std::cout << "Executing action: ";
    switch (action.type) {
    case Action::SHIFT:
        std::cout << "SHIFT to state " << action.value << " with token: " << tokens[pos].value << "\n";
        states.push(action.value);
        symbols.emplace(tokens[pos]);
        pos++;
        break;

    case Action::REDUCE: {
        symbols.emplace(tokens[pos]);
        states.push(action.value);

        Production& prod = productions[action.value];
        std::cout << "REDUCE by production: " << prod.lhs << " -> " << prod.rhsSize << " symbols\n";

        // Проверка переменных
        if (prod.lhs == STMT && prod.rhsSize == 3) {
            if (symbols.top().type == Symbol::TERMINAL) {
                Token id_token = symbols.top().token;
                if (id_token.tableType == 30 && !hashTable.isLexemeExists(id_token.value, 30)) {
                    errors.push_back("Line " + std::to_string(id_token.line) +
                        ": Undefined variable '" + id_token.value + "'");
                }
            }
        }

        if (prod.lhs == EXPR || prod.lhs == FACTOR) {
            for (int i = 0; i < prod.rhsSize - 1; i++) {
                if (symbols.empty()) break;
                symbols.pop();
                states.pop();

            }
        }
        else {
            for (int i = 0; i < prod.rhsSize; i++) {
                if (symbols.empty()) break;
                symbols.pop();
                states.pop();

            }
        }
        // Переход по gotoTable
        if (!states.empty()) {
            auto gotoIt = gotoTable.find(states.top());
            if (gotoIt != gotoTable.end()) {
                auto gotoStateIt = gotoIt->second.find(prod.lhs);
                if (gotoStateIt != gotoIt->second.end()) {
                    states.push(gotoStateIt->second.value);
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
        pos++;
        break;
    }

    case Action::GOTO:
        std::cout << "GOTO state " << action.value << "\n";
        states.push(action.value);
        break;

    case Action::ACCEPT:
        std::cout << "ACCEPT\n";
        break;

    case Action::ERROR:
        std::cout << "ERROR at token: " << tokens[pos - 1].value << "\n";
        handleError(tokens[pos - 1], states.top());
        break;
    }
}
const std::vector<std::string>& Parser::getErrors() const {
    return errors;
}
bool Parser::parse() {
    std::stack<int> states;
    std::stack<Symbol> symbols;
    std::vector<Token> expressionTokens;

    states.push(0);
    if (tokens.size() >= 5 &&
        tokens[0].value == "int" &&
        tokens[1].value == "main" &&
        tokens[2].value == "(" &&
        tokens[3].value == ")") {
        size_t pos = 0;
        // Обрабатываем заголовок main
        executeAction({ Action::SHIFT, 1 }, states, symbols, pos); // int
        executeAction({ Action::SHIFT, 2 }, states, symbols, pos); // main
        executeAction({ Action::SHIFT, 3 }, states, symbols, pos); // (
        executeAction({ Action::SHIFT, 4 }, states, symbols, pos); // )
        executeAction({ Action::SHIFT, 5 }, states, symbols, pos); // {
        states.push(0);
        while (pos < tokens.size()) {
            int state = states.top();
            const Token token = pos < tokens.size() ? tokens[pos] : Token{ "$", T_EOF, -1, -1, -1 };
            int term = getTerminalCode(token);
            auto a = getTerminalCode(token);
            auto terminalCode = getTerminalCode(token);

            if (terminalCode == T_PLUS || terminalCode == T_MINUS ||
                terminalCode == T_MUL || terminalCode == T_EQ ||
                terminalCode == T_EQ_EQ || terminalCode == T_ne_EQ ||
                terminalCode == T_OR_OR || terminalCode == T_AND || terminalCode == T_ID || terminalCode == T_NUM || terminalCode == T_WHILE || terminalCode == T_IF) {
                expressionTokens.push_back(token); // Добавляем токены для постфиксного выражения
            }
            if (state == 28) {
                if (term == T_ELSE) {
                    // Если следующий токен - 'else', выполняем переход в состояние 25
                    executeAction({ Action::SHIFT, 25 }, states, symbols, pos);
                    continue; // Переходим к следующей итерации
                }
                else {
                    states.push(0);
                    continue;
                }
            }
            // Отладочный вывод
            std::cout << "State: " << state << ", Token: " << token.value
                << " (type: " << token.tableType << ", index: " << token.index << ")\n";
            if (term == T_LBRACE) {
                std::string postfixExpr = toPostfix(expressionTokens);
                writePostfixToFile(postfixExpr); // Записываем постфиксное выражение в файл
                expressionTokens.clear();
            }
            if (term == T_SEMICOLON) {
                // Если в текущем состоянии есть reduce по stmt → id = expr
                if (state == 12) {
                    executeAction({ Action::REDUCE, 3 }, states, symbols, pos);
                    std::string postfixExpr = toPostfix(expressionTokens);
                    writePostfixToFile(postfixExpr); // Записываем постфиксное выражение в файл
                    expressionTokens.clear();
                    continue;
                }
                // Если в стеке есть stmt, переходим к stmt_list
                else if (!symbols.empty() && symbols.top().type == Symbol::NON_TERMINAL && symbols.top().nt == STMT) {
                    executeAction({ Action::REDUCE, 1 }, states, symbols, pos);  // stmt_list → stmt ; stmt_list
                    std::string postfixExpr = toPostfix(expressionTokens);
                    writePostfixToFile(postfixExpr); // Записываем постфиксное выражение в файл
                    expressionTokens.clear();
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
            if (pos < tokens.size() && tokens[pos].value == "}") {
                executeAction({ Action::SHIFT, 6 }, states, symbols, pos); // }

                // Проверяем, что это конец программы
                if (pos + 1 >= tokens.size() || tokens[pos + 1].tableType == T_EOF) {
                    executeAction({ Action::ACCEPT, 0 }, states, symbols, pos);
                    return true;
                }
            }
        }
        return true;
    }
}

void Parser::handleError(const Token& token, int state) {
    std::vector<std::string> expected;
    auto stateActions = actionTable.find(state);
    if (stateActions != actionTable.end()) {
        for (const auto& actionPair : stateActions->second) {
            expected.push_back(getTerminalName(actionPair.first));
        }
    }

    std::string errorMessage = "Unexpected " + token.value + ", expected: ";
    if (!expected.empty()) {
        errorMessage += expected[0];
        for (size_t i = 1; i < expected.size(); i++) {
            errorMessage += ", " + expected[i];
        }
    }

    // Добавляем ошибку в вектор ошибок
    errors.push_back(errorMessage);

    // Записываем ошибку в файл
    writeErrorToFile(errorMessage);
}

void Parser::writeErrorToFile(const std::string& errorMessage) {
    std::ofstream errorFile("error2.txt", std::ios::app); // Открываем файл для добавления
    if (errorFile.is_open()) {
        errorFile << errorMessage << std::endl; // Записываем ошибку
        errorFile.close(); // Закрываем файл
    }
    else {
        std::cerr << "Unable to open error file!" << std::endl; // Если не удалось открыть файл
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

    case T_COMMA: return ",";
    default: return "unknown";
    }
}
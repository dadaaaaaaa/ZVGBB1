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
//правила для сверки 
void Parser::init_Product() {
    productions = {
        // Основные правила
        {PROGRAM, 1},          //0 program → stmt_list
        {STMT_LIST, 3},        // 1stmt_list → stmt stmt_list
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
    // Начальные состояния
    actionTable[0][T_ID] = { Action::SHIFT, 10 };      // a
    actionTable[0][T_IF] = { Action::SHIFT, 20 };      // if
    actionTable[0][T_WHILE] = { Action::SHIFT, 20 };  //while
    actionTable[0][T_EOF] = { Action::ACCEPT, 0 };

    // Присваивание
    actionTable[10][T_EQ] = { Action::SHIFT, 11 };      // a =

    actionTable[11][T_ID] = { Action::SHIFT, 12 };      // b
    actionTable[11][T_NUM] = { Action::SHIFT, 12 };      // 4
    actionTable[11][T_SEMICOLON] = { Action::REDUCE, 10 };      // 4
    actionTable[11][T_PLUS] = { Action::SHIFT, 13 };//a=+

    actionTable[12][T_SEMICOLON] = { Action::REDUCE, 16 }; // a = b;

    actionTable[13][T_NUM] = { Action::REDUCE, 6 };      // 4
    actionTable[13][T_ID] = { Action::REDUCE, 6 };      // b

    actionTable[12][T_PLUS] = { Action::SHIFT, 13 };    // b +

    actionTable[14][T_SEMICOLON] = { Action::REDUCE, 16 }; // a = b + c;

    // Условный оператор
    actionTable[20][T_LPAREN] = { Action::SHIFT, 21 };   // (
    actionTable[21][T_ID] = { Action::SHIFT, 22 };       // a
    actionTable[22][T_OR] = { Action::SHIFT, 23 };      // |
    actionTable[23][T_ID] = { Action::SHIFT, 24 };       // b
    actionTable[24][T_pit] = { Action::REDUCE, 16 };    // ,
    actionTable[24][T_RPAREN] = { Action::SHIFT, 25 };    // )

    actionTable[25][T_LBRACE] = { Action::SHIFT, 26 };    // {

    // Добавляем ожидание stmt_list после {
    actionTable[26][T_ID] = { Action::SHIFT, 27 };        // Ожидаем stmt внутри блока
    actionTable[26][T_IF] = { Action::SHIFT, 20 };        // Ожидаем if внутри блока

    // Теперь, когда мы видим stmt внутри блока
    actionTable[27][T_EQ] = { Action::SHIFT, 11 };        // stmt → id = expr;

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
    states.push(0);

    size_t pos = 0;
    while (pos < tokens.size()) {
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

    case T_pit: return ",";
    default: return "unknown";
    }
}
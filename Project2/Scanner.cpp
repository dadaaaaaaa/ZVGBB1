#include "Scanner.h"
#include <cctype>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>

// Конструктор
Scanner::Scanner(const std::string& filename, HashTable& hashTable)
    : file(filename), hashTable(hashTable), lineNumber(1), columnNumber(0) {
    if (!file.is_open()) {
        errors.push_back("Ошибка: не удалось открыть файл " + filename);
    }
}

bool Scanner::scan() {
    if (!file.is_open()) {
        return false;
    }

    char ch;
    bool inString = false;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;
    int multiLineCommentStartLine = 0;
    int stringStartLine = 0;

    // Для отслеживания скобок
    struct {
        int round = 0;    // ()
        int curly = 0;     // {}
        int square = 0;    // []
        int lastRoundLine = 0;
        int lastCurlyLine = 0;
        int lastSquareLine = 0;
    } brackets;

    while ((ch = getNextChar()) != EOF) {
        int currentLine = lineNumber;

        if (ch == '\n') {
            inSingleLineComment = false;
            continue;
        }

        if (!inString && !inSingleLineComment && !inMultiLineComment && isspace(ch)) {
            continue;
        }

        if (ch == '"' && !inSingleLineComment && !inMultiLineComment) {
            inString = !inString;
            if (inString) stringStartLine = currentLine;
            continue;
        }

        // Обработка комментариев
        if (!inString) {
            if (ch == '/' && peekNextChar() == '/') {
                inSingleLineComment = true;
                getNextChar();
                continue;
            }
            if (!inMultiLineComment) {
                // Обработка начала многострочного комментария
                if (ch == '/' && peekNextChar() == '*') {
                    inMultiLineComment = true; // Входим в многострочный комментарий
                    multiLineCommentStartLine = currentLine; // Запоминаем строку начала комментария
                    getNextChar(); // Считываем '*'
                    continue;
                }
            }
            else {
                // Если мы внутри многострочного комментария
                if (ch == '*' && peekNextChar() == '/') {
                    inMultiLineComment = false; // Закрываем комментарий
                    getNextChar(); // Считываем '/'
                    continue;
                }
            }
        }

        if (inString || inSingleLineComment || inMultiLineComment) {
            continue;
        }

        // Обработка скобок
        switch (ch) {
        case '(':
            brackets.round++;
            brackets.lastRoundLine = currentLine;
            break;
        case ')':
            if (brackets.round <= 0) {
                errors.push_back("Ошибка в строке " + std::to_string(currentLine) +
                    ": лишняя закрывающая скобка ')'");
            }
            else {
                brackets.round--;
            }
            break;
        case '{':
            brackets.curly++;
            brackets.lastCurlyLine = currentLine;
            break;
        case '}':
            if (brackets.curly <= 0) {
                errors.push_back("Ошибка в строке " + std::to_string(currentLine) +
                    ": лишняя закрывающая скобка '}'");
            }
            else {
                brackets.curly--;
            }
            break;
        case '[':
            brackets.square++;
            brackets.lastSquareLine = currentLine;
            break;
        case ']':
            if (brackets.square <= 0) {
                errors.push_back("Ошибка в строке " + std::to_string(currentLine) +
                    ": лишняя закрывающая скобка ']'");
            }
            else {
                brackets.square--;
            }
            break;
        }

        // Обработка токенов
        if (!inString && !inSingleLineComment && !inMultiLineComment) {
            std::string buffer;
            buffer += ch;

            char nextCh = peekNextChar();
            while (nextCh != EOF && !isspace(nextCh) &&
                !isDelimiter(std::string(1, nextCh))) {
                buffer += getNextChar();
                nextCh = peekNextChar();
            }

            if (!buffer.empty()) {
                Token token;
                if (isDelimiter(buffer)) {
                    int index = hashTable.findConstantTableIndexByValue(buffer, TABLE_DELIMITERS);
                    token = createToken(buffer, TABLE_DELIMITERS, index);
                }
                else if (isKeyword(buffer)) {
                    int index = hashTable.findConstantTableIndexByValue(buffer, TABLE_KEYWORDS);
                    token = createToken(buffer, TABLE_KEYWORDS, index);
                }
                else if (isValidIdentifier(buffer)) {
                    int index = hashTable.findHashByValue(buffer, TABLE_IDENTIFIERS);
                    token = createToken(buffer, TABLE_IDENTIFIERS, index);
                }
                else if (isValidConstant(buffer)) {
                    int index = hashTable.findHashByValue(buffer, TABLE_CONSTANTS);
                    token = createToken(buffer, TABLE_CONSTANTS, index);
                }
                else {
                    errors.push_back("Ошибка в строке " + std::to_string(currentLine) +
                        ": токен '" + buffer + "' не распознан");
                    token = createToken(buffer, -1, -1);
                }
                tokens.push_back(token);
            }
        }
    }

    // Финальные проверки
    if (inString) {
        errors.push_back("Ошибка в строке " + std::to_string(stringStartLine) +
            ": незакрытая строковая кавычка \"");
    }

    if (inMultiLineComment) {
        errors.push_back("Ошибка в строке " + std::to_string(multiLineCommentStartLine) +
            ": незакрытый комментарий /*");
    }

    // Проверка незакрытых скобок
    if (brackets.round > 0) {
        errors.push_back("Ошибка: незакрытая круглая скобка '(' из строки " +
            std::to_string(brackets.lastRoundLine));
    }
    if (brackets.curly > 0) {
        errors.push_back("Ошибка: незакрытая фигурная скобка '{' из строки " +
            std::to_string(brackets.lastCurlyLine));
    }
    if (brackets.square > 0) {
        errors.push_back("Ошибка: незакрытая квадратная скобка '[' из строки " +
            std::to_string(brackets.lastSquareLine));
    }

    // Сохранение результатов
    if (!errors.empty()) {
        saveErrorsToFile("errors.txt");
    }
    else {
        saveTokensToFile("tokens.txt");
    }

    return errors.empty();
}

bool Scanner::isKeyword(const std::string& value) {
    // 1. Проверяем точное совпадение через хеш-таблицу
    if (hashTable.findConstantTableIndexByValue(value, TABLE_KEYWORDS) != -1) {
        return true; // Это ключевое слово
    }

    // 2. Проверка на опечатки (если не нашли точное совпадение)
    const auto& keywords = hashTable.getKeywordTableValues();

    for (const auto& keyword : keywords) {
        if (value.length() != keyword.length()) {
            continue; // Длины разные → пропускаем
        }

        // Считаем различия между value и keyword
        int diffCount = 0;
        int firstDiffPos = -1;
        int secondDiffPos = -1;

        for (size_t i = 0; i < value.length(); ++i) {
            if (value[i] != keyword[i]) {
                if (firstDiffPos == -1) {
                    firstDiffPos = i;
                }
                else {
                    secondDiffPos = i;
                }
                diffCount++;
            }
        }

        // Если разница в 1 символ → опечатка
        if (diffCount == 1) {
            errors.push_back(
                "Ошибка: возможная опечатка в ключевом слове '" + value +
                "'. Возможно, вы имели в виду '" + keyword + "'."
            );
            return false;
        }

        // Если разница в 2 символа → проверяем перестановку
        if (diffCount == 2 && firstDiffPos != -1 && secondDiffPos != -1) {
            if (value[firstDiffPos] == keyword[secondDiffPos] &&
                value[secondDiffPos] == keyword[firstDiffPos]) {
                errors.push_back(
                    "Ошибка: возможная перестановка символов в ключевом слове '" + value +
                    "'. Возможно, вы имели в виду '" + keyword + "'."
                );
                return false;
            }
        }
    }

    return false; // Не ключевое слово и не опечатка
}

// Просмотр следующего символа без его извлечения
char Scanner::peekNextChar() {
    char ch = file.peek();
    return ch;
}


void Scanner::saveErrorsToFile(const std::string& filename) const {
    std::ofstream errorFile(filename);
    if (errorFile.is_open()) {
        for (const auto& error : errors) {
            errorFile << error << std::endl;
        }
        errorFile.close();
    }
    else {
        std::cerr << "Ошибка: не удалось открыть файл для записи ошибок.\n";
    }
}

void Scanner::saveTokensToFile(const std::string& filename) const {
    std::ofstream tokenFile(filename);
    if (tokenFile.is_open()) {
        for (const auto& token : tokens) {
            tokenFile << "(" << token.tableType << ", " << token.index << ")" << std::endl;
            if (token.tableType == 30)
                hashTable.addLexeme(token.value, 30); // Добавляем идентификатор в таблицу
            if (token.tableType == 40)
                hashTable.addLexeme(token.value, 40); // Добавляем константу в таблицу

        }
        tokenFile.close();
    }
    else {
        std::cerr << "Ошибка: не удалось открыть файл для записи токенов.\n";
    }
}

// Проверка на допустимый идентификатор
bool Scanner::isValidIdentifier(const std::string& identifier) const {
    if (identifier.empty() || !std::isalpha(identifier[0])) {
        return false; // Идентификатор должен начинаться с буквы
    }
    for (char c : identifier) {
        if (!std::isalnum(c) && c != '_') {
            return false; // Разрешены только буквы, цифры и символ подчеркивания
        }
    }
    return true;
}

// Проверка на допустимую константу (число или дробь)
bool Scanner::isValidConstant(const std::string& constant) const {
    std::istringstream iss(constant);
    double d;
    char c;
    return (iss >> d) && !(iss >> c); // Если удалось считать число и больше ничего нет
}

// Создать токен
Token Scanner::createToken(const std::string& value, int tableType, int index) const {
    Token token;
    token.value = value;
    token.tableType = tableType;
    token.index = index;
    token.line = lineNumber;
    token.column = columnNumber;
    return token;
}

// Получить следующий символ
char Scanner::getNextChar() {
    char ch = file.get();
    if (ch == '\n') {
        lineNumber++;
        columnNumber = 0;
    }
    else {
        columnNumber++;
    }
    return ch;
}

// Вернуть символ обратно в поток
void Scanner::ungetChar() {
    file.unget();
    if (columnNumber > 0) {
        columnNumber--;
    }
}

// Проверить, является ли символ разделителем
bool Scanner::isDelimiter(const std::string& ch) const {
    return hashTable.findConstantTableIndexByValue(ch, TABLE_DELIMITERS) != -1;
}

// Получить токены
const std::vector<Token>& Scanner::getTokens() const {
    return tokens;
}

// Получить ошибки
const std::vector<std::string>& Scanner::getErrors() const {
    return errors;
}
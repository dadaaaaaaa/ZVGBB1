#ifndef SCANNER_H
#define SCANNER_H
#include "HashTable.h"
#include <string>
#include <vector>
#include <fstream>
#include <unordered_set>

// Константы для типов таблиц
const int TABLE_DELIMITERS = 10;
const int TABLE_KEYWORDS = 20;
const int TABLE_IDENTIFIERS = 30;
const int TABLE_CONSTANTS = 40;

// Структура для хранения информации о токене
struct Token {
    std::string value;
    int tableType;
    int index;
    int line;
    int column;
};

class Scanner {
public:
    // Конструктор
    Scanner(const std::string& filename, HashTable& hashTable);

    // Основной метод сканирования
    bool scan();

    // Проверка, является ли строка ключевым словом
    bool isKeyword(const std::string& value);

    // Проверка на допустимый идентификатор
    bool isValidIdentifier(const std::string& identifier) const;

    // Проверка на допустимую константу
    bool isValidConstant(const std::string& constant) const;

    // Проверка на разделитель
    bool isDelimiter(const std::string& ch) const;

    // Получение токенов
    const std::vector<Token>& getTokens() const;

    // Получение ошибок
    const std::vector<std::string>& getErrors() const;

private:
    // Вспомогательные методы
    char peekNextChar();
    char getNextChar();
    void ungetChar();
    Token createToken(const std::string& value, int tableType, int index) const;
    void saveErrorsToFile(const std::string& filename) const;
    void saveTokensToFile(const std::string& filename) const;

    // Поля класса
    std::ifstream file;
    HashTable& hashTable;
    std::vector<Token> tokens;
    std::vector<std::string> errors;
    int lineNumber;
    int columnNumber;
};

#endif // SCANNER_H
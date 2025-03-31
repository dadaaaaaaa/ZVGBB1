#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <fstream>
#include <stack>
#include "HashTable.h"

// Типы таблиц
enum TableType {
    TABLE_KEYWORDS = 10,    // Ключевые слова
    TABLE_DELIMITERS = 20,  // Разделители
    TABLE_IDENTIFIERS = 30, // Идентификаторы
    TABLE_CONSTANTS = 40     // Константы
};

// Структура токена
struct Token {
    int tableType;  // Тип таблицы (10, 20, 30, 40)
    int index;      // Номер элемента в таблице
    std::string value; // Значение токена (для отладки)
    int line;       // Номер строки
    int column;     // Номер столбца
};

// Класс сканера
class Scanner {
public:
    Scanner(const std::string& filename, HashTable& hashTable); // Конструктор
    bool scan();                          // Запуск сканирования
    const std::vector<Token>& getTokens() const; // Получить токены
    const std::vector<std::string>& getErrors() const; // Получить ошибки
    bool isMatchingBracket(char open, char close) const; // Проверка соответствия скобок
    int levenshteinDistance(const std::string& s1, const std::string& s2) const; // Расстояние Левенштейна

private:
    std::ifstream file;                   // Файл для чтения
    HashTable& hashTable;                 // Ссылка на HashTable
    std::vector<Token> tokens;            // Массив токенов
    std::vector<std::string> errors;      // Массив ошибок
    std::stack<char> bracketStack;        // Стек для отслеживания скобок
    int lineNumber;                       // Текущая строка
    int columnNumber;                     // Текущий столбец

    // Состояния ДКА
    enum State {
        STATE_START,        // Начальное состояние
        STATE_IDENTIFIER,   // Идентификатор
        STATE_CONSTANT,     // Константа
        STATE_KEYWORD,      // Ключевое слово
        STATE_DELIMITER,    // Разделитель
        STATE_ERROR         // Ошибка
    };
    Token createToken(const std::string& value, int tableType, int index) const; // Создать токен
};

#endif // SCANNER_H
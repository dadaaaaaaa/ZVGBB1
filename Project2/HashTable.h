#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Lexeme.h" // Предполагается, что у вас есть класс Lexeme

class HashTable {
public:
    // Конструктор
    HashTable();

    // Методы для работы с таблицей
    void insert(const Lexeme& lexeme);
    void display() const;
    void addLexemeManually();
    void addLexeme(const std::string& value, int type);
    std::string findByHash(int hash, int type) const;
    int findHashByValue(const std::string& value, int type) const;
    void addAttribute(const std::string& value, const std::string& attribute);
    std::string getAttribute(const std::string& value) const;
    std::string searchInAllTables(const std::string& value) const;
    std::string findByConstantTableIndex(size_t index, int type) const;
    int findConstantTableIndexByValue(const std::string& value, int type) const;

    // Загрузка и сохранение в файл
    void loadFromFile(const std::string& filename, int type);
    void saveToFile(const std::string& filename, int type) const;
    static int getValidatedInput(const std::string& prompt, int min, int max);
private:
    // Вспомогательные методы
   
    size_t hashFunction(const std::string& value, size_t tableSize) const;
    void rehash(std::vector<std::vector<std::unique_ptr<Lexeme>>>& table);
    void sortTable(std::vector<std::unique_ptr<Lexeme>>& table);
    int determineDataType(const std::string& value) const;
    void ensureFileExists(const std::string& filename) const;
    bool isLexemeExists(const std::string& value, int type) const;

    // Таблицы
    std::vector<std::vector<std::unique_ptr<Lexeme>>> identifierTable; // Таблица идентификаторов
    std::vector<std::vector<std::unique_ptr<Lexeme>>> constantTable;   // Таблица констант
    std::vector<std::unique_ptr<Lexeme>> keywordTable;                // Таблица ключевых слов
    std::vector<std::unique_ptr<Lexeme>> delimiterTable;              // Таблица разделителей

    // Атрибуты для лексем
    std::unordered_map<std::string, std::string> attributes; // Хранение атрибутов лексем
};

#endif // HASHTABLE_H
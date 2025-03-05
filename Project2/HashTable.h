#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include "Lexeme.h"

class HashTable {
public:
    HashTable(size_t size);
    void insert(const Lexeme& lexeme); // Вставка лексемы
    void display() const;

    void addLexemeManually(); // Добавление лексемы вручную
    std::string findByHash(int hash) const; // Поиск лексемы по хеш-номеру
    int findHashByValue(const std::string& value) const; // Поиск хеш-номера по значению

    // Работа с атрибутами
    void addAttribute(const std::string& value, const std::string& attribute); // Добавление атрибута
    std::string getAttribute(const std::string& value) const; // Получение атрибута

    // Поиск лексемы во всех таблицах
    std::string searchInAllTables(const std::string& value) const;

    // Статический метод для проверки ввода
    static int getValidatedInput(const std::string& prompt, int min, int max);

    // Методы для работы с файлами
    void loadFromFile(const std::string& filename, int type); // Загрузка данных из файла
    void saveToFile(const std::string& filename, int type) const; // Сохранение данных в файл
    // Поиск лексемы в постоянной таблице по номеру
    std::string findByConstantTableIndex(size_t index) const;

    // Поиск номера лексемы в постоянной таблице по имени
    int findConstantTableIndexByValue(const std::string& value) const;
private:
    size_t hashFunction(const std::string& value) const;
    size_t findNextFreeIndex(size_t index) const; // Поиск следующего свободного индекса

    void sortConstantTable();
    std::vector<std::unique_ptr<Lexeme>> variableTable; // Переменная таблица (40 константы и 30 идентификаторы)
    std::vector<std::unique_ptr<Lexeme>> constantTable; // Постоянная таблица (20 разделители и 10 ключевые слова )

    std::unordered_map<std::string, std::string> attributes; // Атрибуты лексем

    // Метод для автоматического определения типа данных
    int determineDataType(const std::string& value) const;

    // Метод для проверки и создания файла, если он не существует
    void ensureFileExists(const std::string& filename) const;

    // Метод для проверки на повторение лексемы
    bool isLexemeExists(const std::string& value, int type) const;
};
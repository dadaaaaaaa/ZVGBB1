#include "HashTable.h"
#include <iostream>
#include <limits>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <vector>

int HashTable::getValidatedInput(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Ошибка! Введите число от " << min << " до " << max << ".\n";
        }
        else {
            break;
        }
    }
    return value;
}

HashTable::HashTable() {
    // Инициализация таблиц с начальным размером
    identifierTable.resize(16); // Начальный размер таблицы идентификаторов
    constantTable.resize(16);   // Начальный размер таблицы констант
}

size_t HashTable::hashFunction(const std::string& value, size_t tableSize) const {
    return std::hash<std::string>{}(value) % tableSize;
}

void HashTable::rehash(std::vector<std::vector<std::unique_ptr<Lexeme>>>& table) {
    size_t newSize = table.size() * 2; // Увеличиваем размер таблицы в 2 раза
    std::vector<std::vector<std::unique_ptr<Lexeme>>> newTable(newSize);

    for (auto& bucket : table) { // Используем auto& для bucket
        for (auto& lexeme : bucket) { // Используем auto& для lexeme
            if (lexeme) { // Проверяем, что lexeme не nullptr
                size_t index = hashFunction(lexeme->getValue(), newSize);
                newTable[index].push_back(std::move(lexeme)); // Перемещаем lexeme
            }
        }
    }

    table = std::move(newTable); // Перемещаем новую таблицу
}

void HashTable::sortTable(std::vector<std::unique_ptr<Lexeme>>& table) {
    // Удаляем все nullptr из таблицы
    table.erase(
        std::remove_if(table.begin(), table.end(),
            [](const std::unique_ptr<Lexeme>& lexeme) {
                return lexeme == nullptr;
            }),
        table.end());

    // Сортируем таблицу по значению лексем
    std::sort(table.begin(), table.end(),
        [](const std::unique_ptr<Lexeme>& a, const std::unique_ptr<Lexeme>& b) {
            return a->getValue() < b->getValue();
        });
}

void HashTable::insert(const Lexeme& lexeme) {
    if (isLexemeExists(lexeme.getValue(), lexeme.getType())) {
        std::cerr << "Лексема '" << lexeme.getValue() << "' уже существует!\n";
        return;
    }

    if (lexeme.getType() == 30) { // Идентификаторы
        size_t index = hashFunction(lexeme.getValue(), identifierTable.size());
        identifierTable[index].push_back(std::make_unique<Lexeme>(lexeme));

        // Рехэширование, если таблица заполнена на 75%
        if (identifierTable[index].size() > identifierTable.size() * 0.75) {
            rehash(identifierTable);
        }
    }
    else if (lexeme.getType() == 40) { // Константы
        size_t index = hashFunction(lexeme.getValue(), constantTable.size());
        constantTable[index].push_back(std::make_unique<Lexeme>(lexeme));

        // Рехэширование, если таблица заполнена на 75%
        if (constantTable[index].size() > constantTable.size() * 0.75) {
            rehash(constantTable);
        }
    }
    else if (lexeme.getType() == 10) { // Ключевые слова
        keywordTable.push_back(std::make_unique<Lexeme>(lexeme));
        sortTable(keywordTable);
    }
    else if (lexeme.getType() == 20) { // Разделители
        delimiterTable.push_back(std::make_unique<Lexeme>(lexeme));
        sortTable(delimiterTable);
    }
    else {
        std::cerr << "Неверный тип лексемы: " << lexeme.getType() << "\n";
        return;
    }

    // Сохранение в файл
    switch (lexeme.getType()) {
    case 10:
        saveToFile("keywords.txt", 10);
        break;
    case 20:
        saveToFile("delimiters.txt", 20);
        break;
    case 30:
        saveToFile("identifiers.txt", 30);
        break;
    case 40:
        saveToFile("constants.txt", 40);
        break;
    }
}

void HashTable::display() const {
    std::cout << "=== Таблица идентификаторов ===\n";
    for (size_t i = 0; i < identifierTable.size(); ++i) {
        for (const auto& lexeme : identifierTable[i]) {
            std::cout << "Хеш: " << i << "\tИмя: " << lexeme->getValue()
                << "\tТип: " << lexeme->getDataType()
                << "\tЗначение: " << (lexeme->isDefined() ? "Определено" : "Не определено") << "\n";
        }
    }

    std::cout << "=== Таблица констант ===\n";
    for (size_t i = 0; i < constantTable.size(); ++i) {
        for (const auto& lexeme : constantTable[i]) {
            std::cout << "Хеш: " << i << "\tИмя: " << lexeme->getValue()
                << "\tТип: " << lexeme->getDataType()
                << "\tЗначение: " << (lexeme->isDefined() ? "Определено" : "Не определено") << "\n";
        }
    }

    std::cout << "=== Таблица ключевых слов ===\n";
    for (size_t i = 0; i < keywordTable.size(); ++i) {
        if (keywordTable[i] != nullptr) {
            const auto& lexeme = *keywordTable[i];
            std::cout << "Номер: " << i << "\tКлючевое слово: " << lexeme.getValue()
                << "\tТип: " << lexeme.getDataType() << "\n";
        }
    }

    std::cout << "=== Таблица разделителей ===\n";
    for (size_t i = 0; i < delimiterTable.size(); ++i) {
        if (delimiterTable[i] != nullptr) {
            const auto& lexeme = *delimiterTable[i];
            std::cout << "Номер: " << i << "\tРазделитель: " << lexeme.getValue()
                << "\tТип: " << lexeme.getDataType() << "\n";
        }
    }
}

int HashTable::determineDataType(const std::string& value) const {
    for (char ch : value) {
        if (!isdigit(ch)) {
            return 2; // unsigned char
        }
    }
    return 1; // int
}

void HashTable::ensureFileExists(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.good()) {
        std::ofstream newFile(filename);
        if (newFile.is_open()) {
            std::cout << "Файл " << filename << " создан.\n";
            newFile.close();
        }
        else {
            std::cerr << "Ошибка при создании файла: " << filename << "\n";
        }
    }
}

bool HashTable::isLexemeExists(const std::string& value, int type) const {
    if (type == 30) { // Идентификаторы
        for (const auto& bucket : identifierTable) {
            for (const auto& lexeme : bucket) {
                if (lexeme->getValue() == value) {
                    return true;
                }
            }
        }
    }
    else if (type == 40) { // Константы
        for (const auto& bucket : constantTable) {
            for (const auto& lexeme : bucket) {
                if (lexeme->getValue() == value) {
                    return true;
                }
            }
        }
    }
    else if (type == 10) { // Ключевые слова
        for (const auto& lexeme : keywordTable) {
            if (lexeme->getValue() == value) {
                return true;
            }
        }
    }
    else if (type == 20) { // Разделители
        for (const auto& lexeme : delimiterTable) {
            if (lexeme->getValue() == value) {
                return true;
            }
        }
    }
    return false;
}

void HashTable::loadFromFile(const std::string& filename, int type) {
    ensureFileExists(filename);

    std::ifstream file(filename);
    if (file.is_open()) {
        std::string value;
        while (file >> value) {
            int dataType = determineDataType(value);
            insert(Lexeme(value, type, dataType, 1));
        }
        file.close();
    }
    else {
        std::cerr << "Ошибка открытия файла для чтения: " << filename << std::endl;
    }
}

void HashTable::saveToFile(const std::string& filename, int type) const {
    ensureFileExists(filename);

    std::ofstream file(filename);
    if (file.is_open()) {
        if (type == 30) { // Идентификаторы
            for (const auto& bucket : identifierTable) {
                for (const auto& lexeme : bucket) {
                    file << lexeme->getValue() << "\n";
                }
            }
        }
        else if (type == 40) { // Константы
            for (const auto& bucket : constantTable) {
                for (const auto& lexeme : bucket) {
                    file << lexeme->getValue() << "\n";
                }
            }
        }
        else if (type == 10) { // Ключевые слова
            for (const auto& lexeme : keywordTable) {
                file << lexeme->getValue() << "\n";
            }
        }
        else if (type == 20) { // Разделители
            for (const auto& lexeme : delimiterTable) {
                file << lexeme->getValue() << "\n";
            }
        }
        file.close();
    }
    else {
        std::cerr << "Ошибка открытия файла для записи: " << filename << std::endl;
    }
}

void HashTable::addLexemeManually() {
    std::string value;
    int type;

    while (true) {
        std::cout << "Введите значение лексемы (максимум 10 символов): ";
        std::cin >> value;

        if (value.length() <= 10) {
            break;
        }
        else {
            std::cerr << "Ошибка! Длина значения лексемы не должна превышать 10 символов.\n";
        }
    }

    while (true) {
        type = getValidatedInput(
            "Введите тип лексемы (30 - идентификатор, 40 - константа, 10 - ключевое слово, 20 - разделитель): ",
            10, 40);

        if (type == 10 || type == 20 || type == 30 || type == 40) {
            break;
        }
        else {
            std::cerr << "Ошибка! Введите допустимый тип лексемы (10, 20, 30, 40).\n";
        }
    }

    int dataType = determineDataType(value);
    Lexeme lexeme(value, type, dataType, 1);
    insert(lexeme);

    std::cout << "Лексема добавлена и сохранена в файл.\n";
}

void HashTable::addLexeme(const std::string& value, int type) {
    int dataType = determineDataType(value);
    Lexeme lexeme(value, type, dataType, 1);
    insert(lexeme);

    std::cout << "Лексема '" << value << "' добавлена и сохранена в файл.\n";
}

std::string HashTable::findByHash(int hash, int type) const {
    if (type == 30) { // Идентификаторы
        if (hash < 0 || hash >= identifierTable.size()) {
            return "Ошибка: неверный хеш-номер!";
        }
        for (const auto& lexeme : identifierTable[hash]) {
            return lexeme->getValue();
        }
    }
    else if (type == 40) { // Константы
        if (hash < 0 || hash >= constantTable.size()) {
            return "Ошибка: неверный хеш-номер!";
        }
        for (const auto& lexeme : constantTable[hash]) {
            return lexeme->getValue();
        }
    }
    return "Лексема не найдена!";
}

int HashTable::findHashByValue(const std::string& value, int type) const {
    if (type == 30) { // Идентификаторы
        for (size_t i = 0; i < identifierTable.size(); ++i) {
            for (const auto& lexeme : identifierTable[i]) {
                if (lexeme->getValue() == value) {
                    return static_cast<int>(i);
                }
            }
        }
    }
    else if (type == 40) { // Константы
        for (size_t i = 0; i < constantTable.size(); ++i) {
            for (const auto& lexeme : constantTable[i]) {
                if (lexeme->getValue() == value) {
                    return static_cast<int>(i);
                }
            }
        }
    }
    return -1;
}

void HashTable::addAttribute(const std::string& value, const std::string& attribute) {
    if (isLexemeExists(value, 30)) {
        attributes[value] = attribute;
        std::cout << "Атрибут добавлен к лексеме '" << value << "'.\n";
    }
    else {
        std::cerr << "Лексема '" << value << "' не найдена!\n";
    }
}

std::string HashTable::getAttribute(const std::string& value) const {
    auto it = attributes.find(value);
    if (it != attributes.end()) {
        return it->second;
    }
    return "Атрибут не найден!";
}

std::string HashTable::searchInAllTables(const std::string& value) const {
    // Поиск в таблице идентификаторов
    for (size_t i = 0; i < identifierTable.size(); ++i) {
        for (const auto& lexeme : identifierTable[i]) {
            if (lexeme->getValue() == value) {
                return "Лексема найдена в таблице идентификаторов. Хеш: " + std::to_string(i);
            }
        }
    }

    // Поиск в таблице констант
    for (size_t i = 0; i < constantTable.size(); ++i) {
        for (const auto& lexeme : constantTable[i]) {
            if (lexeme->getValue() == value) {
                return "Лексема найдена в таблице констант. Хеш: " + std::to_string(i);
            }
        }
    }

    // Поиск в таблице ключевых слов
    for (size_t i = 0; i < keywordTable.size(); ++i) {
        if (keywordTable[i] != nullptr && keywordTable[i]->getValue() == value) {
            return "Лексема найдена в таблице ключевых слов. Номер: " + std::to_string(i);
        }
    }

    // Поиск в таблице разделителей
    for (size_t i = 0; i < delimiterTable.size(); ++i) {
        if (delimiterTable[i] != nullptr && delimiterTable[i]->getValue() == value) {
            return "Лексема найдена в таблице разделителей. Номер: " + std::to_string(i);
        }
    }

    return "Лексема не найдена ни в одной из таблиц.";
}

std::string HashTable::findByConstantTableIndex(size_t index, int type) const {
    if (type == 10) { // Ключевые слова
        if (index < keywordTable.size() && keywordTable[index] != nullptr) {
            return "Лексема найдена в таблице ключевых слов. Значение: " + keywordTable[index]->getValue();
        }
    }
    else if (type == 20) { // Разделители
        if (index < delimiterTable.size() && delimiterTable[index] != nullptr) {
            return "Лексема найдена в таблице разделителей. Значение: " + delimiterTable[index]->getValue();
        }
    }
    return "Лексема не найдена в постоянной таблице.";
}

int HashTable::findConstantTableIndexByValue(const std::string& value, int type) const {
    if (type == 10) { // Ключевые слова
        for (size_t i = 0; i < keywordTable.size(); ++i) {
            if (keywordTable[i] != nullptr && keywordTable[i]->getValue() == value) {
                return static_cast<int>(i);
            }
        }
    }
    else if (type == 20) { // Разделители
        for (size_t i = 0; i < delimiterTable.size(); ++i) {
            if (delimiterTable[i] != nullptr && delimiterTable[i]->getValue() == value) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

std::vector<std::string> HashTable::getKeywordTableValues() const {
    std::vector<std::string> values;
    for (size_t i = 0; i < keywordTable.size(); ++i) {
        if (keywordTable[i]) {
            values.push_back(keywordTable[i]->getValue());
        }
    }
    return values;
}
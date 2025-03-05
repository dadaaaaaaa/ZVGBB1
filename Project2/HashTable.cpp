#include "HashTable.h"
#include <iostream>
#include <limits>
#include <cctype>

// Реализация статического метода
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

HashTable::HashTable(size_t size) : variableTable(size), constantTable(size) {}

size_t HashTable::hashFunction(const std::string& value) const {
    return std::hash<std::string>{}(value) % variableTable.size();
}

size_t HashTable::findNextFreeIndex(size_t index) const {
    while (variableTable[index] != nullptr) {
        index = (index + 1) % variableTable.size();
    }
    return index;
}

void HashTable::sortConstantTable() {
    // Удаляем все nullptr из таблицы
    constantTable.erase(
        std::remove_if(constantTable.begin(), constantTable.end(),
            [](const std::unique_ptr<Lexeme>& lexeme) {
                return lexeme == nullptr;
            }),
        constantTable.end());

    // Сортируем таблицу
    std::sort(constantTable.begin(), constantTable.end(),
        [](const std::unique_ptr<Lexeme>& a, const std::unique_ptr<Lexeme>& b) {
            return a->getValue() < b->getValue();
        });
}

void HashTable::insert(const Lexeme& lexeme) {
    if (isLexemeExists(lexeme.getValue(), lexeme.getType())) {
        std::cerr << "Лексема '" << lexeme.getValue() << "' уже существует!\n";
        return;
    }

    if (lexeme.getType() == 30 || lexeme.getType() == 40) { // Идентификаторы и константы
        size_t index = hashFunction(lexeme.getValue());

        // Если индекс занят, ищем следующий свободный
        if (variableTable[index] != nullptr) {
            index = findNextFreeIndex(index);
        }

        variableTable[index] = std::make_unique<Lexeme>(lexeme);
    }
    else if (lexeme.getType() == 10 || lexeme.getType() == 20) { // Ключевые слова и разделители
        constantTable.push_back(std::make_unique<Lexeme>(lexeme));
        sortConstantTable(); // Сортируем таблицу после добавления
    }
    else {
        std::cerr << "Неверный тип лексемы: " << lexeme.getType() << "\n";
        return;
    }

    // Сохраняем лексему в файл
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
    std::cout << "=== Переменная таблица ===\n";
    for (size_t i = 0; i < variableTable.size(); ++i) {
        if (variableTable[i] != nullptr) {
            const auto& lexeme = *variableTable[i];
            std::cout << "Хеш: " << i << "\tИмя: " << lexeme.getValue()
                << "\tТип: " << lexeme.getDataType()
                << "\tЗначение: " << (lexeme.isDefined() ? "Определено" : "Не определено") << "\n";
        }
    }

    std::cout << "=== Постоянная таблица ===\n";
    for (size_t i = 0; i < constantTable.size(); ++i) {
        if (constantTable[i] != nullptr) {
            const auto& lexeme = *constantTable[i];
            std::cout << "Номер: " << i << "\tКонстанта: " << lexeme.getValue()
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
    if (type == 30 || type == 40) { // Идентификаторы и константы
        for (size_t i = 0; i < variableTable.size(); ++i) {
            if (variableTable[i] != nullptr && variableTable[i]->getValue() == value) {
                return true;
            }
        }
    }
    else if (type == 10 || type == 20) { // Ключевые слова и разделители
        for (size_t i = 0; i < constantTable.size(); ++i) {
            if (constantTable[i] != nullptr && constantTable[i]->getValue() == value) {
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
            insert(Lexeme(value, type, dataType, 1)); // Значение "определено"
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
        if (type == 10 || type == 30) { // Переменная таблица
            for (size_t i = 0; i < variableTable.size(); ++i) {
                if (variableTable[i] != nullptr && variableTable[i]->getType() == type) {
                    file << variableTable[i]->getValue() << "\n";
                }
            }
        }
        else { // Постоянная таблица
            for (size_t i = 0; i < constantTable.size(); ++i) {
                if (constantTable[i] != nullptr && constantTable[i]->getType() == type) {
                    file << constantTable[i]->getValue() << "\n";
                }
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

    // Ввод значения лексемы
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

    // Ввод типа лексемы
    while (true) {
        type = getValidatedInput(
            "Введите тип лексемы (30 - идентификатор, 40 - константа): ",
            30, 40);

        if ( type == 30 || type == 40) {
            break;
        }
        else {
            std::cerr << "Ошибка! Введите допустимый тип лексемы (30, 40).\n";
        }
    }

    // Определение типа данных
    int dataType = determineDataType(value);

    // Создание и добавление лексемы
    Lexeme lexeme(value, type, dataType, 1); // Значение "определено"
    insert(lexeme);

    std::cout << "Лексема добавлена и сохранена в файл.\n";
}
std::string HashTable::findByHash(int hash) const {
    if (hash < 0 || hash >= variableTable.size()) {
        return "Ошибка: неверный хеш-номер!";
    }

    if (variableTable[hash] != nullptr) {
        return variableTable[hash]->getValue();
    }

    return "Лексема не найдена!";
}

int HashTable::findHashByValue(const std::string& value) const {
    for (size_t i = 0; i < variableTable.size(); ++i) {
        if (variableTable[i] != nullptr && variableTable[i]->getValue() == value) {
            return static_cast<int>(i);
        }
    }

    return -1;
}void HashTable::addAttribute(const std::string& value, const std::string& attribute) {
    if (isLexemeExists(value, 30)) { // Проверяем, существует ли лексема
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
    // Поиск в постоянной таблице (бинарный поиск)
    auto it = std::lower_bound(constantTable.begin(), constantTable.end(), value,
        [](const std::unique_ptr<Lexeme>& a, const std::string& b) {
            return a->getValue() < b;
        });

    if (it != constantTable.end() && (*it)->getValue() == value) {
        size_t index = std::distance(constantTable.begin(), it);
        return "Лексема найдена в постоянной таблице. Номер: " + std::to_string(index);
    }

    // Поиск в переменной таблице
    int hash = findHashByValue(value);
    if (hash != -1) {
        return "Лексема найдена в переменной таблице. Хеш: " + std::to_string(hash);
    }

    return "Лексема не найдена ни в одной из таблиц.";
}
std::string HashTable::findByConstantTableIndex(size_t index) const {
    if (index < constantTable.size() && constantTable[index] != nullptr) {
        return "Лексема найдена в постоянной таблице. Значение: " + constantTable[index]->getValue();
    }
    return "Лексема не найдена в постоянной таблице.";
}

int HashTable::findConstantTableIndexByValue(const std::string& value) const {
    for (size_t i = 0; i < constantTable.size(); ++i) {
        if (constantTable[i] && constantTable[i]->getValue() == value) {
            return static_cast<int>(i);
        }
    }
    return -1; // Если лексема не найдена
}
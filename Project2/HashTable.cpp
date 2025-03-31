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
            std::cerr << "������! ������� ����� �� " << min << " �� " << max << ".\n";
        }
        else {
            break;
        }
    }
    return value;
}

HashTable::HashTable() {
    // ������������� ������ � ��������� ��������
    identifierTable.resize(16); // ��������� ������ ������� ���������������
    constantTable.resize(16);   // ��������� ������ ������� ��������
}

size_t HashTable::hashFunction(const std::string& value, size_t tableSize) const {
    return std::hash<std::string>{}(value) % tableSize;
}

void HashTable::rehash(std::vector<std::vector<std::unique_ptr<Lexeme>>>& table) {
    size_t newSize = table.size() * 2; // ����������� ������ ������� � 2 ����
    std::vector<std::vector<std::unique_ptr<Lexeme>>> newTable(newSize);

    for (auto& bucket : table) { // ���������� auto& ��� bucket
        for (auto& lexeme : bucket) { // ���������� auto& ��� lexeme
            if (lexeme) { // ���������, ��� lexeme �� nullptr
                size_t index = hashFunction(lexeme->getValue(), newSize);
                newTable[index].push_back(std::move(lexeme)); // ���������� lexeme
            }
        }
    }

    table = std::move(newTable); // ���������� ����� �������
}

void HashTable::sortTable(std::vector<std::unique_ptr<Lexeme>>& table) {
    // ������� ��� nullptr �� �������
    table.erase(
        std::remove_if(table.begin(), table.end(),
            [](const std::unique_ptr<Lexeme>& lexeme) {
                return lexeme == nullptr;
            }),
        table.end());

    // ��������� ������� �� �������� ������
    std::sort(table.begin(), table.end(),
        [](const std::unique_ptr<Lexeme>& a, const std::unique_ptr<Lexeme>& b) {
            return a->getValue() < b->getValue();
        });
}

void HashTable::insert(const Lexeme& lexeme) {
    if (isLexemeExists(lexeme.getValue(), lexeme.getType())) {
        std::cerr << "������� '" << lexeme.getValue() << "' ��� ����������!\n";
        return;
    }

    if (lexeme.getType() == 30) { // ��������������
        size_t index = hashFunction(lexeme.getValue(), identifierTable.size());
        identifierTable[index].push_back(std::make_unique<Lexeme>(lexeme));

        // �������������, ���� ������� ��������� �� 75%
        if (identifierTable[index].size() > identifierTable.size() * 0.75) {
            rehash(identifierTable);
        }
    }
    else if (lexeme.getType() == 40) { // ���������
        size_t index = hashFunction(lexeme.getValue(), constantTable.size());
        constantTable[index].push_back(std::make_unique<Lexeme>(lexeme));

        // �������������, ���� ������� ��������� �� 75%
        if (constantTable[index].size() > constantTable.size() * 0.75) {
            rehash(constantTable);
        }
    }
    else if (lexeme.getType() == 10) { // �������� �����
        keywordTable.push_back(std::make_unique<Lexeme>(lexeme));
        sortTable(keywordTable);
    }
    else if (lexeme.getType() == 20) { // �����������
        delimiterTable.push_back(std::make_unique<Lexeme>(lexeme));
        sortTable(delimiterTable);
    }
    else {
        std::cerr << "�������� ��� �������: " << lexeme.getType() << "\n";
        return;
    }

    // ���������� � ����
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
    std::cout << "=== ������� ��������������� ===\n";
    for (size_t i = 0; i < identifierTable.size(); ++i) {
        for (const auto& lexeme : identifierTable[i]) {
            std::cout << "���: " << i << "\t���: " << lexeme->getValue()
                << "\t���: " << lexeme->getDataType()
                << "\t��������: " << (lexeme->isDefined() ? "����������" : "�� ����������") << "\n";
        }
    }

    std::cout << "=== ������� �������� ===\n";
    for (size_t i = 0; i < constantTable.size(); ++i) {
        for (const auto& lexeme : constantTable[i]) {
            std::cout << "���: " << i << "\t���: " << lexeme->getValue()
                << "\t���: " << lexeme->getDataType()
                << "\t��������: " << (lexeme->isDefined() ? "����������" : "�� ����������") << "\n";
        }
    }

    std::cout << "=== ������� �������� ���� ===\n";
    for (size_t i = 0; i < keywordTable.size(); ++i) {
        if (keywordTable[i] != nullptr) {
            const auto& lexeme = *keywordTable[i];
            std::cout << "�����: " << i << "\t�������� �����: " << lexeme.getValue()
                << "\t���: " << lexeme.getDataType() << "\n";
        }
    }

    std::cout << "=== ������� ������������ ===\n";
    for (size_t i = 0; i < delimiterTable.size(); ++i) {
        if (delimiterTable[i] != nullptr) {
            const auto& lexeme = *delimiterTable[i];
            std::cout << "�����: " << i << "\t�����������: " << lexeme.getValue()
                << "\t���: " << lexeme.getDataType() << "\n";
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
            std::cout << "���� " << filename << " ������.\n";
            newFile.close();
        }
        else {
            std::cerr << "������ ��� �������� �����: " << filename << "\n";
        }
    }
}

bool HashTable::isLexemeExists(const std::string& value, int type) const {
    if (type == 30) { // ��������������
        for (const auto& bucket : identifierTable) {
            for (const auto& lexeme : bucket) {
                if (lexeme->getValue() == value) {
                    return true;
                }
            }
        }
    }
    else if (type == 40) { // ���������
        for (const auto& bucket : constantTable) {
            for (const auto& lexeme : bucket) {
                if (lexeme->getValue() == value) {
                    return true;
                }
            }
        }
    }
    else if (type == 10) { // �������� �����
        for (const auto& lexeme : keywordTable) {
            if (lexeme->getValue() == value) {
                return true;
            }
        }
    }
    else if (type == 20) { // �����������
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
        std::cerr << "������ �������� ����� ��� ������: " << filename << std::endl;
    }
}

void HashTable::saveToFile(const std::string& filename, int type) const {
    ensureFileExists(filename);

    std::ofstream file(filename);
    if (file.is_open()) {
        if (type == 30) { // ��������������
            for (const auto& bucket : identifierTable) {
                for (const auto& lexeme : bucket) {
                    file << lexeme->getValue() << "\n";
                }
            }
        }
        else if (type == 40) { // ���������
            for (const auto& bucket : constantTable) {
                for (const auto& lexeme : bucket) {
                    file << lexeme->getValue() << "\n";
                }
            }
        }
        else if (type == 10) { // �������� �����
            for (const auto& lexeme : keywordTable) {
                file << lexeme->getValue() << "\n";
            }
        }
        else if (type == 20) { // �����������
            for (const auto& lexeme : delimiterTable) {
                file << lexeme->getValue() << "\n";
            }
        }
        file.close();
    }
    else {
        std::cerr << "������ �������� ����� ��� ������: " << filename << std::endl;
    }
}

void HashTable::addLexemeManually() {
    std::string value;
    int type;

    while (true) {
        std::cout << "������� �������� ������� (�������� 10 ��������): ";
        std::cin >> value;

        if (value.length() <= 10) {
            break;
        }
        else {
            std::cerr << "������! ����� �������� ������� �� ������ ��������� 10 ��������.\n";
        }
    }

    while (true) {
        type = getValidatedInput(
            "������� ��� ������� (30 - �������������, 40 - ���������, 10 - �������� �����, 20 - �����������): ",
            10, 40);

        if (type == 10 || type == 20 || type == 30 || type == 40) {
            break;
        }
        else {
            std::cerr << "������! ������� ���������� ��� ������� (10, 20, 30, 40).\n";
        }
    }

    int dataType = determineDataType(value);
    Lexeme lexeme(value, type, dataType, 1);
    insert(lexeme);

    std::cout << "������� ��������� � ��������� � ����.\n";
}

void HashTable::addLexeme(const std::string& value, int type) {
    int dataType = determineDataType(value);
    Lexeme lexeme(value, type, dataType, 1);
    insert(lexeme);

    std::cout << "������� '" << value << "' ��������� � ��������� � ����.\n";
}

std::string HashTable::findByHash(int hash, int type) const {
    if (type == 30) { // ��������������
        if (hash < 0 || hash >= identifierTable.size()) {
            return "������: �������� ���-�����!";
        }
        for (const auto& lexeme : identifierTable[hash]) {
            return lexeme->getValue();
        }
    }
    else if (type == 40) { // ���������
        if (hash < 0 || hash >= constantTable.size()) {
            return "������: �������� ���-�����!";
        }
        for (const auto& lexeme : constantTable[hash]) {
            return lexeme->getValue();
        }
    }
    return "������� �� �������!";
}

int HashTable::findHashByValue(const std::string& value, int type) const {
    if (type == 30) { // ��������������
        for (size_t i = 0; i < identifierTable.size(); ++i) {
            for (const auto& lexeme : identifierTable[i]) {
                if (lexeme->getValue() == value) {
                    return static_cast<int>(i);
                }
            }
        }
    }
    else if (type == 40) { // ���������
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
        std::cout << "������� �������� � ������� '" << value << "'.\n";
    }
    else {
        std::cerr << "������� '" << value << "' �� �������!\n";
    }
}

std::string HashTable::getAttribute(const std::string& value) const {
    auto it = attributes.find(value);
    if (it != attributes.end()) {
        return it->second;
    }
    return "������� �� ������!";
}

std::string HashTable::searchInAllTables(const std::string& value) const {
    // ����� � ������� ���������������
    for (size_t i = 0; i < identifierTable.size(); ++i) {
        for (const auto& lexeme : identifierTable[i]) {
            if (lexeme->getValue() == value) {
                return "������� ������� � ������� ���������������. ���: " + std::to_string(i);
            }
        }
    }

    // ����� � ������� ��������
    for (size_t i = 0; i < constantTable.size(); ++i) {
        for (const auto& lexeme : constantTable[i]) {
            if (lexeme->getValue() == value) {
                return "������� ������� � ������� ��������. ���: " + std::to_string(i);
            }
        }
    }

    // ����� � ������� �������� ����
    for (size_t i = 0; i < keywordTable.size(); ++i) {
        if (keywordTable[i] != nullptr && keywordTable[i]->getValue() == value) {
            return "������� ������� � ������� �������� ����. �����: " + std::to_string(i);
        }
    }

    // ����� � ������� ������������
    for (size_t i = 0; i < delimiterTable.size(); ++i) {
        if (delimiterTable[i] != nullptr && delimiterTable[i]->getValue() == value) {
            return "������� ������� � ������� ������������. �����: " + std::to_string(i);
        }
    }

    return "������� �� ������� �� � ����� �� ������.";
}

std::string HashTable::findByConstantTableIndex(size_t index, int type) const {
    if (type == 10) { // �������� �����
        if (index < keywordTable.size() && keywordTable[index] != nullptr) {
            return "������� ������� � ������� �������� ����. ��������: " + keywordTable[index]->getValue();
        }
    }
    else if (type == 20) { // �����������
        if (index < delimiterTable.size() && delimiterTable[index] != nullptr) {
            return "������� ������� � ������� ������������. ��������: " + delimiterTable[index]->getValue();
        }
    }
    return "������� �� ������� � ���������� �������.";
}

int HashTable::findConstantTableIndexByValue(const std::string& value, int type) const {
    if (type == 10) { // �������� �����
        for (size_t i = 0; i < keywordTable.size(); ++i) {
            if (keywordTable[i] != nullptr && keywordTable[i]->getValue() == value) {
                return static_cast<int>(i);
            }
        }
    }
    else if (type == 20) { // �����������
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
#include "HashTable.h"
#include <iostream>
#include <limits>
#include <cctype>

// ���������� ������������ ������
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
    // ������� ��� nullptr �� �������
    constantTable.erase(
        std::remove_if(constantTable.begin(), constantTable.end(),
            [](const std::unique_ptr<Lexeme>& lexeme) {
                return lexeme == nullptr;
            }),
        constantTable.end());

    // ��������� �������
    std::sort(constantTable.begin(), constantTable.end(),
        [](const std::unique_ptr<Lexeme>& a, const std::unique_ptr<Lexeme>& b) {
            return a->getValue() < b->getValue();
        });
}

void HashTable::insert(const Lexeme& lexeme) {
    if (isLexemeExists(lexeme.getValue(), lexeme.getType())) {
        std::cerr << "������� '" << lexeme.getValue() << "' ��� ����������!\n";
        return;
    }

    if (lexeme.getType() == 30 || lexeme.getType() == 40) { // �������������� � ���������
        size_t index = hashFunction(lexeme.getValue());

        // ���� ������ �����, ���� ��������� ���������
        if (variableTable[index] != nullptr) {
            index = findNextFreeIndex(index);
        }

        variableTable[index] = std::make_unique<Lexeme>(lexeme);
    }
    else if (lexeme.getType() == 10 || lexeme.getType() == 20) { // �������� ����� � �����������
        constantTable.push_back(std::make_unique<Lexeme>(lexeme));
        sortConstantTable(); // ��������� ������� ����� ����������
    }
    else {
        std::cerr << "�������� ��� �������: " << lexeme.getType() << "\n";
        return;
    }

    // ��������� ������� � ����
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
    std::cout << "=== ���������� ������� ===\n";
    for (size_t i = 0; i < variableTable.size(); ++i) {
        if (variableTable[i] != nullptr) {
            const auto& lexeme = *variableTable[i];
            std::cout << "���: " << i << "\t���: " << lexeme.getValue()
                << "\t���: " << lexeme.getDataType()
                << "\t��������: " << (lexeme.isDefined() ? "����������" : "�� ����������") << "\n";
        }
    }

    std::cout << "=== ���������� ������� ===\n";
    for (size_t i = 0; i < constantTable.size(); ++i) {
        if (constantTable[i] != nullptr) {
            const auto& lexeme = *constantTable[i];
            std::cout << "�����: " << i << "\t���������: " << lexeme.getValue()
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
    if (type == 30 || type == 40) { // �������������� � ���������
        for (size_t i = 0; i < variableTable.size(); ++i) {
            if (variableTable[i] != nullptr && variableTable[i]->getValue() == value) {
                return true;
            }
        }
    }
    else if (type == 10 || type == 20) { // �������� ����� � �����������
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
            insert(Lexeme(value, type, dataType, 1)); // �������� "����������"
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
        if (type == 10 || type == 30) { // ���������� �������
            for (size_t i = 0; i < variableTable.size(); ++i) {
                if (variableTable[i] != nullptr && variableTable[i]->getType() == type) {
                    file << variableTable[i]->getValue() << "\n";
                }
            }
        }
        else { // ���������� �������
            for (size_t i = 0; i < constantTable.size(); ++i) {
                if (constantTable[i] != nullptr && constantTable[i]->getType() == type) {
                    file << constantTable[i]->getValue() << "\n";
                }
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

    // ���� �������� �������
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

    // ���� ���� �������
    while (true) {
        type = getValidatedInput(
            "������� ��� ������� (30 - �������������, 40 - ���������): ",
            30, 40);

        if ( type == 30 || type == 40) {
            break;
        }
        else {
            std::cerr << "������! ������� ���������� ��� ������� (30, 40).\n";
        }
    }

    // ����������� ���� ������
    int dataType = determineDataType(value);

    // �������� � ���������� �������
    Lexeme lexeme(value, type, dataType, 1); // �������� "����������"
    insert(lexeme);

    std::cout << "������� ��������� � ��������� � ����.\n";
}
std::string HashTable::findByHash(int hash) const {
    if (hash < 0 || hash >= variableTable.size()) {
        return "������: �������� ���-�����!";
    }

    if (variableTable[hash] != nullptr) {
        return variableTable[hash]->getValue();
    }

    return "������� �� �������!";
}

int HashTable::findHashByValue(const std::string& value) const {
    for (size_t i = 0; i < variableTable.size(); ++i) {
        if (variableTable[i] != nullptr && variableTable[i]->getValue() == value) {
            return static_cast<int>(i);
        }
    }

    return -1;
}void HashTable::addAttribute(const std::string& value, const std::string& attribute) {
    if (isLexemeExists(value, 30)) { // ���������, ���������� �� �������
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
    // ����� � ���������� ������� (�������� �����)
    auto it = std::lower_bound(constantTable.begin(), constantTable.end(), value,
        [](const std::unique_ptr<Lexeme>& a, const std::string& b) {
            return a->getValue() < b;
        });

    if (it != constantTable.end() && (*it)->getValue() == value) {
        size_t index = std::distance(constantTable.begin(), it);
        return "������� ������� � ���������� �������. �����: " + std::to_string(index);
    }

    // ����� � ���������� �������
    int hash = findHashByValue(value);
    if (hash != -1) {
        return "������� ������� � ���������� �������. ���: " + std::to_string(hash);
    }

    return "������� �� ������� �� � ����� �� ������.";
}
std::string HashTable::findByConstantTableIndex(size_t index) const {
    if (index < constantTable.size() && constantTable[index] != nullptr) {
        return "������� ������� � ���������� �������. ��������: " + constantTable[index]->getValue();
    }
    return "������� �� ������� � ���������� �������.";
}

int HashTable::findConstantTableIndexByValue(const std::string& value) const {
    for (size_t i = 0; i < constantTable.size(); ++i) {
        if (constantTable[i] && constantTable[i]->getValue() == value) {
            return static_cast<int>(i);
        }
    }
    return -1; // ���� ������� �� �������
}
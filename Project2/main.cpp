#include <iostream>
#include "Scanner.h" // ���������� ������������ ���� �������
#include <clocale>

void displayMenu() {
    std::cout << "=== ���� ===\n";
    std::cout << "1. �������� ������� �������\n";
    std::cout << "2. ���������� ��� �������\n";
    std::cout << "3. ����� ������� �� ��������\n";
    std::cout << "4. ����� ������� �� ���-������\n";
    std::cout << "5. ����� ���-������ �� ��������\n";
    std::cout << "6. ����� ������� � ���������� ������� �� ������\n";
    std::cout << "7. ����� ������ ������� � ���������� ������� �� �����\n";
    std::cout << "8. �������� ������� � �������\n";
    std::cout << "9. �������� ������� �������\n";
    std::cout << "10. ��������� ������\n"; // ����� ����� ��� �������
    std::cout << "11. �����\n";
    std::cout << "�������� �����: ";
}

int main() {
    HashTable ht; // ������� ���-������� (��� �������� �������)
    setlocale(LC_ALL, "");

    // �������� ������ �� ������
    ht.loadFromFile("keywords.txt", 10); // �������� �����
    ht.loadFromFile("delimiters.txt", 20); // �����������
    ht.loadFromFile("identifiers.txt", 30); // ��������������
    ht.loadFromFile("constants.txt", 40); // ���������

    int choice;
    do {
        displayMenu();
        choice = HashTable::getValidatedInput("", 1, 11); // �������� �������� ������

        switch (choice) {
        case 1: { // �������� ������� �������
            ht.addLexemeManually();
            break;
        }
        case 2: { // ���������� ��� �������
            ht.display();
            break;
        }
        case 3: { // ����� ������� �� ��������
            std::string value;
            std::cout << "������� �������� ������� ��� ������: ";
            std::cin >> value;
            std::cout << ht.searchInAllTables(value) << "\n";
            break;
        }
        case 4: { // ����� ������� �� ���-������
            int hash, type;
            std::cout << "������� ��� ������� (30 - �������������, 40 - ���������): ";
            std::cin >> type;
            std::cout << "������� �������� ���� ��� ������ �������: ";
            std::cin >> hash;
            std::cout << ht.findByHash(hash, type) << "\n";
            break;
        }
        case 5: { // ����� ���-������ �� ��������
            std::string value;
            int type;
            std::cout << "������� ��� ������� (30 - �������������, 40 - ���������): ";
            std::cin >> type;
            std::cout << "������� �������� ������� ��� ������ ��� ���-������: ";
            std::cin >> value;
            int hash = ht.findHashByValue(value, type);
            if (hash != -1) {
                std::cout << "���-�����: " << hash << "\n";
            }
            else {
                std::cout << "������� �� �������.\n";
            }
            break;
        }
        case 6: { // ����� ������� � ���������� ������� �� ������
            size_t index;
            int type;
            std::cout << "������� ��� ������� (10 - �������� �����, 20 - �����������): ";
            std::cin >> type;
            std::cout << "������� ����� ������� � ���������� �������: ";
            std::cin >> index;
            std::cout << ht.findByConstantTableIndex(index, type) << "\n";
            break;
        }
        case 7: { // ����� ������ ������� � ���������� ������� �� �����
            std::string value;
            int type;
            std::cout << "������� ��� ������� (10 - �������� �����, 20 - �����������): ";
            std::cin >> type;
            std::cout << "������� �������� ������� ��� ������ � ������: ";
            std::cin >> value;
            int index = ht.findConstantTableIndexByValue(value, type);
            if (index != -1) {
                std::cout << "����� ������� � ���������� �������: " << index << "\n";
            }
            else {
                std::cout << "������� �� �������.\n";
            }
            break;
        }
        case 8: { // �������� ������� � �������
            std::string value, attribute;
            std::cout << "������� �������� �������: ";
            std::cin >> value;
            std::cout << "������� �������: ";
            std::cin >> attribute;
            ht.addAttribute(value, attribute);
            break;
        }
        case 9: { // �������� ������� �������
            std::string value;
            std::cout << "������� �������� �������: ";
            std::cin >> value;
            std::cout << "�������: " << ht.getAttribute(value) << "\n";
            break;
        }
        case 10: { // ������ �������
            std::string filename;
            std::cout << "������� ��� ����� ��� ������������: ";
            std::cin >> filename;

            Scanner scanner(filename, ht); // �������� ������ �� HashTable
            if (scanner.scan()) {
                std::cout << "������������ ��������� �������.\n";
                std::cout << "������:\n";
                for (const auto& token : scanner.getTokens()) {
                    std::cout << "(" << token.tableType << ", " << token.index << ") "
                        << "��������: " << token.value
                        << ", ������: " << token.line << ", �������: " << token.column << "\n";
                }
            }
            else {
                std::cout << "������������ ��������� � ��������.\n";
                for (const auto& error : scanner.getErrors()) {
                    std::cout << error << "\n";
                }
            }
            break;
        }
        case 11: { // �����
            std::cout << "����� �� ���������.\n";
            break;
        }
        default: {
            std::cout << "�������� �����. ���������� �����.\n";
        }
        }
    } while (choice != 11);

    return 0;
}
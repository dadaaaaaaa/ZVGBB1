#include <iostream>
#include "HashTable.h"
#include <clocale>
void displayMenu() {
    std::cout << "=== Меню ===\n";
    std::cout << "1. Добавить лексему вручную\n";
    std::cout << "2. Отобразить все таблицы\n";
    std::cout << "3. Поиск лексемы по значению\n";
    std::cout << "4. Поиск лексемы по хеш-номеру\n";
    std::cout << "5. Поиск хеш-номера по значению\n";
    std::cout << "6. Поиск лексемы в постоянной таблице по номеру\n";
    std::cout << "7. Поиск номера лексемы в постоянной таблице по имени\n";
    std::cout << "8. Добавить атрибут к лексеме\n";
    std::cout << "9. Получить атрибут лексемы\n";
    std::cout << "10. Выход\n";
    std::cout << "Выберите опцию: ";
}

int main() {
    HashTable ht(100);
    setlocale(LC_ALL, "");

    // Загрузка данных из файлов
    ht.loadFromFile("keywords.txt", 10); // Ключевые слова
    ht.loadFromFile("delimiters.txt", 20); // Разделители
    ht.loadFromFile("identifiers.txt", 30); // Идентификаторы
    ht.loadFromFile("constants.txt", 40); // Константы

    int choice;
    do {
        displayMenu();
        choice = HashTable::getValidatedInput("", 1, 10);

        switch (choice) {
        case 1: { // Добавить лексему вручную
            ht.addLexemeManually();
            break;
        }
        case 2: { // Отобразить все таблицы
            ht.display();
            break;
        }
        case 3: { // Поиск лексемы по значению
            std::string value;
            std::cout << "Введите значение лексемы для поиска: ";
            std::cin >> value;
            std::cout << ht.searchInAllTables(value) << "\n";
            break;
        }
        case 4: { // Поиск лексемы по хеш-номеру
            int value;
            std::cout << "Введите значение хеша для поиска лексемы: ";
            std::cin >> value;
            std::cout << ht.findByHash(value) << "\n";
            break;
        }
        case 5: { // Поиск хеш-номера по значению
            std::string value;
            std::cout << "Введите значение лексемы для поиска его хеш-номера: ";
            std::cin >> value;
            int hash = ht.findHashByValue(value);
            if (hash != -1) {
                std::cout << "Хеш-номер: " << hash << "\n";
            }
            else {
                std::cout << "Лексема не найдена.\n";
            }
            break;
        }
        case 6: { // Поиск лексемы в постоянной таблице по номеру
            size_t index;
            std::cout << "Введите номер лексемы в постоянной таблице: ";
            std::cin >> index;
            std::cout << ht.findByConstantTableIndex(index) << "\n";
            break;
        }
        case 7: { // Поиск номера лексемы в постоянной таблице по имени
            std::string value;
            std::cout << "Введите значение лексемы для поиска её номера: ";
            std::cin >> value;
            int index = ht.findConstantTableIndexByValue(value);
            if (index != -1) {
                std::cout << "Номер лексемы в постоянной таблице: " << index << "\n";
            }
            else {
                std::cout << "Лексема не найдена.\n";
            }
            break;
        }
        case 8: { // Добавить атрибут к лексеме
            std::string value, attribute;
            std::cout << "Введите значение лексемы: ";
            std::cin >> value;
            std::cout << "Введите атрибут: ";
            std::cin >> attribute;
            ht.addAttribute(value, attribute);
            break;
        }
        case 9: { // Получить атрибут лексемы
            std::string value;
            std::cout << "Введите значение лексемы: ";
            std::cin >> value;
            std::cout << "Атрибут: " << ht.getAttribute(value) << "\n";
            break;
        }
        case 10: { // Выход
            std::cout << "Выход из программы.\n";
            break;
        }
        default: {
            std::cout << "Неверный выбор. Попробуйте снова.\n";
        }
        }
    } while (choice != 10);

    return 0;
}
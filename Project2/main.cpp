#include <iostream>
#include "Scanner.h"
#include <clocale>
#include "Pars.h"


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
    std::cout << "10. Запустить сканер\n";
    std::cout << "11. Запустить парсер\n";
    std::cout << "12. Выход\n";
    std::cout << "Выберите опцию: ";
}

int main() {
    HashTable ht;
    setlocale(LC_ALL, "");
    Scanner* currentScanner = nullptr;
    Parser* currentParser = nullptr;

    // Загрузка данных из файлов
    ht.loadFromFile("keywords.txt", 10); // Ключевые слова
    ht.loadFromFile("delimiters.txt", 20); // Разделители
    ht.loadFromFile("identifiers.txt", 30); // Идентификаторы
    ht.loadFromFile("constants.txt", 40); // Константы

    int choice;
    do {
        displayMenu();
        choice = HashTable::getValidatedInput("", 1, 12);

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
            int hash, type;
            std::cout << "Введите тип лексемы (30 - идентификатор, 40 - константа): ";
            std::cin >> type;
            std::cout << "Введите значение хеша для поиска лексемы: ";
            std::cin >> hash;
            std::cout << ht.findByHash(hash, type) << "\n";
            break;
        }
        case 5: { // Поиск хеш-номера по значению
            std::string value;
            int type;
            std::cout << "Введите тип лексемы (30 - идентификатор, 40 - константа): ";
            std::cin >> type;
            std::cout << "Введите значение лексемы для поиска его хеш-номера: ";
            std::cin >> value;
            int hash = ht.findHashByValue(value, type);
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
            int type;
            std::cout << "Введите тип лексемы (10 - ключевое слово, 20 - разделитель): ";
            std::cin >> type;
            std::cout << "Введите номер лексемы в постоянной таблице: ";
            std::cin >> index;
            std::cout << ht.findByConstantTableIndex(index, type) << "\n";
            break;
        }
        case 7: { // Поиск номера лексемы в постоянной таблице по имени
            std::string value;
            int type;
            std::cout << "Введите тип лексемы (10 - ключевое слово, 20 - разделитель): ";
            std::cin >> type;
            std::cout << "Введите значение лексемы для поиска её номера: ";
            std::cin >> value;
            int index = ht.findConstantTableIndexByValue(value, type);
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
        case 10: { // Запуск сканера
            std::string filename = "1.txt";
            /*std::cout << "Введите имя файла для сканирования: ";
            std::cin >> filename;*/

            currentScanner = new Scanner(filename, ht);
            if (currentScanner->scan()) {
                std::cout << "Сканирование завершено успешно.\n";
                std::cout << "Токены:\n";
                for (const auto& token : currentScanner->getTokens()) {
                    std::cout << "(" << token.tableType << ", " << token.index << ") "
                        << "Значение: " << token.value
                        << ", Строка: " << token.line << ", Столбец: " << token.column << "\n";
                }
            }
            else {
                std::cout << "Сканирование завершено с ошибками.\n";
                for (const auto& error : currentScanner->getErrors()) {
                    std::cout << error << "\n";
                }
            }
            break;
        }
        case 11: { // Запуск парсера
            if (!currentScanner || currentScanner->getTokens().empty()) {
                std::cout << "Сначала выполните сканирование (пункт 10)\n";
                break;
            }

            // Удаляем предыдущий парсер, если был
            if (currentParser) {
                delete currentParser;
                currentParser = nullptr;
            }

            currentParser = new Parser(currentScanner->getTokens(), ht);
            if (currentParser->parse()) {
                std::cout << "Парсинг завершен успешно!\n";
            }
            else {
                std::cout << "Ошибки парсинга:\n";
                for (const auto& error : currentParser->getErrors()) {
                    std::cout << error << "\n";
                }
            }
            break;
        }
        case 12: { // Выход
            std::cout << "Выход из программы.\n";
            break;
        }
        default: {
            std::cout << "Неверный выбор. Попробуйте снова.\n";
        }
        }
    } while (choice != 12);

    return 0;
}
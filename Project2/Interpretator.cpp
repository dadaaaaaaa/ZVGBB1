#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Interpretator.h"

using namespace std;

PostfixTranslator::PostfixTranslator(HashTable& s) : store(s), executeIf(false), tempRegCount(0), labelCount(0) {}


void PostfixTranslator::translate(const std::string& line) {
    std::stringstream ss(line);
    std::string token, varNameToAssign;
    std::vector<std::string> currentAsmCode;

    // Если строка содержит управляющие конструкции, вызываем соответствующий обработчик
    bool hasControl = line.find("if") != std::string::npos ||
        line.find("while") != std::string::npos ||
        line.find("else") != std::string::npos ||
        line.find("break") != std::string::npos ||
        line.find("continue") != std::string::npos;
    if (hasControl) {
        handleControlStructure(line);
        return;
    }

    // Первый токен – имя переменной для присваивания
    ss >> varNameToAssign;

    // Очистка стеков для нового выражения
    while (!operandStack.empty()) operandStack.pop();
    while (!tempValues.empty()) tempValues.pop();

    // Обработка постфиксного выражения
    while (ss >> token) {
        if (isOperand(token)) {
            operandStack.push(token);
            double value = 0.0;
            try {
                value = std::stod(token);
                // Если константа ещё не добавлена, добавляем в контейнер
                if (numericConstants.find(token) == numericConstants.end()) {
                    numericConstants.insert(token);
                }
                // Формируем команду для числовой константы
                currentAsmCode.push_back("FILD const_" + token + "_0");
            }
            catch (...) {
                // Если токен не является числом, то обрабатываем как переменную
                currentAsmCode.push_back("FILD " + token);
                string attr = store.getAttribute(token);
                value = attr.empty() ? 0.0 : std::stod(attr);
            }
            tempValues.push(value);

        }
        else if (isOperator(token)) {
            // Проверяем, достаточно ли операндов для бинарной операции
            if (operandStack.size() < 2) {
                throw std::runtime_error("Недостаточно операндов для оператора " + token);
            }
            std::string b = operandStack.top();
            operandStack.pop();
            std::string a = operandStack.top();
            operandStack.pop();

            double valB = tempValues.top();
            tempValues.pop();
            double valA = tempValues.top();
            tempValues.pop();

            if (token == "+") {
                currentAsmCode.push_back("FADD ; " + a + " + " + b);
                tempValues.push(valA + valB);
            }
            else if (token == "-") {
                currentAsmCode.push_back("FSUB ; " + a + " - " + b);
                tempValues.push(valA - valB);
            }
            else if (token == "*") {
                currentAsmCode.push_back("FMUL ; " + a + " * " + b);
                tempValues.push(valA * valB);
            }
            else if (token == "/") {
                currentAsmCode.push_back("FDIV ; " + a + " / " + b);
                tempValues.push(valA / valB);
            }
            else if (token == "==" || token == "!=") {
                // Генерация инструкций для сравнения.
                currentAsmCode.push_back("FCOMI cmp ; " + a + " " + token + " " + b);
                double result = (token == "==" ? (valA == valB) : (valA != valB)) ? 1.0 : 0.0;
                tempValues.push(result);
            }
            // Создаем временный регистр для результата операции
            operandStack.push("temp" + std::to_string(tempRegCount++));
        }
        else if (token == "=") {
            double result = tempValues.top();
            tempValues.pop();
            operandStack.pop();

            // Генерация кода присвоения
            currentAsmCode.push_back("FISTP " + varNameToAssign);
            currentAsmCode.push_back("; Присвоение " + varNameToAssign);

            // Сохранение переменной в таблице лексем и дополнительно в контейнере variables
            store.addLexeme(varNameToAssign, 30);
            store.addAttribute(varNameToAssign, std::to_string(result));
            variables.insert(varNameToAssign);

            std::cout << "Значение переменной: " << varNameToAssign
                << " = " << static_cast<int>(result) << std::endl;
        }
    }

    // Добавляем сгенерированный код в общий список кода
    assemblyCode.insert(assemblyCode.end(), currentAsmCode.begin(), currentAsmCode.end());
}
bool PostfixTranslator::isOperand(const string& s) {
    try {
        stod(s);
        return true;
    }
    catch (...) {
        return store.isLexemeExists(s, 30); // Проверяем, существует ли идентификатор
    }
}

bool PostfixTranslator::isOperator(const string& s) {
    return s == "+" || s == "-" || s == "*" || s == "/";
}

int PostfixTranslator::getOperandCount(const string& op) {
    return 2; // Все операции бинарные
}

double PostfixTranslator::generateAssignment(const string& op) {
    string b = operandStack.top(); operandStack.pop();
    string a = operandStack.top(); operandStack.pop();

    double valB = tempValues.top(); tempValues.pop();
    double valA = tempValues.top(); tempValues.pop();

    if (op == "+") return valA + valB;
    else if (op == "-") return valA - valB;
    else if (op == "*") return valA * valB;
    else if (op == "/") return valA / valB;
    return 0; // Для совместимости
}

void PostfixTranslator::handleControlStructure(const string& line) {
    stringstream ss(line);
    string token;

    while (ss >> token) {
        if (token == "if") {
            handleIf(line);
            break;
        }
        else if (token == "while") {
            handleWhile(line);
            break;
        }
        else if (token == "else") {
            handleElse(line);
            break;
        }
        else if (token == "break") {
            handleBreak(line);
            break;
        }
        else if (token == "continue") {
            handleContinue(line);
            break;
        }
    }
}

std::string PostfixTranslator::evaluateCondition(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    std::string condition;

    // Предполагается, что условие начинается с "if" или "while"
    while (iss >> token) {
        if (token == "if" || token == "while") {
            continue; // Пропускаем ключевое слово
        }
        condition += token + " ";
    }

    // Удаляем лишние пробелы и возвращаем условие
    return condition;
}

void PostfixTranslator::handleWhile(const std::string& line) {
    std::string startLabel = "WHILE_START_" + std::to_string(labelCount++);
    std::string endLabel = "WHILE_END_" + std::to_string(labelCount++);
    assemblyCode.push_back(startLabel + ":");

    // Генерация инструкций для вычисления условия
    std::string conditionResult = evaluateCondition(line);
    assemblyCode.push_back("; Вычисление условия while");
    processCondition(conditionResult, endLabel);

    // Перевод тела цикла
    std::string body = line.substr(line.find("while") + 6); // Извлекаем тело цикла
    translate(body); // Перевод тела цикла

    // Безусловный переход на начало цикла
    assemblyCode.push_back("JMP " + startLabel);
    assemblyCode.push_back(endLabel + ":");
}


void PostfixTranslator::handleIf(const std::string& line) {
    std::string trueLabel = "IF_TRUE_" + std::to_string(labelCount++);
    std::string endLabel = "IF_END_" + std::to_string(labelCount++);

    // Извлечение условия
    std::string condition = line.substr(0, line.find("if")); // Условие до "if"
    // Генерация инструкций для вычисления условия
    std::string conditionResult = evaluateCondition(condition);
    assemblyCode.push_back("; Вычисление условия if");
    processCondition(conditionResult, endLabel); // Обработка условия

    // Извлечение тела if
    std::string body = line.substr(line.find("if") + 2); // Извлекаем тело if (после "if ")
    translate(body); // Перевод тела if

    // Добавляем метку для конца if
    assemblyCode.push_back(endLabel + ":");
}

void PostfixTranslator::handleElse(const string& line) {
    // Проверяем, нужно ли выполнять блок else
    if (executeIf) {
        cout << "Пропускаем блок else, так как условие if было истинным" << endl;
        return;
    }

    stringstream ss(line);
    string token;
    ss >> token; // Пропускаем "else"
    string currentExpression;
    while (ss >> token) {
        currentExpression += token + " ";
        if (token == "=") {
            cout << "Выполняется блок else: " << currentExpression << endl;
            translate(currentExpression);
            currentExpression.clear();
        }
    }
}
void PostfixTranslator::processCondition(const std::string& condition, const std::string& jumpLabel) {
    // Здесь вы можете реализовать логику разбора условий
    // Например, если условие содержит "!=" или "==", разбиваем его и генерируем соответствующий код

    // Пример разбора, нужно будет адаптировать под ваши условия
    std::istringstream iss(condition);
    std::string left, op, right;
    iss >> left >> op >> right;
 /*   MOV EAX, [a]; Load a into EAX
        MOV EBX, [b]; Load b into EBX
        CMP EBX, EAX; Compare a and b*/
    // Генерация кода для условия
    if (right == "!=") {
        assemblyCode.push_back("MOV  EAX, [" + left + "] ");
        assemblyCode.push_back("MOV  EAX, [" + op + "] ");
        assemblyCode.push_back("CMP EAX, EBX");

        assemblyCode.push_back("JE " + jumpLabel); // Переход к else, если условие ложно
    }
    else if (right == "==") {
        assemblyCode.push_back("MOV  EAX, [" + left + "] ");
        assemblyCode.push_back("MOV  EAX, [" + op + "] ");
        assemblyCode.push_back("CMP EAX, EBX");
        assemblyCode.push_back("JNE " + jumpLabel); // Переход к else, если условие истинно
    }

    // Обработка логического ИЛИ (||)
    // Это можно реализовать, если вы хотите поддерживать более сложные условия
    // Например, если условие содержит "||", разбиваем его на две части и генерируем код для обеих
}

// Остальные методы остаются без изменений


void PostfixTranslator::handleBreak(const string& line) {
    cout << "Оператор break обнаружен, но должен использоваться внутри цикла while" << endl;
}

void PostfixTranslator::handleContinue(const string& line) {
    cout << "Оператор continue обнаружен, но должен использоваться внутри цикла while" << endl;
}

void PostfixTranslator::printVariables() const {
    cout << "Переменные и их значения:\n";
    for (char c = 'a'; c <= 'z'; ++c) {
        string varName(1, c);
        if (store.isLexemeExists(varName, 30)) { // Проверяем, существует ли идентификатор
            string attr = store.getAttribute(varName);
            if (!attr.empty()) {
                try {
                    int value = static_cast<int>(stod(attr));
                    cout << varName << " = " << value << endl;
                }
                catch (...) {
                    cout << varName << " = (не удалось преобразовать значение: " << attr << ")" << endl;
                }
            }
        }
    }
}

void PostfixTranslator::processFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл: " + filename);
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            try {
                translate(line);
            }
            catch (const exception& e) {
                cerr << "Ошибка обработки строки '" << line << "': " << e.what() << endl;
            }
        }
    }

    file.close();
}

void PostfixTranslator::saveAssemblyToFile(const std::string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw std::runtime_error("Не удалось создать файл: " + filename);
    }
    outFile << ".MODEL FLAT, STDCALL" << std::endl<<
       " OPTION CASEMAP : NONE" << std::endl

        << " EXTERN ExitProcess@4: PROC" << std::endl;
    // Вывод секции данных
    outFile << ".DATA" << std::endl;

    // Вывод объявлений для переменных (например, a dd ?)
    for (const auto& var : variables) {
        // Можно задать начальное значение, например 0
        outFile << var << " dd 0" << std::endl;
    }
    // Вывод числовых констант
    for (const auto& num : numericConstants) {
        outFile << "const_" << num << "_0 dd " << num << std::endl;
    }
    outFile << std::endl;

    // Вывод секции кода
    outFile << ".CODE" << std::endl << "MAIN PROC" << std::endl;
    for (const auto& line : assemblyCode) {
        outFile << line << std::endl;
    }
    outFile << "PUSH 0" << std::endl <<
        "CALL ExitProcess@4" << std::endl <<
        " MAIN ENDP" << std::endl <<
        "END MAIN" << std::endl;
    outFile.close();
}
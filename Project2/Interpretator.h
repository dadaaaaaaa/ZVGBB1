#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <stdexcept>
#include <stack>
#include <unordered_map>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include "HashTable.h"

class PostfixTranslator {
private:
    int i = 1;                            // Указатель стека
    int j = 1;                            // Номер следующей рабочей переменной
    HashTable& store;
    bool executeIf;
    int labelCount; // Инициализируем счетчик меток
    std::stack<std::string> operandStack;
    std::stack<double> tempValues;
    std::vector<std::string> assemblyCode; // Для хранения ассемблерного кода
    int tempRegCount; // Счетчик временных регистров – для управления выполнением if/else
    // Таблица лексем
    void processCondition(const std::string& condition, const std::string& jumpLabel);

    // Контейнер для хранения числовых констант (чтобы они не дублировались)
    std::set<std::string> numericConstants;
    std::unordered_set<std::string> variables;
    bool isOperand(const std::string& token);
    bool isOperator(const std::string& s);
    int getOperandCount(const std::string& op);
    double generateAssignment(const std::string& op);
    void handleControlStructure(const std::string& line);
    std::string evaluateCondition(const std::string& line);
        void handleIf(const std::string& line);
    void handleWhile(const std::string& line);
    void handleElse(const std::string& line);
    void handleBreak(const std::string& line);
    void handleContinue(const std::string& line);
    std::string numToStr(double value);

public:
    void saveAssemblyToFile(const std::string& filename);

    PostfixTranslator(HashTable& s);
    void translate(const std::string& line);
    void printVariables() const;
    void processFile(const std::string& filename);
};

#endif

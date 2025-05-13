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

    // ���� ������ �������� ����������� �����������, �������� ��������������� ����������
    bool hasControl = line.find("if") != std::string::npos ||
        line.find("while") != std::string::npos ||
        line.find("else") != std::string::npos ||
        line.find("break") != std::string::npos ||
        line.find("continue") != std::string::npos;
    if (hasControl) {
        handleControlStructure(line);
        return;
    }

    // ������ ����� � ��� ���������� ��� ������������
    ss >> varNameToAssign;

    // ������� ������ ��� ������ ���������
    while (!operandStack.empty()) operandStack.pop();
    while (!tempValues.empty()) tempValues.pop();

    // ��������� ������������ ���������
    while (ss >> token) {
        if (isOperand(token)) {
            operandStack.push(token);
            double value = 0.0;
            try {
                value = std::stod(token);
                // ���� ��������� ��� �� ���������, ��������� � ���������
                if (numericConstants.find(token) == numericConstants.end()) {
                    numericConstants.insert(token);
                }
                // ��������� ������� ��� �������� ���������
                currentAsmCode.push_back("FILD const_" + token + "_0");
            }
            catch (...) {
                // ���� ����� �� �������� ������, �� ������������ ��� ����������
                currentAsmCode.push_back("FILD " + token);
                string attr = store.getAttribute(token);
                value = attr.empty() ? 0.0 : std::stod(attr);
            }
            tempValues.push(value);

        }
        else if (isOperator(token)) {
            // ���������, ���������� �� ��������� ��� �������� ��������
            if (operandStack.size() < 2) {
                throw std::runtime_error("������������ ��������� ��� ��������� " + token);
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
                // ��������� ���������� ��� ���������.
                currentAsmCode.push_back("FCOMI cmp ; " + a + " " + token + " " + b);
                double result = (token == "==" ? (valA == valB) : (valA != valB)) ? 1.0 : 0.0;
                tempValues.push(result);
            }
            // ������� ��������� ������� ��� ���������� ��������
            operandStack.push("temp" + std::to_string(tempRegCount++));
        }
        else if (token == "=") {
            double result = tempValues.top();
            tempValues.pop();
            operandStack.pop();

            // ��������� ���� ����������
            currentAsmCode.push_back("FISTP " + varNameToAssign);
            currentAsmCode.push_back("; ���������� " + varNameToAssign);

            // ���������� ���������� � ������� ������ � ������������� � ���������� variables
            store.addLexeme(varNameToAssign, 30);
            store.addAttribute(varNameToAssign, std::to_string(result));
            variables.insert(varNameToAssign);

            std::cout << "�������� ����������: " << varNameToAssign
                << " = " << static_cast<int>(result) << std::endl;
        }
    }

    // ��������� ��������������� ��� � ����� ������ ����
    assemblyCode.insert(assemblyCode.end(), currentAsmCode.begin(), currentAsmCode.end());
}
bool PostfixTranslator::isOperand(const string& s) {
    try {
        stod(s);
        return true;
    }
    catch (...) {
        return store.isLexemeExists(s, 30); // ���������, ���������� �� �������������
    }
}

bool PostfixTranslator::isOperator(const string& s) {
    return s == "+" || s == "-" || s == "*" || s == "/";
}

int PostfixTranslator::getOperandCount(const string& op) {
    return 2; // ��� �������� ��������
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
    return 0; // ��� �������������
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

    // ��������������, ��� ������� ���������� � "if" ��� "while"
    while (iss >> token) {
        if (token == "if" || token == "while") {
            continue; // ���������� �������� �����
        }
        condition += token + " ";
    }

    // ������� ������ ������� � ���������� �������
    return condition;
}

void PostfixTranslator::handleWhile(const std::string& line) {
    std::string startLabel = "WHILE_START_" + std::to_string(labelCount++);
    std::string endLabel = "WHILE_END_" + std::to_string(labelCount++);
    assemblyCode.push_back(startLabel + ":");

    // ��������� ���������� ��� ���������� �������
    std::string conditionResult = evaluateCondition(line);
    assemblyCode.push_back("; ���������� ������� while");
    processCondition(conditionResult, endLabel);

    // ������� ���� �����
    std::string body = line.substr(line.find("while") + 6); // ��������� ���� �����
    translate(body); // ������� ���� �����

    // ����������� ������� �� ������ �����
    assemblyCode.push_back("JMP " + startLabel);
    assemblyCode.push_back(endLabel + ":");
}


void PostfixTranslator::handleIf(const std::string& line) {
    std::string trueLabel = "IF_TRUE_" + std::to_string(labelCount++);
    std::string endLabel = "IF_END_" + std::to_string(labelCount++);

    // ���������� �������
    std::string condition = line.substr(0, line.find("if")); // ������� �� "if"
    // ��������� ���������� ��� ���������� �������
    std::string conditionResult = evaluateCondition(condition);
    assemblyCode.push_back("; ���������� ������� if");
    processCondition(conditionResult, endLabel); // ��������� �������

    // ���������� ���� if
    std::string body = line.substr(line.find("if") + 2); // ��������� ���� if (����� "if ")
    translate(body); // ������� ���� if

    // ��������� ����� ��� ����� if
    assemblyCode.push_back(endLabel + ":");
}

void PostfixTranslator::handleElse(const string& line) {
    // ���������, ����� �� ��������� ���� else
    if (executeIf) {
        cout << "���������� ���� else, ��� ��� ������� if ���� ��������" << endl;
        return;
    }

    stringstream ss(line);
    string token;
    ss >> token; // ���������� "else"
    string currentExpression;
    while (ss >> token) {
        currentExpression += token + " ";
        if (token == "=") {
            cout << "����������� ���� else: " << currentExpression << endl;
            translate(currentExpression);
            currentExpression.clear();
        }
    }
}
void PostfixTranslator::processCondition(const std::string& condition, const std::string& jumpLabel) {
    // ����� �� ������ ����������� ������ ������� �������
    // ��������, ���� ������� �������� "!=" ��� "==", ��������� ��� � ���������� ��������������� ���

    // ������ �������, ����� ����� ������������ ��� ���� �������
    std::istringstream iss(condition);
    std::string left, op, right;
    iss >> left >> op >> right;
 /*   MOV EAX, [a]; Load a into EAX
        MOV EBX, [b]; Load b into EBX
        CMP EBX, EAX; Compare a and b*/
    // ��������� ���� ��� �������
    if (right == "!=") {
        assemblyCode.push_back("MOV  EAX, [" + left + "] ");
        assemblyCode.push_back("MOV  EAX, [" + op + "] ");
        assemblyCode.push_back("CMP EAX, EBX");

        assemblyCode.push_back("JE " + jumpLabel); // ������� � else, ���� ������� �����
    }
    else if (right == "==") {
        assemblyCode.push_back("MOV  EAX, [" + left + "] ");
        assemblyCode.push_back("MOV  EAX, [" + op + "] ");
        assemblyCode.push_back("CMP EAX, EBX");
        assemblyCode.push_back("JNE " + jumpLabel); // ������� � else, ���� ������� �������
    }

    // ��������� ����������� ��� (||)
    // ��� ����� �����������, ���� �� ������ ������������ ����� ������� �������
    // ��������, ���� ������� �������� "||", ��������� ��� �� ��� ����� � ���������� ��� ��� �����
}

// ��������� ������ �������� ��� ���������


void PostfixTranslator::handleBreak(const string& line) {
    cout << "�������� break ���������, �� ������ �������������� ������ ����� while" << endl;
}

void PostfixTranslator::handleContinue(const string& line) {
    cout << "�������� continue ���������, �� ������ �������������� ������ ����� while" << endl;
}

void PostfixTranslator::printVariables() const {
    cout << "���������� � �� ��������:\n";
    for (char c = 'a'; c <= 'z'; ++c) {
        string varName(1, c);
        if (store.isLexemeExists(varName, 30)) { // ���������, ���������� �� �������������
            string attr = store.getAttribute(varName);
            if (!attr.empty()) {
                try {
                    int value = static_cast<int>(stod(attr));
                    cout << varName << " = " << value << endl;
                }
                catch (...) {
                    cout << varName << " = (�� ������� ������������� ��������: " << attr << ")" << endl;
                }
            }
        }
    }
}

void PostfixTranslator::processFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("�� ������� ������� ����: " + filename);
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            try {
                translate(line);
            }
            catch (const exception& e) {
                cerr << "������ ��������� ������ '" << line << "': " << e.what() << endl;
            }
        }
    }

    file.close();
}

void PostfixTranslator::saveAssemblyToFile(const std::string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw std::runtime_error("�� ������� ������� ����: " + filename);
    }
    outFile << ".MODEL FLAT, STDCALL" << std::endl<<
       " OPTION CASEMAP : NONE" << std::endl

        << " EXTERN ExitProcess@4: PROC" << std::endl;
    // ����� ������ ������
    outFile << ".DATA" << std::endl;

    // ����� ���������� ��� ���������� (��������, a dd ?)
    for (const auto& var : variables) {
        // ����� ������ ��������� ��������, �������� 0
        outFile << var << " dd 0" << std::endl;
    }
    // ����� �������� ��������
    for (const auto& num : numericConstants) {
        outFile << "const_" << num << "_0 dd " << num << std::endl;
    }
    outFile << std::endl;

    // ����� ������ ����
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
#include "Scanner.h"
#include <cctype>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>

// �����������
Scanner::Scanner(const std::string& filename, HashTable& hashTable)
    : file(filename), hashTable(hashTable), lineNumber(1), columnNumber(0) {
    if (!file.is_open()) {
        errors.push_back("������: �� ������� ������� ���� " + filename);
    }
}

bool Scanner::scan() {
    if (!file.is_open()) {
        return false;
    }

    char ch;
    bool inString = false; // ���� ��� ������������ ��������� ���������
    while ((ch = getNextChar()) != EOF) {
        std::string buffer;
        int startLine = lineNumber;
        int startColumn = columnNumber;

        // ��������� ��������� ���������
        if (ch == '"') {
            inString = !inString; // ����������� ����
            if (!inString) { // ���� ������ �������
                continue;
            }
        }

        // ������� ������������ /* ... */
        if (ch == '/' && peekNextChar() == '*') {
            getNextChar(); // ���������� '*'
            while ((ch = getNextChar()) != EOF) {
                if (ch == '*' && peekNextChar() == '/') {
                    getNextChar(); // ���������� '/'
                    break;
                }
            }
            if (ch == EOF) {
                errors.push_back("������: ����������� /* �� ������.");
            }
            continue;
        }

        // ������� ������������ ������������ //
        if (ch == '/' && peekNextChar() == '/') {
            while ((ch = getNextChar()) != EOF && ch != '\n') {}
            continue;
        }

        // ��������� ������
        if (ch == '{' || ch == '[' || ch == '(' || ch == '"') {
            bracketStack.push(ch);
        }
        else if (ch == '}' || ch == ']' || ch == ')' || ch == '"') {
            if (bracketStack.empty() || !isMatchingBracket(bracketStack.top(), ch)) {
                errors.push_back("������: ������������ ����������� ������ '" + std::string(1, ch) + "' � ������ " + std::to_string(lineNumber));
            }
            else {
                bracketStack.pop();
            }
        }

        // ��������� ������� �� ������� ��� ����� ������
        while (ch != EOF && !std::isspace(ch)) {
            buffer += ch; // ��������� ������ � �����
            ch = getNextChar(); // ��������� ��������� ������
        }

        // ��������� ������� ����� ���������� ���������� �����
        if (!buffer.empty()) {
            Token token;

            // ��������, �������� �� ����� ������������
            if (isDelimiter(buffer)) {
                int index = hashTable.findConstantTableIndexByValue(buffer, TABLE_DELIMITERS);
                token = createToken(buffer, TABLE_DELIMITERS, index);
            }
            // ��������, �������� �� ����� �������� ������
            else if (isKeyword(buffer)) {
                int index = hashTable.findConstantTableIndexByValue(buffer, TABLE_KEYWORDS);
                token = createToken(buffer, TABLE_KEYWORDS, index);
            }
            // ��������, �������� �� ����� ���������������
            else if (isValidIdentifier(buffer)) {
                int index = hashTable.findHashByValue(buffer, TABLE_IDENTIFIERS);
                token = createToken(buffer, TABLE_IDENTIFIERS, index);
            }
            // ��������, �������� �� ����� ����������
            else if (isValidConstant(buffer)) {
                int index = hashTable.findHashByValue(buffer, TABLE_CONSTANTS);
                token = createToken(buffer, TABLE_CONSTANTS, index);
            }
            // ���� ����� �� ���������
            else {
                errors.push_back("������: ����� '" + buffer + "' �� ���������.");
                token = createToken(buffer, -1, -1); // ������
            }

            tokens.push_back(token);
        }
    }

  

    // �������� �� ���������� ������
    while (!bracketStack.empty()) {
        errors.push_back("������: ���������� ������ '" + std::string(1, bracketStack.top()) + "'");
        bracketStack.pop(); // ������� ��� ���������� ������ �� �����
    }

    // ������� ��������� �� ������, ����� ���������
    if (!errors.empty()) {
        saveErrorsToFile("errors.txt");
    }
    else {
        saveTokensToFile("tokens.txt");
    }

    return errors.empty();
}


// ������� ��� ���������� ���������� �����������
int Scanner::levenshteinDistance(const std::string& s1, const std::string& s2) const {
    const std::size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));

    d[0][0] = 0;
    for (std::size_t i = 1; i <= len1; ++i) d[i][0] = i;
    for (std::size_t i = 1; i <= len2; ++i) d[0][i] = i;

    for (std::size_t i = 1; i <= len1; ++i)
        for (std::size_t j = 1; j <= len2; ++j)
            d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) });

    return d[len1][len2];
}

bool Scanner::isKeyword(const std::string& value)  {
    const std::vector<std::string> keywords = { "while", "break", "continue", "if", "else" };

    // ��������� ������ ����������
    if (hashTable.findConstantTableIndexByValue(value, TABLE_KEYWORDS) != -1) {
        return true;
    }

    // ���������� ����� ���������� � ����������� �� ����� ������
    int threshold = (value.length() <= 2) ? 1 : 2;

    // ��������� ��������� ��������
    for (const auto& keyword : keywords) {
        // �������� �� �������� �� ���������� �����������
        if (levenshteinDistance(value, keyword) <= threshold) {
            errors.push_back("������: ��������� �������� � �������� ����� '" + value + "'. ��������, �� ����� � ���� '" + keyword + "'.");
        }

        // �������� �� ���������� ����
        if (hasSameCharacters(value, keyword)) {
            errors.push_back("������: ��������� �������� � �������� ����� '" + value + "'. ��������, �� ����� � ���� '" + keyword + "'.");            return false;
        }
    }

    return false;
}

bool Scanner::hasSameCharacters(const std::string& s1, const std::string& s2) const {
    if (s1.length() != s2.length()) {
        return false;
    }

    std::unordered_map<char, int> charCount;

    for (char c : s1) {
        charCount[c]++;
    }

    for (char c : s2) {
        if (charCount.find(c) == charCount.end() || charCount[c] == 0) {
            return false;
        }
        charCount[c]--;
    }

    return true;
}

// �������� �� ������������ ������
bool Scanner::isMatchingBracket(char open, char close) const {
    return (open == '{' && close == '}') ||
        (open == '[' && close == ']') ||
        (open == '(' && close == ')');
}

// �������� ���������� ������� ��� ��� ����������
char Scanner::peekNextChar() {
    char ch = file.peek();
    return ch;
}


void Scanner::saveErrorsToFile(const std::string& filename) const {
    std::ofstream errorFile(filename);
    if (errorFile.is_open()) {
        for (const auto& error : errors) {
            errorFile << error << std::endl;
        }
        errorFile.close();
    }
    else {
        std::cerr << "������: �� ������� ������� ���� ��� ������ ������.\n";
    }
}

void Scanner::saveTokensToFile(const std::string& filename) const {
    std::ofstream tokenFile(filename);
    if (tokenFile.is_open()) {
        for (const auto& token : tokens) {
            tokenFile << "(" << token.tableType << ", " << token.index << ")" << std::endl;
            if (token.tableType==30)
                hashTable.addLexeme(token.value, 30); // ��������� ������������� � �������
            if (token.tableType == 40)
                hashTable.addLexeme(token.value, 40); // ��������� ��������� � �������

        }
        tokenFile.close();
    }
    else {
        std::cerr << "������: �� ������� ������� ���� ��� ������ �������.\n";
    }
}

// �������� �� ���������� �������������
bool Scanner::isValidIdentifier(const std::string& identifier) const {
    if (identifier.empty() || !std::isalpha(identifier[0])) {
        return false; // ������������� ������ ���������� � �����
    }
    for (char c : identifier) {
        if (!std::isalnum(c) && c != '_') {
            return false; // ��������� ������ �����, ����� � ������ �������������
        }
    }
    return true;
}

// �������� �� ���������� ��������� (����� ��� �����)
bool Scanner::isValidConstant(const std::string& constant) const {
    std::istringstream iss(constant);
    double d;
    char c;
    return (iss >> d) && !(iss >> c); // ���� ������� ������� ����� � ������ ������ ���
}

// ������� �����
Token Scanner::createToken(const std::string& value, int tableType, int index) const {
    Token token;
    token.value = value;
    token.tableType = tableType;
    token.index = index;
    token.line = lineNumber;
    token.column = columnNumber;
    return token;
}

// �������� ��������� ������
char Scanner::getNextChar() {
    char ch = file.get();
    if (ch == '\n') {
        lineNumber++;
        columnNumber = 0;
    }
    else {
        columnNumber++;
    }
    return ch;
}

// ������� ������ ������� � �����
void Scanner::ungetChar() {
    file.unget();
    if (columnNumber > 0) {
        columnNumber--;
    }
}

// ���������, �������� �� ������ ������������
bool Scanner::isDelimiter(const std::string& ch) const {
    return hashTable.findConstantTableIndexByValue(ch, TABLE_DELIMITERS) != -1;
}

// �������� ������
const std::vector<Token>& Scanner::getTokens() const {
    return tokens;
}

// �������� ������
const std::vector<std::string>& Scanner::getErrors() const {
    return errors;
}

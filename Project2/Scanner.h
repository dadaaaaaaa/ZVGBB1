#ifndef SCANNER_H
#define SCANNER_H
#include "HashTable.h"
#include <string>
#include <vector>
#include <fstream>
#include <unordered_set>

// ��������� ��� ����� ������
const int TABLE_DELIMITERS = 10;
const int TABLE_KEYWORDS = 20;
const int TABLE_IDENTIFIERS = 30;
const int TABLE_CONSTANTS = 40;

// ��������� ��� �������� ���������� � ������
struct Token {
    std::string value;
    int tableType;
    int index;
    int line;
    int column;
};

class Scanner {
public:
    // �����������
    Scanner(const std::string& filename, HashTable& hashTable);

    // �������� ����� ������������
    bool scan();

    // ��������, �������� �� ������ �������� ������
    bool isKeyword(const std::string& value);

    // �������� �� ���������� �������������
    bool isValidIdentifier(const std::string& identifier) const;

    // �������� �� ���������� ���������
    bool isValidConstant(const std::string& constant) const;

    // �������� �� �����������
    bool isDelimiter(const std::string& ch) const;

    // ��������� �������
    const std::vector<Token>& getTokens() const;

    // ��������� ������
    const std::vector<std::string>& getErrors() const;

private:
    // ��������������� ������
    char peekNextChar();
    char getNextChar();
    void ungetChar();
    Token createToken(const std::string& value, int tableType, int index) const;
    void saveErrorsToFile(const std::string& filename) const;
    void saveTokensToFile(const std::string& filename) const;

    // ���� ������
    std::ifstream file;
    HashTable& hashTable;
    std::vector<Token> tokens;
    std::vector<std::string> errors;
    int lineNumber;
    int columnNumber;
};

#endif // SCANNER_H
#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <fstream>
#include <stack>
#include "HashTable.h"

// ���� ������
enum TableType {
    TABLE_KEYWORDS = 10,    // �������� �����
    TABLE_DELIMITERS = 20,  // �����������
    TABLE_IDENTIFIERS = 30, // ��������������
    TABLE_CONSTANTS = 40     // ���������
};

// ��������� ������
struct Token {
    int tableType;  // ��� ������� (10, 20, 30, 40)
    int index;      // ����� �������� � �������
    std::string value; // �������� ������ (��� �������)
    int line;       // ����� ������
    int column;     // ����� �������
};

// ����� �������
class Scanner {
public:
    Scanner(const std::string& filename, HashTable& hashTable); // �����������
    bool scan();                          // ������ ������������
    const std::vector<Token>& getTokens() const; // �������� ������
    const std::vector<std::string>& getErrors() const; // �������� ������
    bool isMatchingBracket(char open, char close) const; // �������� ������������ ������
    int levenshteinDistance(const std::string& s1, const std::string& s2) const; // ���������� �����������

private:
    std::ifstream file;                   // ���� ��� ������
    HashTable& hashTable;                 // ������ �� HashTable
    std::vector<Token> tokens;            // ������ �������
    std::vector<std::string> errors;      // ������ ������
    std::stack<char> bracketStack;        // ���� ��� ������������ ������
    int lineNumber;                       // ������� ������
    int columnNumber;                     // ������� �������

    // ��������� ���
    enum State {
        STATE_START,        // ��������� ���������
        STATE_IDENTIFIER,   // �������������
        STATE_CONSTANT,     // ���������
        STATE_KEYWORD,      // �������� �����
        STATE_DELIMITER,    // �����������
        STATE_ERROR         // ������
    };

    bool hasSameCharacters(const std::string& s1, const std::string& s2) const;
    void saveErrorsToFile(const std::string& filename) const; // ��������� ������ � ����
    void saveTokensToFile(const std::string& filename) const; // ��������� ������ � ����
    char peekNextChar(); // ����������� ��������� ������
    bool isValidIdentifier(const std::string& identifier) const; // ��������� �������������
    bool isValidConstant(const std::string& constant) const; // ��������� ���������
    char getNextChar(); // �������� ��������� ������
    void ungetChar(); // ������� ������ ������� � �����
    Token createToken(const std::string& value, int tableType, int index) const; // ������� �����
    bool isKeyword(const std::string& value) ; // ���������, �������� �� ����� �������� ������
    bool isDelimiter(const std::string& ch) const; // ���������, �������� �� ������ ������������
};

#endif // SCANNER_H
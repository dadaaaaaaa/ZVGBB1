#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Lexeme.h" // ��������������, ��� � ��� ���� ����� Lexeme

class HashTable {
public:
    // �����������
    HashTable();

    // ������ ��� ������ � ��������
    void insert(const Lexeme& lexeme);
    void display() const;
    void addLexemeManually();
    void addLexeme(const std::string& value, int type);
    std::string findByHash(int hash, int type) const;
    int findHashByValue(const std::string& value, int type) const;
    void addAttribute(const std::string& value, const std::string& attribute);
    std::string getAttribute(const std::string& value) const;
    std::string searchInAllTables(const std::string& value) const;
    std::string findByConstantTableIndex(size_t index, int type) const;
    int findConstantTableIndexByValue(const std::string& value, int type) const;

    // �������� � ���������� � ����
    void loadFromFile(const std::string& filename, int type);
    void saveToFile(const std::string& filename, int type) const;
    static int getValidatedInput(const std::string& prompt, int min, int max);
private:
    // ��������������� ������
   
    size_t hashFunction(const std::string& value, size_t tableSize) const;
    void rehash(std::vector<std::vector<std::unique_ptr<Lexeme>>>& table);
    void sortTable(std::vector<std::unique_ptr<Lexeme>>& table);
    int determineDataType(const std::string& value) const;
    void ensureFileExists(const std::string& filename) const;
    bool isLexemeExists(const std::string& value, int type) const;

    // �������
    std::vector<std::vector<std::unique_ptr<Lexeme>>> identifierTable; // ������� ���������������
    std::vector<std::vector<std::unique_ptr<Lexeme>>> constantTable;   // ������� ��������
    std::vector<std::unique_ptr<Lexeme>> keywordTable;                // ������� �������� ����
    std::vector<std::unique_ptr<Lexeme>> delimiterTable;              // ������� ������������

    // �������� ��� ������
    std::unordered_map<std::string, std::string> attributes; // �������� ��������� ������
};

#endif // HASHTABLE_H
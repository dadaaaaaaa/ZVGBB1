#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include "Lexeme.h"

class HashTable {
public:
    HashTable(size_t size);
    void insert(const Lexeme& lexeme); // ������� �������
    void display() const;

    void addLexemeManually(); // ���������� ������� �������
    std::string findByHash(int hash) const; // ����� ������� �� ���-������
    int findHashByValue(const std::string& value) const; // ����� ���-������ �� ��������

    // ������ � ����������
    void addAttribute(const std::string& value, const std::string& attribute); // ���������� ��������
    std::string getAttribute(const std::string& value) const; // ��������� ��������

    // ����� ������� �� ���� ��������
    std::string searchInAllTables(const std::string& value) const;

    // ����������� ����� ��� �������� �����
    static int getValidatedInput(const std::string& prompt, int min, int max);

    // ������ ��� ������ � �������
    void loadFromFile(const std::string& filename, int type); // �������� ������ �� �����
    void saveToFile(const std::string& filename, int type) const; // ���������� ������ � ����
    // ����� ������� � ���������� ������� �� ������
    std::string findByConstantTableIndex(size_t index) const;

    // ����� ������ ������� � ���������� ������� �� �����
    int findConstantTableIndexByValue(const std::string& value) const;
private:
    size_t hashFunction(const std::string& value) const;
    size_t findNextFreeIndex(size_t index) const; // ����� ���������� ���������� �������

    void sortConstantTable();
    std::vector<std::unique_ptr<Lexeme>> variableTable; // ���������� ������� (40 ��������� � 30 ��������������)
    std::vector<std::unique_ptr<Lexeme>> constantTable; // ���������� ������� (20 ����������� � 10 �������� ����� )

    std::unordered_map<std::string, std::string> attributes; // �������� ������

    // ����� ��� ��������������� ����������� ���� ������
    int determineDataType(const std::string& value) const;

    // ����� ��� �������� � �������� �����, ���� �� �� ����������
    void ensureFileExists(const std::string& filename) const;

    // ����� ��� �������� �� ���������� �������
    bool isLexemeExists(const std::string& value, int type) const;
};
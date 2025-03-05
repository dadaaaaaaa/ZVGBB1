#include <string>

class Lexeme {
public:
    Lexeme(const std::string& value, int type, int dataType = 0, int isDefined = 0);

    std::string getValue() const;
    int getType() const;
    int getDataType() const;
    int isDefined() const;

private:
    std::string value; // �������� �������
    int type;         // ��� ������� (10 - �������� �����, 20 - �����������, 30 - �������������, 40 - ���������)
    int dataType;     // ��� ������ (0 - �� ����������, 1 - int, 2 - unsigned char)
    int defined;      // �������� (0 - �� ����������, 1 - ����������)
};
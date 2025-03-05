#include <string>

class Lexeme {
public:
    Lexeme(const std::string& value, int type, int dataType = 0, int isDefined = 0);

    std::string getValue() const;
    int getType() const;
    int getDataType() const;
    int isDefined() const;

private:
    std::string value; // Значение лексемы
    int type;         // Тип лексемы (10 - ключевое слово, 20 - разделитель, 30 - идентификатор, 40 - константа)
    int dataType;     // Тип данных (0 - не определено, 1 - int, 2 - unsigned char)
    int defined;      // Значение (0 - не определено, 1 - определено)
};
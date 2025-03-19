#ifndef LEXEM_H
#define LEXEM_H

#include <string>

class Lexeme {
public:
    Lexeme(const std::string& value, int type, int dataType, int isDefined);

    std::string getValue() const;
    int getType() const;
    int getDataType() const;
    int isDefined() const;

private:
    std::string value;
    int type;
    int dataType;
    int defined;
};

#endif // LEXEM_H
#include "Lexeme.h"

Lexeme::Lexeme(const std::string& value, int type, int dataType, int isDefined)
    : value(value), type(type), dataType(dataType), defined(isDefined) {
}

std::string Lexeme::getValue() const {
    return value;
}

int Lexeme::getType() const {
    return type;
}

int Lexeme::getDataType() const {
    return dataType;
}

int Lexeme::isDefined() const {
    return defined;
}
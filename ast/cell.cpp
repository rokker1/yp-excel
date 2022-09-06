#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(){}

Cell::~Cell() {}

void Cell::Set(std::string text) {}

void Cell::Clear() {}

Cell::Value Cell::GetValue() const {}
std::string Cell::GetText() const {}

class Cell::Impl {
public:

private:

};

class Cell::EmptyImpl : public Cell::Impl {

};

class Cell::TextImpl : public Cell::Impl {

};

class Cell::FormulaImpl : public Cell::Impl {

};
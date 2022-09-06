#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell()
    : impl_(std::make_unique<Impl>())
{}

Cell::~Cell() {
    Clear();
}

void Cell::Set(std::string text) {
    if(text.empty()) {
        Clear();
    }
    if(text[0] == FORMULA_SIGN) {
        text = text.substr(1);
        if(text.empty()) {
            // создать ячейку с одним символом =
            Clear();
        } else {
            //  это формула - спарсить формулу
            Clear();
            std::unique_ptr<FormulaInterface> formula = ParseFormula(text);
            impl_.reset(formula);
        }
    } else if (text[0] == ESCAPE_SIGN) {
        text = text.substr(1);
        Clear();
    } else {
        // это текст
        Clear();
    }
}

void Cell::Clear() {
    impl_.reset();
}

Cell::Value Cell::GetValue() const {}
std::string Cell::GetText() const {}

class Cell::Impl {
public:
    Impl();
    ~Impl();

    virtual void Cell::Set(std::string text) = 0;
    virtual Cell::Value Cell::GetValue() const = 0;
    virtual std::string Cell::GetText() const = 0;

private:
    std::variant<std::monostate, std::string, std::unique_ptr<FormulaInterface>> impl_;
};

class Cell::EmptyImpl : public Cell::Impl {

};

class Cell::TextImpl : public Cell::Impl {
    
};

class Cell::FormulaImpl : public Cell::Impl {
    
};
#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell() {}

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
            impl_ = std::make_unique<TextImpl>("="s);
        } else {
            //  это формула - спарсить формулу
            std::unique_ptr<FormulaInterface> formula;
            try {
                formula = ParseFormula(text);
            } catch (FormulaException& e) {
                return;
            }

            Clear();
            impl_ = std::make_unique<FormulaImpl>(std::move(formula));


            try {
                impl_.get()->GetValue();
            } catch (FormulaError& error) {
                ;
            }
        }
    } else if (text[0] == ESCAPE_SIGN) {
        text = text.substr(1);
        Clear();
        impl_ = std::make_unique<TextImpl>(std::move(text));
    } else {
        // это текст
        Clear();
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    impl_.reset();
}

Cell::Value Cell::GetValue() const {

}
std::string Cell::GetText() const {

}

class Cell::Impl {
public:
    virtual Cell::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
};

class Cell::EmptyImpl : public Cell::Impl {
public:
    Cell::Value GetValue() const override {
        return {};
    }
    std::string Cell::GetText() const override {
        return {};
    }
};

class Cell::TextImpl : public Cell::Impl {
public:
    explicit FormulaImpl(std::string text)
        : text(std::move(text))

    Cell::Value GetValue() const override {
        return text_;
    }
    std::string Cell::GetText() const override {
        return text_;
    }
private:
    std::string text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    explicit FormulaImpl(std::unique_ptr<FormulaInterface> formula)
        : formula_(std::move())

    Cell::Value GetValue() const override {
        return formula_.get()->Evaluate();
    }

    std::string Cell::GetText() const override {
        return formula_.get()->GetExpression();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
};
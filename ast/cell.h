#pragma once

#include "common.h"
#include "formula.h"


class Cell : public CellInterface {
public:

    Cell() 
        : impl_(std::make_unique<EmptyImpl>())
    {

    }

    Cell(std::string text) {
        Set(text);
    }

    void Set(std::string text) {
        if(text.empty()) {
            Clear();
        }
        // добавляется возможность индексов ячеек
        if(text[0] == FORMULA_SIGN) {
            if(text.size() == 1) {
                // создать ячейку с одним символом =
                impl_ = std::make_unique<TextImpl>("=");
            } else {
                //  это формула - спарсить формулу
                std::unique_ptr<FormulaInterface> formula = ParseFormula(text.substr(1));
                impl_ = std::make_unique<FormulaImpl>(std::move(formula));
            }
        } else if (text[0] == ESCAPE_SIGN) {
            impl_ = std::make_unique<TextImpl>(std::move(text));
        } else {
            // это текст
            impl_ = std::make_unique<TextImpl>(std::move(text));
        }
    }

    // Возвращает видимое значение ячейки.
    // В случае текстовой ячейки это её текст (без экранирующих символов). В
    // случае формулы - числовое значение формулы или сообщение об ошибке.
    Value GetValue() const override {
        return impl_.get()->GetValue();
    }

    // Возвращает внутренний текст ячейки, как если бы мы начали её
    // редактирование. В случае текстовой ячейки это её текст (возможно,
    // содержащий экранирующие символы). В случае формулы - её выражение.
    std::string GetText() const override {
        return impl_.get()->GetText();
    }

    void Clear() {
        impl_ = std::make_unique<EmptyImpl>();
    }

private:

    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
        Value GetValue() const override {
            return {};
        }
        std::string GetText() const override {
            return {};
        }
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text)
            : text_(text) {}

        Value GetValue() const override {
            if(!text_.empty() && text_.at(0) == ESCAPE_SIGN) {
                return text_.substr(1);
            }
            return text_;
        }
        std::string GetText() const override {
            return text_;
        }

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::unique_ptr<FormulaInterface> formula)
            : formula_(std::move(formula)) {}

        Value GetValue() const override {
            FormulaInterface::Value res = formula_.get()->Evaluate();
            if (std::holds_alternative<double>(res)) {
                return std::get<double>(res);
            } else if (std::holds_alternative<FormulaError>(res)) {
                return std::get<FormulaError>(res);
            } else {
                throw std::runtime_error("e");
            }
        }

        std::string GetText() const override {
            std::string res = "=" + formula_.get()->GetExpression();
            return res;
        }
        
    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

    std::unique_ptr<Impl> impl_;
};
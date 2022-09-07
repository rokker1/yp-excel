#pragma once

#include "common.h"
#include "formula.h"


class Cell : public CellInterface {
public:

    Cell() 
        : impl_(std::make_unique<EmptyImpl>())
    {

    }


    void Set(std::string text) override {
        ; // заглушка
    }

    // Возвращает видимое значение ячейки.
    // В случае текстовой ячейки это её текст (без экранирующих символов). В
    // случае формулы - числовое значение формулы или сообщение об ошибке.
    Value GetValue() const override {
        return {};
    }

    // Возвращает внутренний текст ячейки, как если бы мы начали её
    // редактирование. В случае текстовой ячейки это её текст (возможно,
    // содержащий экранирующие символы). В случае формулы - её выражение.
    std::string GetText() const override {
        return {};
    }

private:

    class Impl {
        virtual Value GetValue() const = 0;
        virtual Value GetText() const = 0;
    };

    class EmptyImpl : public Impl {
        Value GetValue() const override {
            return {};
        }
        Value GetText() const override {
            return {};
        }
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text)
            : text_(text) {}

        Value GetValue() const override {
            return text_;
        }
        Value GetText() const override {
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
            }
        }

        Value GetText() const override {
            std::string res = formula_.get()->GetExpression();
            return res;
        }
        
    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

    std::unique_ptr<Impl> impl_;
};
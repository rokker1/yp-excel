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

    std::unique_ptr<Impl> impl_;
};
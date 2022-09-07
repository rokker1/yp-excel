#pragma once

#include "common.h"
#include "formula.h"


class Cell : public CellInterface {
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
};
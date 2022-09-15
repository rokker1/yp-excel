#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
        Cell(Sheet& sheet);
        ~Cell();

        void Set(std::string text);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool IsReferenced() const;

private:
        class Impl;
        class EmptyImpl;
        class TextImpl;
        class FormulaImpl;
        Sheet& sheet_;
        std::unique_ptr<Impl> impl_;

        // Добавьте поля и методы для связи с таблицей, проверки циклических 
        // зависимостей, графа зависимостей и т. д.
    std::vector<Position> referenced_cells_;
    std::vector<Position> dependent_cells_;
    
};
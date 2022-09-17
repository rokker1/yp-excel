#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    Cell(SheetInterface& sheet, std::string text);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    void AddDependentCell(Position dependent_cell);
    virtual std::set<Position> GetDependentCells() const;

private:

    class Impl {
    public:
        Impl(SheetInterface& sheet)
            : sheet_(sheet) {}
        
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        SheetInterface& GetSheet() const {
            return sheet_;
        }
        virtual const std::vector<Position> GetReferencedCells() const = 0;

        void AddDependentCell(Position dependent_cell) {
            dependent_cells_.insert(dependent_cell);
        }
        std::set<Position> GetDependentCells() const {
            return dependent_cells_;
        }

    protected:    
        SheetInterface& sheet_;
        std::optional<double> cached_value_;
        std::set<Position> dependent_cells_;
    };
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
    // Добавьте поля и методы для связи с таблицей, проверки циклических 
    // зависимостей, графа зависимостей и т. д.

};

class Cell::EmptyImpl : public Impl {
public:
    EmptyImpl(SheetInterface& sheet)
        : Impl(sheet) {}

    Value GetValue() const override {
        return 0.0;
    }
    std::string GetText() const override {
        return {};
    }
    virtual const std::vector<Position> GetReferencedCells() const override {
        return {};
    }
};

class Cell::TextImpl : public Impl {
public:
    explicit TextImpl(SheetInterface& sheet, std::string text)
        : Impl(sheet)
        , text_(text) {}

    Value GetValue() const override {
        if(!text_.empty() && text_.at(0) == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }
    std::string GetText() const override {
        return text_;
    }
    virtual const std::vector<Position> GetReferencedCells() const override {
        return {};
    }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(SheetInterface& sheet, std::unique_ptr<FormulaInterface> formula)
        : Impl(sheet)
        , formula_(std::move(formula))
        // , referenced_cells_(formula_.get()->GetReferencedCells())
    {}

    Value GetValue() const override {
        FormulaInterface::Value res = formula_.get()->Evaluate(sheet_);
        if (std::holds_alternative<double>(res)) {
            return std::get<double>(res);
        } else if (std::holds_alternative<FormulaError>(res)) {
            return std::get<FormulaError>(res);
        } else {
            throw std::runtime_error("e");
        }
        return {};
    }

    std::string GetText() const override {
        std::string res = "=" + formula_.get()->GetExpression();
        return res;
    }
    virtual const std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

//    virtual void AddDependentCell(Position dependent_cell) override {
//        dependent_cells_.insert(dependent_cell);
//    }
//    std::set<Position> GetDependentCells() const {
//        return dependent_cells_;
//    }
private:
    std::unique_ptr<FormulaInterface> formula_;
    // список ячеек, которые ссылкаются на данную ячейку

};
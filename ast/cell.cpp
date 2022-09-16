#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>


Cell::Cell(SheetInterface& sheet) 
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>(sheet))
{}

Cell::Cell(SheetInterface& sheet, std::string text) 
    : sheet_(sheet) {
    Set(text);
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if(text.empty()) {
        Clear();
        return;
    }
    // добавляется возможность индексов ячеек
    if(text[0] == FORMULA_SIGN) {
        if(text.size() == 1) {
            // создать ячейку с одним символом =
            impl_ = std::make_unique<TextImpl>(sheet_, "=");
        } else {
            //  это формула - спарсить формулу
            std::unique_ptr<FormulaInterface> formula = ParseFormula(text.substr(1));
            formula->GetReferencedCells();
            impl_ = std::make_unique<FormulaImpl>(sheet_, std::move(formula));
        }
    } else if (text[0] == ESCAPE_SIGN) {
        impl_ = std::make_unique<TextImpl>(sheet_, std::move(text));
    } else {
        // это текст или число!
        double number = 0.0;
        size_t processed = 0;
        try {
            number = std::stod(text, &processed);
        } catch (const std::invalid_argument& e) {
            impl_ = std::make_unique<TextImpl>(sheet_, std::move(text));
            return;
        } catch (const std::out_of_range& e) {
            throw;
        }

        if(text.substr(processed).empty()) {
            // текст ячейки состоял только из цифр и они все прочитаны, текста в ячейке не осталось
            impl_ = std::make_unique<FormulaImpl>(sheet_, ParseFormula(std::move(text)));
        } else {
            // после цифр что-то было
            impl_ = std::make_unique<TextImpl>(sheet_, std::move(text));
        }
    }
}

    void Cell::Clear() {
        impl_ = std::make_unique<EmptyImpl>(sheet_);
    }

    CellInterface::Value Cell::GetValue() const {
        return impl_->GetValue();
    }
    std::string Cell::GetText() const {
        return impl_->GetText();
    }

    std::vector<Position> Cell::GetReferencedCells() const {
        return std::vector<Position>{referenced_cells_.begin(), referenced_cells_.end()};
    }
    // class Cell::Impl {
    // public:
    //     Impl(SheetInterface& sheet)
    //         : sheet_(sheet) {}
        
    //     virtual Value GetValue() const = 0;
    //     virtual std::string GetText() const = 0;
    //     SheetInterface& GetSheet() const {
    //         return sheet_;
    //     }

    //     // virtual const std::set<Position> GetReferencedCells() const = 0;

    // protected:    
    //     SheetInterface& sheet_;

    //     std::optional<double> cached_value_;
    // };

    // class Cell::EmptyImpl : public Impl {
    // public:
    //     EmptyImpl(SheetInterface& sheet)
    //         : Impl(sheet) {}

    //     Value GetValue() const override {
    //         return {};
    //     }
    //     std::string GetText() const override {
    //         return {};
    //     }
    //     // const std::set<Position> GetReferencedCells() const override {
    //     //     return {};
    //     // }
    // };

    // class Cell::TextImpl : public Impl {
    // public:
    //     explicit TextImpl(SheetInterface& sheet, std::string text)
    //         : Impl(sheet)
    //         , text_(text) {}

    //     Value GetValue() const override {
    //         if(!text_.empty() && text_.at(0) == ESCAPE_SIGN) {
    //             return text_.substr(1);
    //         }
    //         return text_;
    //     }
    //     std::string GetText() const override {
    //         return text_;
    //     }

    // private:
    //     std::string text_;
    // };

    // class Cell::FormulaImpl : public Impl {
    // public:
    //     explicit FormulaImpl(SheetInterface& sheet, std::unique_ptr<FormulaInterface> formula)
    //         : Impl(sheet)
    //         , formula_(std::move(formula))
    //         // , referenced_cells_(formula_.get()->GetReferencedCells())
    //     {}

    //     Value GetValue() const override {
    //         FormulaInterface::Value res = formula_.get()->Evaluate(sheet_);
    //         if (std::holds_alternative<double>(res)) {
    //             return std::get<double>(res);
    //         } else if (std::holds_alternative<FormulaError>(res)) {
    //             return std::get<FormulaError>(res);
    //         } else {
    //             throw std::runtime_error("e");
    //         }
    //         return {};
    //     }

    //     std::string GetText() const override {
    //         std::string res = "=" + formula_.get()->GetExpression();
    //         return res;
    //     }


        
    // private:
    //     std::unique_ptr<FormulaInterface> formula_;
    //     std::set<Position> referenced_cells_;
    // };
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
            std::unique_ptr<FormulaInterface> formula;
            try {
                formula = ParseFormula(text.substr(1));

            } catch (...) {
                throw FormulaException("bad formula");
            }

//          formula->GetReferencedCells();
            impl_ = std::make_unique<FormulaImpl>(sheet_, std::move(formula));
        }
    } else if (text[0] == ESCAPE_SIGN) {
        impl_ = std::make_unique<TextImpl>(sheet_, std::move(text));
    } else {
        // это текст или число!
        [[maybe_unused]] double number = 0.0;
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
        try {
            return impl_->GetValue();
        } catch (const FormulaError& e) {
            return e;
        }
    }
    std::string Cell::GetText() const {
        return impl_->GetText();
    }

    std::vector<Position> Cell::GetReferencedCells() const {
        return impl_->GetReferencedCells();
    }

    void Cell::AddDependentCell(Position dependent_cell) {
        impl_->AddDependentCell(dependent_cell);
    }
    std::set<Position> Cell::GetDependentCells() const {
        return impl_->GetDependentCells();
    }

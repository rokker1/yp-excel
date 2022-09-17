#pragma once

#include "cell.h"
#include "common.h"
#include <map>
#include <memory>
#include <vector>
#include <iostream>

#include <functional>

class Sheet : public SheetInterface {
public:
    Sheet() {
        
    }
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами
    void AddDependentCell(Position referenced_cell, Position dependent_cell);

private:
    // ячейки
    std::vector<std::vector<std::unique_ptr<CellInterface>>> sheet_;
    // максимальный размер таблицы по столбцам
    size_t max_x_ = 0;
    // максимальный размер таблицы по строкам
    size_t max_y_ = 0;

    // граф
    // http://shujkova.ru/sites/default/files/algorithm2.pdf
    // https://habr.com/ru/post/504374/
    
    
    struct CellValuePrinter {
        CellValuePrinter(std::ostream& output)
            : output(output) {}

        std::ostream& operator()(std::string value) {
            return output << value;
        }
        std::ostream& operator()(double value) {
            return output << value;
        }
        std::ostream& operator()(FormulaError& error) {
            return output << error.ToString();
        }

        std::ostream& output;
    };
};
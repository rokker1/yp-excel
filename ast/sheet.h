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
    // метод, когда надо создать пустую ячейку, на которую кто-то ссылается
    // эта ячейка не попадет в печатную зону.
    void SetEmptyReferencedCellNotInPrintableArea(Position referenced_cell);
    bool HasDependentCells(Position pos) const;
private:
    // ячейки
    std::vector<std::vector<std::unique_ptr<CellInterface>>> sheet_;
    // скольк печатных элементов в каждой строке
    std::vector<size_t> rows_printable_size_; // rps
    // // максимальный размер таблицы по столбцам (печатная область)
    // size_t max_x_ = 0;
    // // максимальный размер таблицы по строкам (печатная область)
    // size_t max_y_ = 0;

    // размер максимальной строки в печатной области
    size_t longest_row_ = 0;

    // размер печатной области
    Size print_area_ = {0, 0};

    // граф
    // http://shujkova.ru/sites/default/files/algorithm2.pdf
    // https://habr.com/ru/post/504374/
    void CheckCycles(const std::vector<Position>& ref_cells, Position begin);
    // возвращает истону, если ячейка хранит не пустой текст
    bool IsNonEmptyCell(Position position) const;
    bool IsNonEmptyCell(const std::unique_ptr<CellInterface>& cell) const;

    
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
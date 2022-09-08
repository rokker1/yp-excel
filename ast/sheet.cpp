#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    }

    const size_t y = pos.row;
    const size_t x = pos.col;

    if(sheet_.size() <= x) {
        sheet_.resize(x + 1);
        if (max_x_ < x + 1) {
            max_x_ = x + 1;
        }
    }

    if(sheet_.at(x).size() <= y) {
        sheet_.at(x).resize(y + 1);
        if (max_y_ < y + 1) {
            max_y_ = y + 1;
        }
    }
    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>();
    cell.get()->Set(text);
    
    sheet_[x][y] = std::move(cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    ;
}

Size Sheet::GetPrintableSize() const {
    return {0, 0};
}

void Sheet::PrintValues(std::ostream& output) const {
    return;
}
void Sheet::PrintTexts(std::ostream& output) const {
    return;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
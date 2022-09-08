#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

#include <fstream>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    }

    const size_t y = pos.row;
    const size_t x = pos.col;

    if(sheet_.size() <= y) {
        sheet_.resize(y + 1);
        if (max_y_ < y + 1) {
            max_y_ = y + 1;
        }
    }

    if(sheet_.at(y).size() <= x) {
        sheet_.at(y).resize(x + 1);
        if (max_x_ < x + 1) {
            max_x_ = x + 1;
        }
    }
    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>();
    cell.get()->Set(text);
    
    sheet_[y][x] = std::move(cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    }
    const size_t y = pos.row;
    const size_t x = pos.col;

    if(sheet_.size() <= y) {
        return nullptr;
    }
    if(sheet_.at(y).size() <= x) {
        return nullptr;
    }
    return sheet_.at(y).at(x).get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    }
    const size_t y = pos.row;
    const size_t x = pos.col;
    if(sheet_.size() <= y) {
        sheet_.resize(y + 1);
        if (max_y_ < y + 1) {
            max_y_ = y + 1;
        }
    }

    if(sheet_.at(y).size() <= x) {
        sheet_.at(y).resize(x + 1);
        if (max_x_ < x + 1) {
            max_x_ = x + 1;
        }
    }
    return sheet_.at(y).at(x).get();
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    };
    const size_t y = pos.row;
    const size_t x = pos.col;
    if(sheet_.size() <= y) {
        return;
    }
    if(sheet_.at(y).size() <= x) {
        return;
    }
    sheet_[y][x].reset();
    // поменять минимальную печатную область!
}

Size Sheet::GetPrintableSize() const {
    return {(int)max_y_, (int)max_x_};
}

void Sheet::PrintValues(std::ostream& output) const {
    if(!sheet_.empty()) {
        
        for (size_t j = 0; j < max_y_; ++j) {
            bool is_first_col = true;
            for (size_t i = 0; i < sheet_.at(j).size(); ++i) {
                if(!is_first_col) {
                    output << '\t';
                }
                is_first_col = false;
                if(auto cell = sheet_.at(j).at(i).get(); cell != nullptr) {
                    auto value = cell->GetValue();
                    std::visit(CellValuePrinter{output}, value);
                } else {
                    output << "";
                }

                if(max_x_ > sheet_.at(j).size()) {
                    for(size_t i = (max_x_ - sheet_.at(j).size()); i < max_x_; ++i) {
                        output << '\t';
                    }
                }
            }
            output << '\n';
        }
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    if(!sheet_.empty()) {
        
        for (size_t j = 0; j < max_y_; ++j) {
            bool is_first_col = true;
            for (size_t i = 0; i < sheet_.at(j).size(); ++i) {
                if(!is_first_col) {
                    output << '\t';
                }
                is_first_col = false;
                if(auto cell = sheet_.at(j).at(i).get(); cell != nullptr) {
                    output << cell->GetText();
                } else {
                    output << "";
                }

                if(max_x_ > sheet_.at(j).size()) {
                    for(size_t i = (max_x_ - sheet_.at(j).size()); i < max_x_; ++i) {
                        output << '\t';
                    }
                }
            }
            output << '\n';
        }
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
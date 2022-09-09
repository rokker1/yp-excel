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

    if(y >= sheet_.size()) {
        // строки с таким индексом нет
        sheet_.resize(y + 1);
        max_y_ = y + 1;

        sheet_.at(y).resize(x + 1);
        if(x >= max_x_) {
            max_x_ = x + 1;
        }
    } else {
        // строка с таким индексом есть
        if(x >= sheet_.at(y).size()) {
            sheet_.at(y).resize(x + 1);
            if(x >= max_x_) {
                max_x_ = x + 1;
            }
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

    if(y >= sheet_.size()) {
        return nullptr;
    }

    if(x >= sheet_.at(y).size()) {
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

    if(y >= sheet_.size()) {
        SetCell(pos, "");
        sheet_.at(y).at(x) = nullptr;
        return sheet_.at(y).at(x).get();
    }
    if(x >= sheet_.at(y).size()) {
        SetCell(pos, "");
        sheet_.at(y).at(x) = nullptr;
        return sheet_.at(y).at(x).get();
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

    if(sheet_.at(y).size() == x + 1) {
        // просиходит удаление последнего элемента строки
        bool longest_row = false;
        if (sheet_.at(y).size() == max_x_) {
            longest_row = true;
            // данная строка таблицы является (одной из) самых длинных
            // необходим пересчет max_x_
        }
        // удаляем последний элемент в строке
        sheet_.at(y).erase(prev(sheet_.at(y).end()));
        // ищем первый не нулевой
        [[maybe_unused]] auto reverse_it = std::find_if(sheet_.at(y).rbegin(), 
                                                        sheet_.at(y).rend(),
                [](const std::unique_ptr<CellInterface>& it){
                    return it != nullptr;
                });
        // сколько пустых элементов в конце надо удалить
        size_t to_be_remove = std::distance(sheet_.at(y).rbegin(), reverse_it);
        sheet_.at(y).resize(sheet_.at(y).size() - to_be_remove);

        if(y + 1 == sheet_.size()) {
            // удаляемая ячейка находится в последней строке
            if (sheet_.at(y).size() == 0 && sheet_.size() != 0) {
                // последняя строка пуста
                sheet_.resize(sheet_.size() - 1);
                max_y_ = sheet_.size();
            }
        }
        // здесь нужна логика поиска последней ненулевой строки
        [[maybe_unused]] auto last_non_empty_row_it = std::find_if(sheet_.rbegin(), 
                                                        sheet_.rend(),
                [](const auto& row){
                    return !row.empty();
                });
        // сколько пустых элементов в конце надо удалить
        to_be_remove = std::distance(sheet_.rbegin(), last_non_empty_row_it);
        if(to_be_remove != 0) {
            sheet_.resize(sheet_.size() - to_be_remove);
            max_y_ = sheet_.size();
        }


        if(longest_row) {
            size_t new_max_x = 0;
            std::for_each(sheet_.begin(), sheet_.end(),
                [&new_max_x](const auto& v) {
                    if(new_max_x < v.size()) {
                        new_max_x = v.size();
                    }
                });
            max_x_ = new_max_x;
        }
    } else {
        sheet_.at(y).at(x) = nullptr;
    }



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
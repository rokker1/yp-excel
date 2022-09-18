#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
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
        if (!text.empty()) {
            max_y_ = y + 1;
        }


        sheet_.at(y).resize(x + 1);
        if(!text.empty() && x >= max_x_) {
            max_x_ = x + 1;
        }
    } else {
        // строка с таким индексом есть
        if(x >= sheet_.at(y).size()) {
            sheet_.at(y).resize(x + 1);
            if(!text.empty() && x >= max_x_) {
                max_x_ = x + 1;
            }
        }
    }

    // создается текущая ячейка
    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>(*this, text);

    // если на текущий момент в ячейке [y, x] уже была ячейка, и там были записаны зависимые ячейки,
    // их нужно перенести во вновь созданную ячейку, чтобы сохранить
    if(GetCell(pos) != nullptr) {
        Cell* c = dynamic_cast<Cell*>(GetCell(pos));
        if(!c) {
            throw std::runtime_error("Not a cell!");
        }
        if(!c->GetDependentCells().empty()) {
            Cell* cell_ptr = dynamic_cast<Cell*>(cell.get());
            if(!cell_ptr) {
                throw std::runtime_error("Not a cell!");
            }
            cell_ptr->SetDependentCells(std::move(c->GetDependentCells()));
        }
    }

    // Выполним проверку на циклы
    const auto& ref_cells = cell->GetReferencedCells();
    CheckCycles(ref_cells, pos);

    // записать себя в список зависимых ячеек для всех ячеек,
    // которые указаны в моем перечне  referenced_cells_

    for(Position referenced_cell : cell->GetReferencedCells()) {
        if(GetCell(referenced_cell) == nullptr) {
            // мы ссылаемся на пустую ячейку - создадим ее пустым текстом
            // здесь пытается прокрасться ошибка! Мы найдем её!
            SetCell(referenced_cell, "");
            SetEmptyReferencedCellNotInPrintableArea(referenced_cell);
        }
        AddDependentCell(referenced_cell, Position{static_cast<int>(y), static_cast<int>(x)});
    }

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
    /*
     * !!! это крайне неправильно что гетселл меняет печатную область !!!
     * печастная область должна быть сама по себе как независимый параметр
     * возможно надо завести еще один параметр - максимальный размер строки в листе
     */
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    }
    const size_t y = pos.row;
    const size_t x = pos.col;

    if(y >= sheet_.size()) {
//        SetCell(pos, "");
//        sheet_.at(y).at(x) = nullptr;
//        return sheet_.at(y).at(x).get();
        return nullptr;
    }
    if(x >= sheet_.at(y).size()) {

//        SetCell(pos, "");
//        sheet_.at(y).at(x) = nullptr;
//        return sheet_.at(y).at(x).get();
        return nullptr;
    }

    return sheet_.at(y).at(x).get();
}

void Sheet::ClearCell(Position pos) {
    /*
     * теперь нельзя просто взять и сделать ресайз
     * если ячейка имеет зависимые ячейки, то
     * будет меняться только размер печатной области
     */
    if(!pos.IsValid()) {
        throw InvalidPositionException("invalid position!");
    };

    bool has_dependent_cells = false;
    if(CellInterface* c = GetCell(pos)) {
        Cell* cell_ptr = dynamic_cast<Cell*>(c);
        if(!cell_ptr) {
            throw std::runtime_error("not a cell!!");
        }
        if(!cell_ptr->GetDependentCells().empty()) {
            has_dependent_cells = true;
        }
    }
    
    const size_t y = pos.row;
    const size_t x = pos.col;
    if(sheet_.size() <= y) {
        return;
    }
    if(sheet_.at(y).size() <= x) {
        return;
    }


    if(has_dependent_cells) {
        // зависимые ячейки есть
        if(sheet_.at(y).size() == x + 1) {
            // просиходит удаление последнего элемента строки
            bool longest_row = false;
            if (sheet_.at(y).size() == max_x_) {
                longest_row = true;
                // данная строка таблицы является (одной из) самых длинных
                // необходим пересчет max_x_
            }
            // ячейка становится пустой, зависимые сохраняются
            SetCell(pos, "");
            // ищем первый не нулевой
            [[maybe_unused]] auto reverse_it = std::find_if(next(sheet_.at(y).rbegin()), // next
                                                    sheet_.at(y).rend(),
            [](const std::unique_ptr<CellInterface>& it){
                return it != nullptr;
            });
            size_t to_be_remove = std::distance(sheet_.at(y).rbegin(), reverse_it);



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
        }

    } else {
        // зависимых ячеек нет
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
            sheet_.at(y).at(x) = nullptr; // wrong
        }

    }


}

Size Sheet::GetPrintableSize() const {
    return {(int)max_y_, (int)max_x_};
}

void Sheet::PrintValues(std::ostream& output) const {
    for(const auto& row : sheet_) {
        bool is_first_col = true;
        for(const auto& cell : row) {
            if(!is_first_col) {
                output << '\t';
            }
            is_first_col = false;

            if(cell.get() != nullptr) {
                auto value = cell->GetValue();
                std::visit(CellValuePrinter{output}, value);
            } else {
                output << "";
            }
        }

        if(row.empty()) {
            // выведи max_x_ - 1 табуляций
            for (size_t i = 0; i + 1 < max_x_; ++i) {
                output << '\t';
            }
        } else {
            // выведи max_x_ - size() табуляций
            for (size_t i = 0; i < (max_x_ - row.size()); ++i) {
                output << '\t';
            }
        }

        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for(const auto& row : sheet_) {
        bool is_first_col = true;
        for(const auto& cell : row) {
            if(!is_first_col) {
                output << '\t';
            }
            is_first_col = false;

            if(cell.get() != nullptr) {
                output << cell->GetText();

            } else {
                output << "";
            }
        }

        if(row.empty()) {
            // выведи max_x_ - 1 табуляций
            for (size_t i = 0; i + 1 < max_x_; ++i) {
                output << '\t';
            }
        } else {
            // выведи max_x_ - size() табуляций
            for (size_t i = 0; i < (max_x_ - row.size()); ++i) {
                output << '\t';
            }
        }

        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::AddDependentCell(Position referenced_cell, Position dependent_cell) {
    CellInterface* ref_cell = GetCell(referenced_cell);
    if(!ref_cell) {
        SetCell(referenced_cell, "");
    }
    Cell* c = dynamic_cast<Cell*>(ref_cell);
    if(c) {
        c->AddDependentCell(dependent_cell);
    } else {
        throw std::runtime_error("wtf");
    }
}

void Sheet::CheckCycles(const std::vector<Position>& ref_cells, Position begin) {

    for(const Position ref_cell : ref_cells) {
        if (begin == ref_cell) {
            throw CircularDependencyException("cycle catched");
        }
    }

    CellInterface* begin_cell = GetCell(begin);
    if(begin_cell) {
        Cell* begin_cell_ptr = dynamic_cast<Cell*>(begin_cell);
        if(!begin_cell_ptr) {
            throw  std::runtime_error("not a cell!");
        }
        for(const auto dep_cell : begin_cell_ptr->GetDependentCells()) {
            CheckCycles(ref_cells, dep_cell);
        }
    }
}

void Sheet::SetEmptyReferencedCellNotInPrintableArea(Position referenced_cell) {
    if(!referenced_cell.IsValid()) {
            throw InvalidPositionException("invalid position!");
        }
        const size_t y = referenced_cell.row;
        const size_t x = referenced_cell.col;

        if(y >= sheet_.size()) {
            // строки с таким индексом нет
            sheet_.resize(y + 1);
            // max_y_ = y + 1;

            sheet_.at(y).resize(x + 1);
            // if(x >= max_x_) {
            //     max_x_ = x + 1;
            // }
        } else {
            // строка с таким индексом есть
            if(x >= sheet_.at(y).size()) {
                sheet_.at(y).resize(x + 1);
                // if(x >= max_x_) {
                //     max_x_ = x + 1;
                // }
            }
        }

    // создается пустая текущая ячейка
    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>(*this);
    sheet_[y][x] = std::move(cell);
}

bool Sheet::NonEmptyCell(Position position) const {
    const CellInterface* cell = GetCell(position);
    if(!cell) {
        return false;
    } else {
        Cell::Value v = cell->GetValue();
        std::ostringstream os;
        std::visit(CellValuePrinter{os}, v);
        return !os.str().empty();
    }
}
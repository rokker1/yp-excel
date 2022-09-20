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

    // создается текущая ячейка
    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>(*this, text);
    bool is_printable = IsNonEmptyCell(cell);
    // если ячейка печатная, она поменяет размер печатной области
    // если ячейка непечатная она не поменяет размер печатной области

    if(y >= sheet_.size()) {
        // строки с таким индексом нет
        sheet_.resize(y + 1);
        rows_printable_size_.resize(y + 1);
        if (is_printable) {
            print_area_.rows = y + 1;
            last_printable_row = y + 1;
        }
        sheet_.at(y).resize(x + 1);
        if(is_printable && x >= static_cast<size_t>(print_area_.cols)) {
            print_area_.cols = x + 1;
            rows_printable_size_[y] = x + 1;
        }
    } else {
        // строка с таким индексом есть
        if(x >= sheet_.at(y).size()) {
            sheet_.at(y).resize(x + 1);
            if(is_printable && x >= static_cast<size_t>(print_area_.cols)) {
                print_area_.cols = x + 1;
                rows_printable_size_[y] = x + 1;
            }
        }

        if(is_printable && x >= rows_printable_size_[y]) {
            print_area_.cols = x + 1;
            rows_printable_size_[y] = x + 1;
        }
    }

    // здесь для старой ячейки нужно пройтись по зависимым ячейкам и удалить ссылку на это ячейку
    if(GetCell(pos) != nullptr) {
        Cell* previous_cell = dynamic_cast<Cell*>(GetCell(pos));
        for(Position referenced_cell : previous_cell->GetReferencedCells()) {
            RemoveDependentCell(referenced_cell, pos);
        }
    }


    // если на текущий момент в ячейке [y, x] уже была ячейка, и там были записаны зависимые ячейки,
    // их нужно перенести во вновь созданную ячейку, чтобы сохранить
    if(GetCell(pos) != nullptr) {
        Cell* previous_cell = dynamic_cast<Cell*>(GetCell(pos));
        if(!previous_cell) {
            throw std::runtime_error("Not a cell!");
        }
        if(!previous_cell->GetDependentCells().empty()) {
            Cell* new_cell = dynamic_cast<Cell*>(cell.get());
            if(!new_cell) {
                throw std::runtime_error("Not a cell!");
            }
            new_cell->SetDependentCells(std::move(previous_cell->GetDependentCells()));
        }
    }

    // Выполним проверку на циклы
    const auto& ref_cells = cell->GetReferencedCells();
    CheckCycles(ref_cells, pos);

    // записать себя в список зависимых ячеек для всех ячеек,
    // которые указаны в моем перечне referenced_cells_
    for(Position referenced_cell : cell->GetReferencedCells()) {
        if(GetCell(referenced_cell) == nullptr) {
            // мы ссылаемся на пустую ячейку - создадим ее пустым текстом
            // здесь пытается прокрасться ошибка! Мы найдем её!
            SetCell(referenced_cell, "");
            SetEmptyReferencedCellNotInPrintableArea(referenced_cell);
        }
        AddDependentCell(referenced_cell, pos);
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

    bool has_dependent_cells = HasDependentCells(pos);
    /*
    от того, имеет ли ячейка зависимые - зависит дальнейшее поведение.
    Если не имеет - мы запишем вместо нее nullptr и уменьшим размеры векторов.
    Если имеет - мы запишем пустой текст, и уменьшим печатную область.
    */
    
    const size_t y = pos.row;
    const size_t x = pos.col;
    if(sheet_.size() <= y) {
        return;
    }
    if(sheet_.at(y).size() <= x) {
        return;
    }

    bool is_printable = IsNonEmptyCell(pos);

    if(has_dependent_cells) {
        // зависимые ячейки есть
        
        if (!is_printable) {
            // ячейка становится пустой, зависимые сохраняются
            SetCell(pos, "");
        } else {
            // ячейка становится пустой, зависимые сохраняются
            SetCell(pos, "");

            // ячейка печатная
            // нужен пересчет printable area
            if(sheet_.at(y).size() == x + 1) {

                bool longest_row = false;
                if (x + 1 == static_cast<size_t>(print_area_.cols)) {
                    longest_row = true;
                    // данная строка таблицы является (одной из) самых длинных
                    // необходим пересчет max_x_
                }
                // ищем первый не нулевой
                [[maybe_unused]] auto reverse_it = std::find_if(sheet_.at(y).rbegin(), 
                                                                sheet_.at(y).rend(),
                        [this](const std::unique_ptr<CellInterface>& it){
                            return IsNonEmptyCell(it);
                        });
                // сколько пустых элементов в конце надо удалить
                size_t to_be_remove = std::distance(sheet_.at(y).rbegin(), reverse_it);
                rows_printable_size_[y] -= to_be_remove;

                if(y + 1 == static_cast<size_t>(print_area_.rows)) {
                    // это была последняя строка в печтатной области
                    if(rows_printable_size_[y] == 0 && print_area_.rows != 0) {
                        print_area_.rows--;

                        // здесь нужна логика поиска последней печатной строки
                        size_t last_printable_row = y;
                        while (rows_printable_size_[last_printable_row] == 0
                                && last_printable_row > 0) {
                            --last_printable_row;
                        }
                        print_area_.rows = last_printable_row + 1;
                    }
                }

                if(longest_row) {
                    size_t new_max_x = 0;
                    std::for_each(rows_printable_size_.begin(),
                                    rows_printable_size_.end(),
                                    [&](const size_t n){
                                        if (n > new_max_x) {
                                            new_max_x = n;
                                        }
                                    });
                    print_area_.rows = new_max_x;
                }
            }
        }

    } else {
        // зависимых ячеек нет
        if(sheet_.at(y).size() == x + 1) {
            // просиходит удаление последнего элемента строки
            bool longest_row = false;
            if (sheet_.at(y).size() == static_cast<size_t>(print_area_.cols)) {
                longest_row = true;
                // данная строка таблицы является (одной из) самых длинных
                // необходим пересчет max_x_
            }
            // удаляем последний элемент в строке
            sheet_.at(y).erase(prev(sheet_.at(y).end()));
            if (is_printable) {
                rows_printable_size_[y]--;
            }
            // ищем первый не нулевой
            [[maybe_unused]] auto reverse_it = std::find_if(sheet_.at(y).rbegin(), 
                                                            sheet_.at(y).rend(),
                    [this](const std::unique_ptr<CellInterface>& it){
                        return IsNonEmptyCell(it);
                    });
            // сколько пустых элементов в конце надо удалить
            size_t to_be_remove = std::distance(sheet_.at(y).rbegin(), reverse_it);
            sheet_.at(y).resize(sheet_.at(y).size() - to_be_remove);
            rows_printable_size_[y] -= to_be_remove;

            
            if(y + 1 == sheet_.size()) {
                // удаляемая ячейка находится в последней строке
                if (sheet_.at(y).size() == 0 && sheet_.size() != 0) {
                    // последняя строка пуста
                    sheet_.resize(sheet_.size() - 1);
                    rows_printable_size_.resize(sheet_.size());
                    print_area_.rows = sheet_.size();
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
                print_area_.rows = sheet_.size();
            }
            // уменьшение числа строк в печатной области
            size_t last_printable_row = rows_printable_size_.size() - 1;
            while (rows_printable_size_[last_printable_row] == 0
                    && last_printable_row > 0) {
                --last_printable_row;
            }
            print_area_.rows = last_printable_row + 1;


            if(longest_row) {
                size_t new_max_x = 0;
                std::for_each(rows_printable_size_.begin(),
                rows_printable_size_.end(),
                [&](const size_t n){
                    if (n > new_max_x) {
                        new_max_x = n;
                    }
                });
                print_area_.cols = new_max_x;
            }
        } else {
            sheet_.at(y).at(x) = nullptr; // wrong
        }

    }
}

Size Sheet::GetPrintableSize() const {
    return print_area_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for(int row_index = 0; row_index < print_area_.rows; ++row_index) {
        bool is_first_col = true;

        for(int x = 0; x < print_area_.cols; ++x) {
            if(!is_first_col) {
                output << '\t';
            }
            is_first_col = false;
            auto cell = GetCell({row_index, x});
            if(cell != nullptr) {
                auto value = cell->GetValue();
                output << "{";
                std::visit(CellValuePrinter{output}, value);
                output << "}";
            } else {
                output << "{}";
            }
        }

        // if(row.empty()) {
        //     // выведи max_x_ - 1 табуляций
        //     for (size_t i = 0; i + 1 < static_cast<size_t>(print_area_.cols); ++i) {
        //         output << '\t';
        //     }
        // } else {
        //     // выведи max_x_ - size() табуляций
        //     for (size_t i = 0; i < (print_area_.cols - row.size()); ++i) {
        //         output << '\t';
        //     }
        // }

        output << std::endl;
    }
    output << std::endl;
}
void Sheet::PrintTexts(std::ostream& output) const {
    for(int row_index = 0; row_index < print_area_.rows; ++row_index) {
        bool is_first_col = true;

        for(int x = 0; x < print_area_.cols; ++x) {
            if(!is_first_col) {
                output << '\t';
            }
            is_first_col = false;
            auto cell = GetCell({row_index, x});
            if(cell != nullptr) {
                auto value = cell->GetText();
                output << '{' << value << '}';
            } else {
                output << "{}";
            }
        }

        // if(row.empty()) {
        //     // выведи max_x_ - 1 табуляций
        //     for (size_t i = 0; i + 1 < static_cast<size_t>(print_area_.cols); ++i) {
        //         output << '\t';
        //     }
        // } else {
        //     // выведи max_x_ - size() табуляций
        //     for (size_t i = 0; i < (print_area_.cols - row.size()); ++i) {
        //         output << '\t';
        //     }
        // }

        output << '\n';
    }
    output << '\n';
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
    sheet_[y][x] = std::make_unique<Cell>(*this);
}

bool Sheet::IsNonEmptyCell(Position position) const {
    const CellInterface* cell = GetCell(position);
    if(!cell) {
        // nullptr вернет false
        return false;
    } else {
        return !cell->GetText().empty();
        Cell::Value v = cell->GetValue();
        std::ostringstream os;
        std::visit(CellValuePrinter{os}, v);
        return !os.str().empty();
    }
}
bool Sheet::IsNonEmptyCell(const std::unique_ptr<CellInterface>& cell) const {
    if(!cell) {
        // nullptr вернет false
        return false;
    } else {
        /*
        Cell::Value v = cell->GetValue();
        std::ostringstream os;
        std::visit(CellValuePrinter{os}, v);
        std::cout << os.str() << std::endl; // debug
        return !os.str().empty();
        */
       return !cell->GetText().empty();
    }
}

bool Sheet::HasDependentCells(Position pos) const {
    bool has_dependent_cells = false;
    if(const CellInterface* c = GetCell(pos)) {
        const Cell* cell_ptr = dynamic_cast<const Cell*>(c);
        if(!cell_ptr) {
            throw std::runtime_error("not a cell!!");
        }
        if(!cell_ptr->GetDependentCells().empty()) {
            has_dependent_cells = true;
        }
    }
    return has_dependent_cells;
}


void Sheet::RemoveDependentCell(Position referenced_cell, Position dependent_cell) {
    CellInterface* ref_cell = GetCell(referenced_cell);
    Cell* c = dynamic_cast<Cell*>(ref_cell);
    if(c) {
        c->RemoveDependentCell(dependent_cell);
    } else {
        throw std::runtime_error("wtf");
    }
}
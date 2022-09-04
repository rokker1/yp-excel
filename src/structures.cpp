#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return (rhs.col == col && rhs.row == row);
}

bool Position::operator<(const Position rhs) const {
    return 
        (row < rhs.row && col <= rhs.col) ||
        (row <= rhs.row && col < rhs.col);
}

bool Position::IsValid() const {
    if(col < 0 || row < 0 || col >= MAX_COLS || row >= MAX_ROWS) {
        return false;
    } else {
        return true;
    }
}

std::string Position::ToString() const {
    std::string position;
    if(!IsValid()) {
        return position;
    }
   
    // букву в младшем разряде нужно обработать перед циклом, отдельно
    // прибавить к ней единицу
    int integer = col; // целое
    int remander = integer % LETTERS; // остаток
    position += static_cast<char>(remander + 'A');


    integer /= LETTERS;
    if(integer != 0) {
        for(; integer > LETTERS; integer /= LETTERS) {
            remander = integer % LETTERS;
            position += static_cast<char>(remander + 'A' - 1);
        }
        position += static_cast<char>(integer + 'A' - 1);
    }



    std::reverse(position.begin(), position.end());

    position += std::to_string(row + 1);
    return position;
}

int ColumnIndexFromString(std::string_view column) {
    int column_index = 0;
    int multiplier = 1;

    for(auto it = column.rbegin(); it != column.rend(); ++it) {
        column_index += (((*it) - 'A' + 1) * multiplier);
        multiplier *= LETTERS;
        
    }

    return column_index == 0 ? 0 : --column_index;
}

Position Position::FromString(std::string_view str) {
    Position position;
    std::string column; // A
    std::string row; // 1

    auto it = str.begin();
    while(std::isalpha(*it) && std::isupper(*it) && it != str.end()) {
        column += *it;
        ++it;
    }
    while(std::isdigit(*it) && it != str.end()) {
        row += *it;
        ++it;
    }
    if(it != str.end()) {
        column.clear();
        row.clear();
    }
    
    if(column.empty() || row.empty()) {
        return Position::NONE;
    }

    if(!(row.length() > 5)) {
        position.row = std::stoi(row) - 1;
    } else {
        return Position::NONE;
    }

    if(column.length() <= MAX_POS_LETTER_COUNT) {
        position.col = ColumnIndexFromString(column);
    } else {
        return Position::NONE;
    }


    if(!position.IsValid()) {
        return Position::NONE;
    }

    return position;
}
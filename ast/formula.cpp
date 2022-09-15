#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    // Реализуйте следующие методы:
    explicit Formula(std::string expression) 
            : ast_(ParseFormulaAST(expression))
    {}

    Value Evaluate(const SheetInterface& sheet) const override {
        // рекурсивный вызов Evaluate(sheet) для дочерних ячеек
        double result = 0.0;
        try {
            result = ast_.Execute(sheet);
        } catch (FormulaError& e) {
            return e;
        }
        return result;
    }

    std::string GetExpression() const override {
        std::ostringstream s;
        ast_.PrintFormula(s);
        return s.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        [[maybe_unused]] const std::forward_list<Position>& fl =
                                                         ast_.GetCells();
        
        // referenced_cells_ = std::move(std::vector<Position>{fl.begin(), fl.end()});
        // return referenced_cells_;

        // что тут надо делать?
        return {};
    }

private:
    FormulaAST ast_;
    std::vector<Position> referenced_cells_;
};
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

FormulaError::FormulaError(Category category)
    : category_(category) {}


FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_)
    {
    case Category::Ref:
        return "#REF!";
        break;
    case Category::Value:
        return "#VALUE!";
        break;
    case Category::Div0:
        return "#DIV/0!";
        break;
    
    default:
        break;
        
    }
    throw std::runtime_error("Unknown category!");
}
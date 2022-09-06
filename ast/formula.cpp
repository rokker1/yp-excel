#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
    // Реализуйте следующие методы:
    explicit Formula(std::string expression) 
            : ast_(ParseFormulaAST(expression))
    {}

    Value Evaluate() const override {
        double result = 0.0;
        try {
            result = ast_.Execute();
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
private:
    FormulaAST ast_;
};
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
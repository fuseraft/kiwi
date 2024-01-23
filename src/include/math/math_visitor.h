#ifndef MATH_VISITOR_H
#define MATH_VISITOR_H

#include <variant>
#include <cmath>
#include "../errors/error.h"
#include "../parsing/tokens.h"
#include "../typing/value_type.h"

struct {
    bool is_zero(ValueType type, std::variant<int, double, bool, std::string> v) {
        if (type == ValueType::Double) {
            return std::get<double>(v) == 0.0;
        }
        else if (type == ValueType::Integer) {
            return std::get<int>(v) == 0;
        }

        return false;
    }

    std::variant<int, double, bool, std::string> do_addition(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;

        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) + std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) + std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) + std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) + static_cast<double>(std::get<int>(right));
        }
        else if (vtleft == ValueType::String) {
            std::ostringstream build;
            build << std::get<std::string>(left);

            if (vtright == ValueType::Integer) {
                build << std::get<int>(right);
            }
            else if (vtright == ValueType::Double) {
                build << std::get<double>(right);
            }
            else if (vtright == ValueType::Boolean) {
                build << std::boolalpha << std::get<bool>(right);
            }
            else if (vtright == ValueType::String) {
                build << std::get<std::string>(right);
            }

            result = build.str();
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_subtraction(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;

        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) - std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) - std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) - std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) - static_cast<double>(std::get<int>(right));
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_exponentiation(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;

        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = static_cast<int>(pow(std::get<int>(left), std::get<int>(right)));
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = pow(std::get<double>(left), std::get<double>(right));
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = pow(static_cast<double>(std::get<int>(left)), std::get<double>(right));
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = pow(std::get<double>(left), static_cast<double>(std::get<int>(right)));
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_modulus(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;

        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            int rhs = std::get<int>(right);
            if (rhs == 0) {
                throw DivideByZeroError(token);
            }
            result = std::get<int>(left) % std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            double rhs = std::get<double>(right);
            if (rhs == 0.0) {
                throw DivideByZeroError(token);
            }
            result = fmod(std::get<double>(left), rhs);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            double rhs = std::get<double>(right);
            if (rhs == 0.0) {
                throw DivideByZeroError(token);
            }
            result = fmod(std::get<int>(left), rhs);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            double rhs = static_cast<double>(std::get<int>(right));
            if (rhs == 0) {
                throw DivideByZeroError(token);
            }
            result = fmod(std::get<double>(left), rhs);
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_division(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;

        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            int rhs = std::get<int>(right);
            if (rhs == 0) {
                throw DivideByZeroError(token);
            }
            result = std::get<int>(left) / rhs;
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            double rhs = std::get<double>(right);
            if (rhs == 0.0) {
                throw DivideByZeroError(token);
            }
            result = std::get<double>(left) / rhs;
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            double rhs = std::get<double>(right);
            if (rhs == 0.0) {
                throw DivideByZeroError(token);
            }
            result = static_cast<double>(std::get<int>(left)) / rhs;
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            double rhs = static_cast<double>(std::get<int>(right));
            if (rhs == 0.0) {
                throw DivideByZeroError(token);
            }
            result = std::get<double>(left) / rhs;
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_multiplication(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) * std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) * std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) * std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) * static_cast<double>(std::get<int>(right));
        }
        else if (vtleft == ValueType::String && vtright == ValueType::Integer) {
            std::string string = std::get<std::string>(left);
            int multiplier = std::get<int>(right);
            
            if (multiplier < 1) {
                throw SyntaxError(token, "String multiplier must be a positive non-zero integer.");
            }
            
            std::ostringstream build;
            build << string;
            
            for (int i = 1; i < multiplier; ++i) {
                build << string;
            }

            result = build.str();
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_eq_comparison(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) == std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) == std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) == std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) == static_cast<double>(std::get<int>(right));
        }
        else if (vtleft == ValueType::String && vtright == ValueType::String) {
            result = std::get<std::string>(left) == std::get<std::string>(right);
        }
        else if (vtleft == ValueType::Boolean && vtright == ValueType::Boolean) {
            result = std::get<bool>(left) == std::get<bool>(right);
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_neq_comparison(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) != std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) != std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) != std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) != static_cast<double>(std::get<int>(right));
        }
        else if (vtleft == ValueType::String && vtright == ValueType::String) {
            result = std::get<std::string>(left) != std::get<std::string>(right);
        }
        else if (vtleft == ValueType::Boolean && vtright == ValueType::Boolean) {
            result = std::get<bool>(left) != std::get<bool>(right);
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_lt_comparison(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) < std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) < std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) < std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) < static_cast<double>(std::get<int>(right));
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_lte_comparison(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) <= std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) <= std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) <= std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) <= static_cast<double>(std::get<int>(right));
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_gt_comparison(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) > std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) > std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) > std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) > static_cast<double>(std::get<int>(right));
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }

    std::variant<int, double, bool, std::string> do_gte_comparison(const Token& token, ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
            result = std::get<int>(left) >= std::get<int>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
            result = std::get<double>(left) >= std::get<double>(right);
        }
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
            result = static_cast<double>(std::get<int>(left)) >= std::get<double>(right);
        }
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
            result = std::get<double>(left) >= static_cast<double>(std::get<int>(right));
        }
        else {
            throw ConversionError(token);
        }

        return result;
    }
} MathImpl;

struct AddVisitor {
    const Token& token;

    AddVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_addition(token, vtleft, vtright, left, right);
    }
};

struct SubtractVisitor {
    const Token& token;

    SubtractVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        if (vtright == ValueType::Integer || vtright == ValueType::Double) {
            return MathImpl.do_subtraction(token, vtleft, vtright, left, right);
        }
        else {
            throw ConversionError(token);
        }
    }
};

struct MultiplyVisitor {
    const Token& token;

    MultiplyVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_multiplication(token, vtleft, vtright, left, right);
    }
};

struct DivideVisitor {
    const Token& token;

    DivideVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);

        if (vtright == ValueType::Integer || vtright == ValueType::Double) {
            if (MathImpl.is_zero(vtright, right)) {
                throw DivideByZeroError(token);
            }
            else {
                return MathImpl.do_division(token, vtleft, vtright, left, right);
            }
        } 
        else {
            throw ConversionError(token);
        }
    }
};

struct PowerVisitor {
    const Token& token;

    PowerVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        if (vtright == ValueType::Integer || vtright == ValueType::Double) {
            return MathImpl.do_exponentiation(token, vtleft, vtright, left, right);
        }
        else {
            throw ConversionError(token);
        }
    }
};

struct ModuloVisitor {
    const Token& token;

    ModuloVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        if (vtright == ValueType::Integer || vtright == ValueType::Double) {
            return MathImpl.do_modulus(token, vtleft, vtright, left, right);
        }
        else {
            throw ConversionError(token);
        }
    }
};

struct EqualityVisitor {
    const Token& token;

    EqualityVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_eq_comparison(token, vtleft, vtright, left, right);
    }
};

struct InequalityVisitor {
    const Token& token;

    InequalityVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_neq_comparison(token, vtleft, vtright, left, right);
    }
};

struct LessThanVisitor {
    const Token& token;

    LessThanVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_lt_comparison(token, vtleft, vtright, left, right);
    }
};

struct LessThanOrEqualVisitor {
    const Token& token;

    LessThanOrEqualVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_lte_comparison(token, vtleft, vtright, left, right);
    }
};

struct GreaterThanVisitor {
    const Token& token;

    GreaterThanVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_gt_comparison(token, vtleft, vtright, left, right);
    }
};

struct GreaterThanOrEqualVisitor {
    const Token& token;

    GreaterThanOrEqualVisitor(const Token& token) : token(token) {}

    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        
        return MathImpl.do_gte_comparison(token, vtleft, vtright, left, right);
    }
};

#endif
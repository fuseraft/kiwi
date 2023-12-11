#ifndef INTERP_VISITOR_H
#define INTERP_VISITOR_H

#include <variant>
#include <cmath>
#include "value_type.h"

struct {
    bool is_zero(ValueType type, std::variant<int, double, bool, std::string> v) {
        if (type == ValueType::Double)
            return std::get<double>(v) == 0.0;
        else if (type == ValueType::Integer)
            return std::get<int>(v) == 0;
        return false;
    }

    std::variant<int, double, bool, std::string> do_addition(ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer)
            result = std::get<int>(left) + std::get<int>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Double)
            result = std::get<double>(left) + std::get<double>(right);
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double)
            result = static_cast<double>(std::get<int>(left)) + std::get<double>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer)
            result = std::get<double>(left) + static_cast<double>(std::get<double>(right));
        else if (vtleft == ValueType::String) {
            std::ostringstream build;
            if (vtright == ValueType::Integer)
                build << std::get<int>(right);
            else if (vtright == ValueType::Double)
                build << std::get<double>(right);
            else if (vtright == ValueType::Boolean)
                build << std::boolalpha << std::get<bool>(right);
            else if (vtright == ValueType::String)
                build << std::get<std::string>(right);
            result = build.str();
        }
        else
            throw std::runtime_error("Conversion error.");
        return result;
    }

    std::variant<int, double, bool, std::string> do_subtraction(ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer)
            result = std::get<int>(left) - std::get<int>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Double)
            result = std::get<double>(left) - std::get<double>(right);
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double)
            result = static_cast<double>(std::get<int>(left)) - std::get<double>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer)
            result = std::get<double>(left) - static_cast<double>(std::get<double>(right));
        else
            throw std::runtime_error("Conversion error.");
        return result;
    }

    std::variant<int, double, bool, std::string> do_exponentiation(ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer)
            result = static_cast<int>(pow(std::get<int>(left), std::get<int>(right)));
        else if (vtleft == ValueType::Double && vtright == ValueType::Double)
            result = pow(std::get<double>(left), std::get<double>(right));
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double)
            result = static_cast<int>(pow(static_cast<double>(std::get<int>(left)), std::get<double>(right)));
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer)
            result = pow(std::get<double>(left), static_cast<double>(std::get<double>(right)));
        else
            throw std::runtime_error("Conversion error.");
        return result;
    }

    std::variant<int, double, bool, std::string> do_division(ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer)
            result = std::get<int>(left) / std::get<int>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Double)
            result = std::get<double>(left) / std::get<double>(right);
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double)
            result = static_cast<double>(std::get<int>(left)) / std::get<double>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer)
            result = std::get<double>(left) / static_cast<double>(std::get<double>(right));
        else
            throw std::runtime_error("Conversion error.");
        return result;
    }

    std::variant<int, double, bool, std::string> do_multiplication(ValueType vtleft, ValueType vtright, std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) {
        std::variant<int, double, bool, std::string> result;
        if (vtleft == ValueType::Integer && vtright == ValueType::Integer)
            result = std::get<int>(left) * std::get<int>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Double)
            result = std::get<double>(left) * std::get<double>(right);
        else if (vtleft == ValueType::Integer && vtright == ValueType::Double)
            result = static_cast<double>(std::get<int>(left)) * std::get<double>(right);
        else if (vtleft == ValueType::Double && vtright == ValueType::Integer)
            result = std::get<double>(left) * static_cast<double>(std::get<double>(right));
        else if (vtleft == ValueType::String && vtright == ValueType::Integer)
        {
            std::string string = std::get<std::string>(left);
            int multiplier = std::get<int>(right);
            if (multiplier < 1)
                throw std::runtime_error("String multiplier must be a positive non-zero integer.");
            std::ostringstream build;
            build << string;
            for (int i = 1; i < multiplier; ++i)
                build << string;
            result = build.str();
        }
        else
            throw std::runtime_error("Conversion error.");
        return result;
    }
} MathImpl;

struct AddVisitor {
    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        return MathImpl.do_addition(vtleft, vtright, left, right);
    }
};

struct SubtractVisitor {
    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        if (vtright == ValueType::Integer || vtright == ValueType::Double)
            return MathImpl.do_subtraction(vtleft, vtright, left, right);
        else
            throw std::runtime_error("Conversion error.");
    }
};

struct MultiplyVisitor {
    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        return MathImpl.do_multiplication(vtleft, vtright, left, right);
    }
};

struct DivideVisitor {
    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        if (vtright == ValueType::Integer || vtright == ValueType::Double) {
            if (MathImpl.is_zero(vtright, right))
                throw std::runtime_error("Division by zero.");
            else
                return MathImpl.do_division(vtleft, vtright, left, right);
        } else
            throw std::runtime_error("Conversion error.");
    }
};

struct PowerVisitor {
    std::variant<int, double, bool, std::string> operator()(std::variant<int, double, bool, std::string> left, std::variant<int, double, bool, std::string> right) const {
        ValueType vtleft = get_value_type(left);
        ValueType vtright = get_value_type(right);
        if (vtright == ValueType::Integer || vtright == ValueType::Double)
            return MathImpl.do_exponentiation(vtleft, vtright, left, right);
        else
            throw std::runtime_error("Conversion error.");
    }
};

#endif
#ifndef BOOL_EXPR_H
#define BOOL_EXPR_H

#include <iostream>
#include <memory>
#include <functional>

class ExpressionNode {
 public:
  virtual bool evaluate() const = 0;
  virtual ~ExpressionNode() = default;
};

class ValueNode : public ExpressionNode {
  bool value;

 public:
  ValueNode(bool val) : value(val) {}
  bool evaluate() const override { return value; }
};

class AndNode : public ExpressionNode {
  std::shared_ptr<ExpressionNode> left, right;

 public:
  AndNode(std::shared_ptr<ExpressionNode> left,
          std::shared_ptr<ExpressionNode> right)
      : left(left), right(right) {}

  bool evaluate() const override {
    return left->evaluate() && right->evaluate();
  }
};

class OrNode : public ExpressionNode {
  std::shared_ptr<ExpressionNode> left, right;

 public:
  OrNode(std::shared_ptr<ExpressionNode> left,
         std::shared_ptr<ExpressionNode> right)
      : left(left), right(right) {}

  bool evaluate() const override {
    return left->evaluate() || right->evaluate();
  }
};

class NotNode : public ExpressionNode {
  std::shared_ptr<ExpressionNode> node;

 public:
  NotNode(std::shared_ptr<ExpressionNode> node) : node(node) {}
  bool evaluate() const override { return !node->evaluate(); }
};

class BooleanExpressionBuilder {
  std::shared_ptr<ExpressionNode> root;
  bool _isSet = false;

 public:
  BooleanExpressionBuilder& value(bool val) {
    root = std::make_shared<ValueNode>(val);
    _isSet = true;
    return *this;
  }

  BooleanExpressionBuilder& andOperation(std::shared_ptr<ExpressionNode> node) {
    root = std::make_shared<AndNode>(root, node);
    return *this;
  }

  BooleanExpressionBuilder& orOperation(std::shared_ptr<ExpressionNode> node) {
    root = std::make_shared<OrNode>(root, node);
    return *this;
  }

  BooleanExpressionBuilder& notOperation() {
    root = std::make_shared<NotNode>(root);
    return *this;
  }

  bool isSet() { return _isSet; }

  void reset() { _isSet = true; }

  bool evaluate() const { return root->evaluate(); }
};

#endif
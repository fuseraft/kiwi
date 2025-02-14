#ifndef KIWI_CALLABLE_H
#define KIWI_CALLABLE_H

#include <memory>
#include <unordered_set>
#include "parsing/ast.h"
#include "typing/value.h"

enum class KCallableType {
  Builtin,
  Function,
  Method,
  Lambda,
};

class KCallable {
 public:
  KCallableType type;
  std::vector<std::pair<k_string, KValue>> parameters;
  std::unordered_set<k_string> defaultParameters;
  KCallable(KCallableType type) : type(type) {}
  virtual ~KCallable() = default;
  virtual const std::vector<std::unique_ptr<ASTNode>>& getBody() const = 0;
};

class KBuiltin : public KCallable {
 public:
  k_string name;
  Token token;

  KBuiltin(const Token& token, const k_string& name)
      : KCallable(KCallableType::Builtin), name(name), token(token) {}

  const std::vector<std::unique_ptr<ASTNode>>& getBody() const override {
    return body;
  }

  std::unique_ptr<KCallable> clone() {
    auto cloned = std::make_unique<KBuiltin>(token, name);
    cloned->parameters = parameters;
    cloned->defaultParameters = defaultParameters;
    return cloned;
  }

 private:
  std::vector<std::unique_ptr<ASTNode>> body;
};

class KFunction : public KCallable {
 public:
  k_string name;
  std::unique_ptr<FunctionDeclarationNode> decl;
  bool isStatic = false;
  bool isPrivate = false;
  bool isCtor = false;
  std::unordered_map<k_string, KName> typeHints;
  KName returnTypeHint = KName::Types_Any;

  KFunction(std::unique_ptr<ASTNode> node)
      : KCallable(KCallableType::Function) {
    std::unique_ptr<FunctionDeclarationNode> nodeptr(
        dynamic_cast<FunctionDeclarationNode*>(node->clone().release()));
    decl = std::move(nodeptr);
  }

  const std::vector<std::unique_ptr<ASTNode>>& getBody() const override {
    return decl->body;
  }

  std::unique_ptr<KFunction> clone() {
    std::unique_ptr<FunctionDeclarationNode> nodeptr(
        dynamic_cast<FunctionDeclarationNode*>(decl->clone().release()));
    auto cloned = std::make_unique<KFunction>(std::move(nodeptr));
    cloned->name = name;
    cloned->isStatic = isStatic;
    cloned->isPrivate = isPrivate;
    cloned->isCtor = isCtor;
    cloned->parameters = parameters;
    cloned->defaultParameters = defaultParameters;
    cloned->typeHints = typeHints;
    cloned->returnTypeHint = returnTypeHint;
    return cloned;
  }
};

class KLambda : public KCallable {
 public:
  std::unique_ptr<LambdaNode> decl;
  std::unordered_map<k_string, KName> typeHints;
  KName returnTypeHint = KName::Types_Any;

  KLambda(std::unique_ptr<ASTNode> node) : KCallable(KCallableType::Lambda) {
    std::unique_ptr<LambdaNode> nodeptr(
        dynamic_cast<LambdaNode*>(node->clone().release()));
    decl = std::move(nodeptr);
  }

  const std::vector<std::unique_ptr<ASTNode>>& getBody() const override {
    return decl->body;
  }

  std::unique_ptr<KLambda> clone() {
    std::unique_ptr<LambdaNode> nodeptr(
        dynamic_cast<LambdaNode*>(decl->clone().release()));
    auto cloned = std::make_unique<KLambda>(std::move(nodeptr));
    cloned->parameters = parameters;
    cloned->defaultParameters = defaultParameters;
    cloned->typeHints = typeHints;
    cloned->returnTypeHint = returnTypeHint;
    return cloned;
  }
};

class KStruct {
 public:
  k_string name;
  k_string baseStruct;
  std::unordered_map<k_string, std::unique_ptr<KFunction>> methods;

  std::unique_ptr<KStruct> clone() const {
    auto cloned = std::make_unique<KStruct>();
    cloned->name = name;
    cloned->baseStruct = baseStruct;

    for (const auto& [methodName, methodPtr] : methods) {
      cloned->methods[methodName] = std::unique_ptr<KFunction>(
          static_cast<KFunction*>(methodPtr->clone().release()));
    }

    return cloned;
  }
};

class KPackage {
 public:
  std::unique_ptr<PackageNode> decl;

  KPackage(std::unique_ptr<ASTNode> node) {
    std::unique_ptr<PackageNode> nodeptr(
        dynamic_cast<PackageNode*>(node->clone().release()));
    decl = std::move(nodeptr);
  }

  std::unique_ptr<KPackage> clone() const {
    std::unique_ptr<PackageNode> nodeptr(
        dynamic_cast<PackageNode*>(decl->clone().release()));
    return std::make_unique<KPackage>(std::move(nodeptr));
  }
};

#endif
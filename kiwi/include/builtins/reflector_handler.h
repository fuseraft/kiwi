#ifndef KIWI_BUILTINS_REFLECTOR_H
#define KIWI_BUILTINS_REFLECTOR_H

#include <cstdlib>
#include <string>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/file.h"
#include "globals.h"

class ReflectorBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Reflector_RInspect:
        return executeRInspect(term, args);

      case KName::Builtin_Reflector_RList:
        return executeRList(term, args);

      case KName::Builtin_Reflector_RBin:
        return executeRBin(term, args);

      case KName::Builtin_Reflector_RLib:
        return executeRLib(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeRInspect(const Token& term,
                                 const std::vector<k_value>& args) {
    throw UnknownBuiltinError(term, ReflectorBuiltins.RInspect);
  }

  static k_value executeRBin(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, ReflectorBuiltins.RBin);
    }

    return File::getExecutablePath().string();
  }

  static k_value executeRLib(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, ReflectorBuiltins.RLib);
    }

    return File::getLibraryPath();
  }

  static k_value executeRList(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, ReflectorBuiltins.RList);
    }

    auto rlist = std::make_shared<Hash>();
    auto rlistPackages = std::make_shared<List>();
    auto rlistClasses = std::make_shared<List>();
    auto rlistMethods = std::make_shared<List>();
    auto rlistStack = std::make_shared<List>();
    
    for (const auto& m : methods) {
        rlistMethods->elements.emplace_back(m.first);
    }

    for (const auto& p : packages) {
        rlistPackages->elements.emplace_back(p.first);
    }

    for (const auto& c : classes) {
        rlistClasses->elements.emplace_back(c.first);
    }

    std::stack<std::shared_ptr<CallStackFrame>> tempStack(callStack);
    while (!tempStack.empty()) {
      const auto& outerFrame = tempStack.top();
      const auto& frameVariables = outerFrame->variables;
      const auto& frameLambdas = outerFrame->lambdas;
      const auto& frameAliases = outerFrame->aliases;
      
      auto rlistStackFrame = std::make_shared<Hash>();
      auto rlistStackFrameVariables = std::make_shared<List>();
      auto rlistStackFrameLambdas = std::make_shared<List>();
      
      for (const auto& v : frameVariables) {
        auto rlistStackFrameVariable = std::make_shared<Hash>();
        rlistStackFrameVariable->add(v.first, v.second);
        rlistStackFrameVariables->elements.emplace_back(rlistStackFrameVariable);
      }

      for (const auto& l : frameLambdas) {
        auto rlistStackFrameLambda = std::make_shared<Hash>();
        rlistStackFrameLambda->add(l.first, l.second.getName());
        rlistStackFrameLambdas->elements.emplace_back(rlistStackFrameLambda);
      }

      auto rlistStackFrameAliases = std::make_shared<List>();
      for (const auto& a : frameAliases) {
        rlistStackFrameAliases->elements.emplace_back(a);
      }

      rlistStackFrame->add("aliases", rlistStackFrameAliases);
      rlistStackFrame->add("lambdas", rlistStackFrameLambdas);
      rlistStackFrame->add("variables", rlistStackFrameVariables);
      
      rlistStack->elements.emplace_back(rlistStackFrame);

      tempStack.pop();
    }

    std::reverse(rlistStack->elements.begin(), rlistStack->elements.end());
    
    rlist->add("packages", rlistMethods);
    rlist->add("classes", rlistClasses);
    rlist->add("methods", rlistMethods);
    rlist->add("callstack", rlistStack);

    return rlist;
  }
};

#endif
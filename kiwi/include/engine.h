#ifndef KIWI_ENGINE_H
#define KIWI_ENGINE_H

#include <unordered_map>
#include <vector>
#include "parsing/lexer.h"
#include "parsing/parser.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"
#include "parsing/keywords.h"
#include "typing/value.h"
#include "util/file.h"
#include "globals.h"
#include "stackframe.h"

#include "interpreter.h"

const Token& engineToken = Token::createExternal();

class Engine {
 public:
  Engine() {}
  ~Engine() {}

  void setKiwiArgs(const std::unordered_map<k_string, k_string>& args) {
    kiwiArgs = args;
  }

  int runStreamCollection() {
    auto ast = parser.parseTokenStreamCollection(streamCollection);
    auto result = interp.interpret(ast.get());
    if (std::holds_alternative<k_int>(result)) {
      return static_cast<int>(std::get<k_int>(result));
    }
    return 0;
  }

  int interpretKiwi(const k_string& kiwiCode) {
    Lexer lexer(kiwi_arg, kiwiCode);
    auto tokenStream = lexer.getTokenStream();
    streamCollection.push_back(tokenStream);
    return runStreamCollection();
  }

  void printAST(const k_string& path) {
    auto content = File::readFile(engineToken, path);
    if (content.empty()) {
      return;
    }

    Lexer lexer(path, content);

    auto tokenStream = lexer.getTokenStream();
    auto ast = parser.parseTokenStream(tokenStream);
    auto node = ast.get();
    node->print(0);

    return;
  }

  int interpretScript(const k_string& path) {
    auto content = File::readFile(engineToken, path);
    if (content.empty()) {
      return 1;
    }

    Lexer lexer(path, content);

    auto tokenStream = lexer.getTokenStream();
    streamCollection.push_back(tokenStream);

    return 0;
  }

 private:
  Parser parser;
  KInterpreter interp;
  std::vector<k_stream> streamCollection;
};

#endif

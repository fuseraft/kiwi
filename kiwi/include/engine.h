#ifndef KIWI_ENGINE_H
#define KIWI_ENGINE_H

#include <unordered_map>
#include <vector>
#include "parsing/lexer.h"
#include "parsing/parser.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/file.h"
#include "globals.h"
#include "interpreter.h"

const Token& engineToken = Token::createExternal();

class Engine {
 public:
  Engine() {}
  ~Engine() {}

  void setProgramArgs(const std::unordered_map<k_string, k_string>& args) {
    interp.setProgramArgs(args);
  }

  int runStreamCollection() {
    auto ast = parser.parseTokenStreamCollection(streamCollection);

    interp.setContext(std::make_unique<KContext>());

    auto result = interp.interpret(ast.get());

    while (interp.hasActiveTasks()) {
      // avoid busy-waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (result.isInteger()) {
      return static_cast<int>(result.getInteger());
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

  int parseScript(const k_string& path) {
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

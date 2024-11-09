#ifndef KIWI_REPL_H
#define KIWI_REPL_H

#include <iostream>
#include <sstream>
#include <string>
#include "parsing/keywords.h"
#include "tracing/error.h"
#include "tracing/handler.h"
#include "engine.h"

#ifdef _WIN64
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

class Repl {
 public:
  Repl(Engine& engine);
  ~Repl();

  int run();

 private:
  /// @brief The REPL lines.
  std::vector<std::string> lines;
  /// @brief The active REPL line.
  size_t activeReplLine = 0;

  /// @brief The interpreter.
  Engine& engine;

  void beginREPLInput();
  void endREPLInput();
  char getch();
  std::string getline();
};

/// @brief Instantiate a REPL.
/// @param args REPL arguments.
Repl::Repl(Engine& engine) : engine(engine) {}

/// @brief Destroy the REPL.
Repl::~Repl() {}

/// @brief Run the REPL.
/// @return Integer containing exit code.
int Repl::run() {
  std::ostringstream input;
  int result = 0;

  std::cout << kiwi_name << " v" << kiwi_version << " REPL" << std::endl
            << std::endl
            << "Use `" << Keywords.Go << "` to execute, `" << Keywords.Exit
            << "` to exit the REPL." << std::endl
            << std::endl;

  lines.emplace_back("");
  activeReplLine = 1;

  while (true) {
    try {
      auto line = getline();

      if (line == Keywords.Go) {
        auto kiwiCode = input.str();
        result += engine.interpretKiwi(kiwiCode);
        input.str("");
        input.clear();
      } else if (line == Keywords.Exit) {
        break;
      } else {
        input << line << std::endl;
      }
    } catch (const KiwiError& e) {
      ErrorHandler::handleError(e);
    } catch (const std::exception& e) {
      ErrorHandler::printError(e);
    }
  }

  return result;
}

/// @brief Begin REPL input.
void Repl::beginREPLInput() {
#ifdef _WIN64
#else
  struct termios config;
  tcgetattr(0, &config);
  config.c_lflag &= ~ICANON;
  tcsetattr(0, TCSANOW, &config);
#endif
}

/// @brief End REPL input.
void Repl::endREPLInput() {
#ifdef _WIN64
#else
  struct termios config;
  tcgetattr(0, &config);
  config.c_lflag |= ICANON;
  tcsetattr(0, TCSANOW, &config);
#endif
}

/// @brief Get the next character of REPL input.
/// @return Character from REPL input.
char Repl::getch() {
#ifdef _WIN64
  return (char)_getch();
#else
  char ch;
  auto getchres = read(0, &ch, 1);
  if (getchres < 0) {
    // this is an error. 0 is EOF.
  }
  return ch;
#endif
}

/// @brief Get a line of text from REPL input.
/// @return String from REPL input.
std::string Repl::getline() {
  beginREPLInput();
  std::string line;
#ifdef _WIN64
  std::cout << "> ";
  std::getline(std::cin, line);
#else
  size_t pos = 0;

  while (true) {
    line = lines[activeReplLine - 1];
    std::cout << "\r> " << line << std::string(line.size() + 2, ' ') << "\r> "
              << line;

    for (size_t i = 0; i < line.length() - pos; i++) {
      std::cout << "\b";
    }
    std::cout.flush();

    char ch = getch();
    if (ch == '\033') {
      char arrow1 = getch();
      char arrow2 = getch();
      if (arrow1 == '[') {
        if (arrow2 == 'A') {
          if (static_cast<int>(activeReplLine) - 1 > 0) {
            --activeReplLine;
          }
          std::cout << "\033[2K\r" << std::flush;
        } else if (arrow2 == 'B') {
          if (activeReplLine + 1 < lines.size()) {
            ++activeReplLine;
          }
          std::cout << "\033[2K\r" << std::flush;
        } else if (arrow2 == 'C') {
          if (pos + 1 < line.size() + 1) {
            ++pos;
          }
        } else if (arrow2 == 'D') {
          if (pos > 0) {
            --pos;
          }
        }
        continue;
      }
    } else if (ch == ' ') {
      line.insert(pos++, 1, ' ');
    } else if (ch == '\b' || ch == 127) {
      if (pos > 0) {
        line.erase(--pos, 1);
      }
    } else if (ch == '\t') {
      line.insert(pos++, 1, ' ');
      line.insert(pos++, 1, ' ');
    } else if (ch == '\n') {
      lines[activeReplLine - 1] = line;
      lines.emplace_back("");
      activeReplLine = lines.size();
      std::cout << "\033[2K\r" << std::flush;
      break;
    } else {
      line.insert(pos++, 1, ch);
    }

    lines[activeReplLine - 1] = line;
  }
#endif

  endREPLInput();
  return line;
}

#endif
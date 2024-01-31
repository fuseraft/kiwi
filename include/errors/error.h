#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <exception>
#include "parsing/tokens.h"

class KiwiError : public std::exception {
 public:
  KiwiError(Token token, std::string error, std::string message = "")
      : token(token), error(error), message(message) {}

  const char* what() const noexcept override { return message.c_str(); }
  const Token getToken() const { return token; }
  const std::string getError() const { return error; }
  const std::string getMessage() const { return message; }

  static KiwiError create(std::string message = "") {
    return KiwiError(Token::createEmpty(), "KiwiError", message);
  }

 private:
  Token token;
  std::string error;
  std::string message;
};

class UnknownBuiltinError : public KiwiError {
 public:
  UnknownBuiltinError(const Token& token, std::string name)
      : KiwiError(token, "UnknownBuiltinError",
                  "The builtin `" + name + "` is unknown.") {}
};

class UnknownIdentifierError : public KiwiError {
 public:
  UnknownIdentifierError(const Token& token, std::string name)
      : KiwiError(token, "UnknownIdentifierError",
                  "The identifier `" + name + "` is unknown.") {}
};

class UnrecognizedTokenError : public KiwiError {
 public:
  UnrecognizedTokenError(const Token& token,
                         std::string message = "Unrecognized token.")
      : KiwiError(token, "UnrecognizedTokenError", message) {}
};

class SyntaxError : public KiwiError {
 public:
  SyntaxError(const Token& token, std::string message = "Invalid syntax.")
      : KiwiError(token, "SyntaxError", message) {}
};

class ParameterMissingError : public KiwiError {
 public:
  ParameterMissingError(const Token& token, std::string name)
      : KiwiError(token, "ParameterMissingError",
                  "The parameter `" + name + "` was expected but missing.") {}
};

class BuiltinUnexpectedArgumentError : public KiwiError {
 public:
  BuiltinUnexpectedArgumentError(const Token& token, std::string name)
      : KiwiError(token, "BuiltinUnexpectedArgumentError",
                  "Unexpected argument count for builtin `" + name + "`.") {}
};

class ConversionError : public KiwiError {
 public:
  ConversionError(const Token& token)
      : KiwiError(token, "ConversionError", "A conversion error occurred.") {}
};

class DivideByZeroError : public KiwiError {
 public:
  DivideByZeroError(const Token& token)
      : KiwiError(token, "DivideByZeroError", "Attempted to divide by zero.") {}
};

class ModuleUndefinedError : public KiwiError {
 public:
  ModuleUndefinedError(const Token& token, std::string name)
      : KiwiError(token, "ModuleUndefinedError",
                  "Module `" + name + "` is undefined.") {}
};

class MethodUndefinedError : public KiwiError {
 public:
  MethodUndefinedError(const Token& token, std::string name)
      : KiwiError(token, "MethodUndefinedError",
                  "Method `" + name + "` is undefined.") {}
};

class VariableUndefinedError : public KiwiError {
 public:
  VariableUndefinedError(const Token& token, std::string name)
      : KiwiError(token, "VariableUndefinedError",
                  "Variable `" + name + "` is undefined.") {}
};

class EmptyStackError : public KiwiError {
 public:
  EmptyStackError(const Token& token)
      : KiwiError(token, "EmptyStackError", "The stack is empty.") {}
};

// TODO: refine this.
class IllegalNameError : public KiwiError {
 public:
  IllegalNameError(const Token& token, std::string name)
      : KiwiError(token, "IllegalNameError",
                  "The name `" + name + "` is illegal.") {}
};

class FileNotFoundError : public KiwiError {
 public:
  FileNotFoundError(std::string path)
      : KiwiError(Token::createEmpty(), "FileNotFoundError",
                  "File not found: " + path) {}
  FileNotFoundError(const Token& token, std::string path)
      : KiwiError(token, "FileNotFoundError", "File not found: " + path) {}
};

#endif
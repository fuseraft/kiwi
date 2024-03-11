#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <exception>
#include "parsing/tokens.h"

class KiwiError : public std::exception {
 public:
  KiwiError(const Token& token, std::string error,
            const std::string& message = "")
      : token(token), error(error), message(message) {}

  const char* what() const noexcept override { return message.c_str(); }
  const Token getToken() const { return token; }
  const std::string getError() const { return error; }
  const std::string getMessage() const { return message; }

  static KiwiError create(const std::string& message = "") {
    return KiwiError(Token::createEmpty(), "KiwiError", message);
  }

 private:
  Token token;
  std::string error;
  std::string message;
};

class UnknownBuiltinError : public KiwiError {
 public:
  UnknownBuiltinError(const Token& token, const std::string& name)
      : KiwiError(token, "UnknownBuiltinError",
                  "The builtin `" + name + "` is unknown.") {}
};

class UnknownIdentifierError : public KiwiError {
 public:
  UnknownIdentifierError(const Token& token, const std::string& name)
      : KiwiError(token, "UnknownIdentifierError",
                  "The identifier `" + name + "` is unknown.") {}
};

class UnrecognizedTokenError : public KiwiError {
 public:
  UnrecognizedTokenError(const Token& token,
                         const std::string& message = "Unrecognized token.")
      : KiwiError(token, "UnrecognizedTokenError", message) {}
};

class SyntaxError : public KiwiError {
 public:
  SyntaxError(const Token& token,
              const std::string& message = "Invalid syntax.")
      : KiwiError(token, "SyntaxError", message) {}
};

class RangeError : public KiwiError {
 public:
  RangeError(const Token& token, const std::string& message = "Invalid range.")
      : KiwiError(token, "RangeError", message) {}
};

class IndexError : public KiwiError {
 public:
  IndexError(const Token& token, const std::string& message = "Invalid index.")
      : KiwiError(token, "IndexError", message) {}
};

class HashKeyError : public KiwiError {
 public:
  HashKeyError(const Token& token, const std::string& key)
      : KiwiError(token, "HashKeyError",
                  "The key `" + key + "` does not exist in the hash.") {}
};

class ParameterMissingError : public KiwiError {
 public:
  ParameterMissingError(const Token& token, const std::string& name)
      : KiwiError(token, "ParameterMissingError",
                  "The parameter `" + name + "` was expected but missing.") {}
};

class ParameterCountMismatchError : public KiwiError {
 public:
  ParameterCountMismatchError(const Token& token, const std::string& name)
      : KiwiError(token, "ParameterCountMismatchError",
                  "The parameter count for method `" + name +
                      "` does not match parameters passed.") {}
};

class UnrecognizedBuiltinError : public KiwiError {
 public:
  UnrecognizedBuiltinError(const Token& token, const std::string& name)
      : KiwiError(token, "UnrecognizedBuiltinError",
                  "Unrecognized builtin `" + name + "`.") {}
};

class ClassRedefinitionError : public KiwiError {
 public:
  ClassRedefinitionError(const Token& token, const std::string& name)
      : KiwiError(token, "ClassRedefinitionError",
                  "Attempted to redefine class already defined class `" + name +
                      "`.") {}
};

class ClassUndefinedError : public KiwiError {
 public:
  ClassUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "ClassUndefinedError",
                  "The class `" + name + "` is undefined.") {}
};

class UnimplementedMethodError : public KiwiError {
 public:
  UnimplementedMethodError(const Token& token, const std::string& className,
                           const std::string& methodName)
      : KiwiError(token, "UnimplementedMethodError",
                  "The class `" + className +
                      "` has an unimplemented method `" + methodName + "`") {}
};

class BuiltinUnexpectedArgumentError : public KiwiError {
 public:
  BuiltinUnexpectedArgumentError(const Token& token, const std::string& name)
      : KiwiError(token, "BuiltinUnexpectedArgumentError",
                  "Unexpected argument count for builtin `" + name + "`.") {}
};

class DbConnectionError : public KiwiError {
 public:
  DbConnectionError(
      const Token& token,
      const std::string& message = "A database connection error occurred.")
      : KiwiError(token, "DbConnectionError", message) {}
};

class DbError : public KiwiError {
 public:
  DbError(const Token& token,
          const std::string& message = "A general database error occurred.")
      : KiwiError(token, "DbError", message) {}
};

class InvalidContextError : public KiwiError {
 public:
  InvalidContextError(const Token& token,
                      const std::string& message = "Invalid context.")
      : KiwiError(token, "InvalidContextError", message) {}
};

class InvalidOperationError : public KiwiError {
 public:
  InvalidOperationError(const Token& token,
                        const std::string& message = "Invalid operation.")
      : KiwiError(token, "InvalidOperationError", message) {}
};

class InvalidTypeNameError : public KiwiError {
 public:
  InvalidTypeNameError(const Token& token, const std::string& name)
      : KiwiError(token, "InvalidTypeNameError",
                  "Invalid type name `" + name + "`") {}
};

class ConversionError : public KiwiError {
 public:
  ConversionError(const Token& token,
                  const std::string& message = "A conversion error occurred.")
      : KiwiError(token, "ConversionError", message) {}
};

class DivideByZeroError : public KiwiError {
 public:
  DivideByZeroError(const Token& token)
      : KiwiError(token, "DivideByZeroError", "Attempted to divide by zero.") {}
};

class HomeModuleUndefinedError : public KiwiError {
 public:
  HomeModuleUndefinedError(const Token& token, const std::string& home,
                           const std::string& name)
      : KiwiError(token, "HomeModuleUndefinedError",
                  "Home module `@" + home + "/" + name + "` is undefined.") {}
};

class ModuleUndefinedError : public KiwiError {
 public:
  ModuleUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "ModuleUndefinedError",
                  "Module `" + name + "` is undefined.") {}
};

class ModuleError : public KiwiError {
 public:
  ModuleError(const Token& token, const std::string& message)
      : KiwiError(token, "ModuleError", message) {}
};

class SystemError : public KiwiError {
 public:
  SystemError(const Token& token, const std::string& message)
      : KiwiError(token, "SystemError", message) {}
};

class MethodUndefinedError : public KiwiError {
 public:
  MethodUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "MethodUndefinedError",
                  "Method `" + name + "` is undefined.") {}
};

class VariableUndefinedError : public KiwiError {
 public:
  VariableUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "VariableUndefinedError",
                  "Variable `" + name + "` is undefined.") {}
};

class ValueTypeError : public KiwiError {
 public:
  ValueTypeError(const Token& token, const std::string& expectedType)
      : KiwiError(token, "ValueTypeError",
                  "Value is not a `" + expectedType + "`.") {}
};

class EmptyStackError : public KiwiError {
 public:
  EmptyStackError(const Token& token)
      : KiwiError(token, "EmptyStackError", "The stack is empty.") {}
};

class IllegalNameError : public KiwiError {
 public:
  IllegalNameError(const Token& token, const std::string& name)
      : KiwiError(token, "IllegalNameError",
                  "The name `" + name + "` is illegal.") {}
};

class FileNotFoundError : public KiwiError {
 public:
  FileNotFoundError(const std::string& path)
      : KiwiError(Token::createEmpty(), "FileNotFoundError",
                  "File not found: " + path) {}
  FileNotFoundError(const Token& token, const std::string& path)
      : KiwiError(token, "FileNotFoundError", "File not found: " + path) {}
};

class FileReadError : public KiwiError {
 public:
  FileReadError(const std::string& path)
      : KiwiError(Token::createEmpty(), "FileReadError",
                  "Cannot read file: " + path) {}
  FileReadError(const Token& token, const std::string& path)
      : KiwiError(token, "FileReadError", "Cannot read file: " + path) {}
};

class FileWriteError : public KiwiError {
 public:
  FileWriteError(const std::string& path)
      : KiwiError(Token::createEmpty(), "FileWriteError",
                  "Cannot write to file: " + path) {}
  FileWriteError(const Token& token, const std::string& path)
      : KiwiError(token, "FileWriteError", "Cannot write to file: " + path) {}
};

class FileSystemError : public KiwiError {
 public:
  FileSystemError(const std::string& message)
      : KiwiError(Token::createEmpty(), "FileSystemError", message) {}
  FileSystemError(const Token& token, const std::string& message)
      : KiwiError(token, "FileSystemError", message) {}
};

class OdbcError : public KiwiError {
 public:
  OdbcError(const std::string& message)
      : KiwiError(Token::createEmpty(), "OdbcError", message) {}
  OdbcError(const Token& token, const std::string& message)
      : KiwiError(token, "OdbcError", message) {}
};

template <typename T>
class Thrower {
 public:
  template <typename... Args>
  void throwError(const Token& token, Args&&... args) {
    static_assert(std::is_base_of<KiwiError, T>::value,
                  "T must be a subtype of `KiwiError`.");
    throw T(token, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void throwError(Args&&... args) {
    static_assert(std::is_base_of<KiwiError, T>::value,
                  "T must be a subtype of `KiwiError`.");
    Token emptyToken = Token::createEmpty();
    throw T(emptyToken, std::forward<Args>(args)...);
  }
};

#endif
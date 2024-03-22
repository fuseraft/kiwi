#ifndef ASTRAL_TRACING_ERROR_H
#define ASTRAL_TRACING_ERROR_H

#include <iostream>
#include <exception>
#include "parsing/tokens.h"

class AstralError : public std::exception {
 public:
  AstralError(const Token& token, std::string error,
            const std::string& message = "")
      : token(token), error(error), message(message) {}

  const char* what() const noexcept override { return message.c_str(); }
  const Token getToken() const { return token; }
  const std::string getError() const { return error; }
  const std::string getMessage() const { return message; }

  static AstralError create(const std::string& message = "") {
    return AstralError(Token::createEmpty(), "AstralError", message);
  }

 private:
  Token token;
  std::string error;
  std::string message;
};

class UnknownBuiltinError : public AstralError {
 public:
  UnknownBuiltinError(const Token& token, const std::string& name)
      : AstralError(token, "UnknownBuiltinError",
                  "The builtin `" + name + "` is unknown.") {}
};

class UnknownIdentifierError : public AstralError {
 public:
  UnknownIdentifierError(const Token& token, const std::string& name)
      : AstralError(token, "UnknownIdentifierError",
                  "The identifier `" + name + "` is unknown.") {}
};

class UnrecognizedTokenError : public AstralError {
 public:
  UnrecognizedTokenError(const Token& token,
                         const std::string& message = "Unrecognized token.")
      : AstralError(token, "UnrecognizedTokenError", message) {}
};

class SyntaxError : public AstralError {
 public:
  SyntaxError(const Token& token,
              const std::string& message = "Invalid syntax.")
      : AstralError(token, "SyntaxError", message) {}
};

class RangeError : public AstralError {
 public:
  RangeError(const Token& token, const std::string& message = "Invalid range.")
      : AstralError(token, "RangeError", message) {}
};

class IndexError : public AstralError {
 public:
  IndexError(const Token& token, const std::string& message = "Invalid index.")
      : AstralError(token, "IndexError", message) {}
};

class EmptyListError : public AstralError {
 public:
  EmptyListError(const Token& token,
                 const std::string& message = "This list is empty.")
      : AstralError(token, "EmptyListError", message) {}
};

class HashKeyError : public AstralError {
 public:
  HashKeyError(const Token& token, const std::string& key)
      : AstralError(token, "HashKeyError",
                  "The key `" + key + "` does not exist in the hash.") {}
};

class ParameterMissingError : public AstralError {
 public:
  ParameterMissingError(const Token& token, const std::string& name)
      : AstralError(token, "ParameterMissingError",
                  "The parameter `" + name + "` was expected but missing.") {}
};

class ParameterCountMismatchError : public AstralError {
 public:
  ParameterCountMismatchError(const Token& token, const std::string& name)
      : AstralError(token, "ParameterCountMismatchError",
                  "The parameter count for method `" + name +
                      "` does not match parameters passed.") {}
};

class UnrecognizedBuiltinError : public AstralError {
 public:
  UnrecognizedBuiltinError(const Token& token, const std::string& name)
      : AstralError(token, "UnrecognizedBuiltinError",
                  "Unrecognized builtin `" + name + "`.") {}
};

class ClassRedefinitionError : public AstralError {
 public:
  ClassRedefinitionError(const Token& token, const std::string& name)
      : AstralError(token, "ClassRedefinitionError",
                  "Attempted to redefine class already defined class `" + name +
                      "`.") {}
};

class ClassUndefinedError : public AstralError {
 public:
  ClassUndefinedError(const Token& token, const std::string& name)
      : AstralError(token, "ClassUndefinedError",
                  "The class `" + name + "` is undefined.") {}
};

class UnimplementedMethodError : public AstralError {
 public:
  UnimplementedMethodError(const Token& token, const std::string& className,
                           const std::string& methodName)
      : AstralError(token, "UnimplementedMethodError",
                  "The class `" + className +
                      "` has an unimplemented method `" + methodName + "`") {}
};

class BuiltinUnexpectedArgumentError : public AstralError {
 public:
  BuiltinUnexpectedArgumentError(const Token& token, const std::string& name)
      : AstralError(token, "BuiltinUnexpectedArgumentError",
                  "Unexpected argument count for builtin `" + name + "`.") {}
};

class DbConnectionError : public AstralError {
 public:
  DbConnectionError(
      const Token& token,
      const std::string& message = "A database connection error occurred.")
      : AstralError(token, "DbConnectionError", message) {}
};

class DbError : public AstralError {
 public:
  DbError(const Token& token,
          const std::string& message = "A general database error occurred.")
      : AstralError(token, "DbError", message) {}
};

class InvalidContextError : public AstralError {
 public:
  InvalidContextError(const Token& token,
                      const std::string& message = "Invalid context.")
      : AstralError(token, "InvalidContextError", message) {}
};

class InvalidOperationError : public AstralError {
 public:
  InvalidOperationError(const Token& token,
                        const std::string& message = "Invalid operation.")
      : AstralError(token, "InvalidOperationError", message) {}
};

class InvalidTypeNameError : public AstralError {
 public:
  InvalidTypeNameError(const Token& token, const std::string& name)
      : AstralError(token, "InvalidTypeNameError",
                  "Invalid type name `" + name + "`") {}
};

class ConversionError : public AstralError {
 public:
  ConversionError(const Token& token,
                  const std::string& message = "A conversion error occurred.")
      : AstralError(token, "ConversionError", message) {}
};

class DivideByZeroError : public AstralError {
 public:
  DivideByZeroError(const Token& token)
      : AstralError(token, "DivideByZeroError", "Attempted to divide by zero.") {}
};

class HomeModuleUndefinedError : public AstralError {
 public:
  HomeModuleUndefinedError(const Token& token, const std::string& home,
                           const std::string& name)
      : AstralError(token, "HomeModuleUndefinedError",
                  "Home module `@" + home + "/" + name + "` is undefined.") {}
};

class ModuleUndefinedError : public AstralError {
 public:
  ModuleUndefinedError(const Token& token, const std::string& name)
      : AstralError(token, "ModuleUndefinedError",
                  "Module `" + name + "` is undefined.") {}
};

class ModuleError : public AstralError {
 public:
  ModuleError(const Token& token, const std::string& message)
      : AstralError(token, "ModuleError", message) {}
};

class SystemError : public AstralError {
 public:
  SystemError(const Token& token, const std::string& message)
      : AstralError(token, "SystemError", message) {}
};

class MethodUndefinedError : public AstralError {
 public:
  MethodUndefinedError(const Token& token, const std::string& name)
      : AstralError(token, "MethodUndefinedError",
                  "Method `" + name + "` is undefined.") {}
};

class VariableUndefinedError : public AstralError {
 public:
  VariableUndefinedError(const Token& token, const std::string& name)
      : AstralError(token, "VariableUndefinedError",
                  "Variable `" + name + "` is undefined.") {}
};

class ValueTypeError : public AstralError {
 public:
  ValueTypeError(const Token& token, const std::string& expectedType)
      : AstralError(token, "ValueTypeError",
                  "Value is not a `" + expectedType + "`.") {}
};

class EmptyStackError : public AstralError {
 public:
  EmptyStackError(const Token& token)
      : AstralError(token, "EmptyStackError", "The stack is empty.") {}
};

class IllegalNameError : public AstralError {
 public:
  IllegalNameError(const Token& token, const std::string& name)
      : AstralError(token, "IllegalNameError",
                  "The name `" + name + "` is illegal.") {}
};

class FileNotFoundError : public AstralError {
 public:
  FileNotFoundError(const std::string& path)
      : AstralError(Token::createEmpty(), "FileNotFoundError",
                  "File not found: " + path) {}
  FileNotFoundError(const Token& token, const std::string& path)
      : AstralError(token, "FileNotFoundError", "File not found: " + path) {}
};

class FileReadError : public AstralError {
 public:
  FileReadError(const std::string& path)
      : AstralError(Token::createEmpty(), "FileReadError",
                  "Cannot read file: " + path) {}
  FileReadError(const Token& token, const std::string& path)
      : AstralError(token, "FileReadError", "Cannot read file: " + path) {}
};

class FileWriteError : public AstralError {
 public:
  FileWriteError(const std::string& path)
      : AstralError(Token::createEmpty(), "FileWriteError",
                  "Cannot write to file: " + path) {}
  FileWriteError(const Token& token, const std::string& path)
      : AstralError(token, "FileWriteError", "Cannot write to file: " + path) {}
};

class FileSystemError : public AstralError {
 public:
  FileSystemError(const std::string& message)
      : AstralError(Token::createEmpty(), "FileSystemError", message) {}
  FileSystemError(const Token& token, const std::string& message)
      : AstralError(token, "FileSystemError", message) {}
};

class OdbcError : public AstralError {
 public:
  OdbcError(const std::string& message)
      : AstralError(Token::createEmpty(), "OdbcError", message) {}
  OdbcError(const Token& token, const std::string& message)
      : AstralError(token, "OdbcError", message) {}
};

template <typename T>
class Thrower {
 public:
  template <typename... Args>
  void throwError(const Token& token, Args&&... args) {
    static_assert(std::is_base_of<AstralError, T>::value,
                  "T must be a subtype of `AstralError`.");
    throw T(token, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void throwError(Args&&... args) {
    static_assert(std::is_base_of<AstralError, T>::value,
                  "T must be a subtype of `AstralError`.");
    Token emptyToken = Token::createEmpty();
    throw T(emptyToken, std::forward<Args>(args)...);
  }
};

#endif
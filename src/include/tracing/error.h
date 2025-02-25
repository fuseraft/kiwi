#ifndef KIWI_TRACING_ERROR_H
#define KIWI_TRACING_ERROR_H

#include <iostream>
#include <exception>
#include "parsing/tokens.h"

const Token kiwiErrorToken = Token::createExternal();

class KiwiError : public std::exception {
 public:
  KiwiError(const Token& token, std::string error,
            const std::string& message = "")
      : token(token), error(error), message(message) {}

  const char* what() const noexcept override { return message.c_str(); }
  const Token getToken() const { return token; }
  const std::string& getError() const { return error; }
  const std::string& getMessage() const { return message; }

  static KiwiError create(const std::string& message = "") {
    return KiwiError(kiwiErrorToken, "KiwiError", message);
  }

  static KiwiError create(const Token& token, const std::string& message = "") {
    return KiwiError(token, "KiwiError", message);
  }

 private:
  Token token;
  std::string error;
  std::string message;
};

class InfiniteLoopError : public KiwiError {
 public:
  InfiniteLoopError(const Token& token, const std::string& message)
      : KiwiError(token, "InfiniteLoopError", message) {}
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

class TaskError : public KiwiError {
 public:
  TaskError(const Token& token,
            const std::string& message = "A task error occurred.")
      : KiwiError(token, "TaskError", message) {}
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

class EmptyListError : public KiwiError {
 public:
  EmptyListError(const Token& token,
                 const std::string& message = "This list is empty.")
      : KiwiError(token, "EmptyListError", message) {}
};

class HashKeyError : public KiwiError {
 public:
  HashKeyError(const Token& token, const std::string& key)
      : KiwiError(token, "HashKeyError",
                  "The key `" + key + "` does not exist in the hashmap.") {}
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
                  "The parameter count for function `" + name +
                      "` does not match parameters passed.") {}
};

class UnrecognizedBuiltinError : public KiwiError {
 public:
  UnrecognizedBuiltinError(const Token& token, const std::string& name)
      : KiwiError(token, "UnrecognizedBuiltinError",
                  "Unrecognized builtin `" + name + "`.") {}
};

class StructRedefinitionError : public KiwiError {
 public:
  StructRedefinitionError(const Token& token, const std::string& name)
      : KiwiError(
            token, "StructRedefinitionError",
            "Attempted to redefine already defined struct `" + name + "`.") {}
};

class StructUndefinedError : public KiwiError {
 public:
  StructUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "StructUndefinedError",
                  "Struct `" + name + "` is undefined.") {}
};

class UnimplementedMethodError : public KiwiError {
 public:
  UnimplementedMethodError(const Token& token, const std::string& structName,
                           const std::string& methodName)
      : KiwiError(token, "UnimplementedMethodError",
                  "Struct `" + structName + "` has an unimplemented method `" +
                      methodName + "`") {}
};

class BuiltinUnexpectedArgumentError : public KiwiError {
 public:
  BuiltinUnexpectedArgumentError(const Token& token, const std::string& name)
      : KiwiError(token, "BuiltinUnexpectedArgumentError",
                  "Unexpected argument count for builtin `" + name + "`.") {}
};

class ArgumentError : public KiwiError {
 public:
  ArgumentError(const Token& token, const std::string& message)
      : KiwiError(token, "ArgumentError", message) {}
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

class TypeError : public KiwiError {
 public:
  TypeError(const Token& token,
            const std::string& message = "A type error occurred.")
      : KiwiError(token, "TypeError", message) {}
};

class DivideByZeroError : public KiwiError {
 public:
  DivideByZeroError(const Token& token)
      : KiwiError(token, "DivideByZeroError", "Attempted to divide by zero.") {}
};

class HomePackageUndefinedError : public KiwiError {
 public:
  HomePackageUndefinedError(const Token& token, const std::string& home,
                            const std::string& name)
      : KiwiError(token, "HomePackageUndefinedError",
                  "Home package `@" + home + "/" + name + "` is undefined.") {}
};

class PackageUndefinedError : public KiwiError {
 public:
  PackageUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "PackageUndefinedError",
                  "Package `" + name + "` is undefined.") {}
};

class PackageError : public KiwiError {
 public:
  PackageError(const Token& token, const std::string& message)
      : KiwiError(token, "PackageError", message) {}
};

class SystemError : public KiwiError {
 public:
  SystemError(const Token& token, const std::string& message)
      : KiwiError(token, "SystemError", message) {}
};

class SignalError : public KiwiError {
 public:
  SignalError(const Token& token, const std::string& message)
      : KiwiError(token, "SignalError", message) {}
};

class FunctionUndefinedError : public KiwiError {
 public:
  FunctionUndefinedError(const Token& token, const std::string& name)
      : KiwiError(token, "FunctionUndefinedError",
                  "Function `" + name + "` is undefined.") {}
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
      : KiwiError(kiwiErrorToken, "FileNotFoundError",
                  "File not found: " + path) {}
  FileNotFoundError(const Token& token, const std::string& path)
      : KiwiError(token, "FileNotFoundError", "File not found: " + path) {}
};

class FileReadError : public KiwiError {
 public:
  FileReadError(const std::string& path)
      : KiwiError(kiwiErrorToken, "FileReadError",
                  "Cannot read file: " + path) {}
  FileReadError(const Token& token, const std::string& path)
      : KiwiError(token, "FileReadError", "Cannot read file: " + path) {}
};

class FileWriteError : public KiwiError {
 public:
  FileWriteError(const std::string& path)
      : KiwiError(kiwiErrorToken, "FileWriteError",
                  "Cannot write to file: " + path) {}
  FileWriteError(const Token& token, const std::string& path)
      : KiwiError(token, "FileWriteError", "Cannot write to file: " + path) {}
};

class FileSystemError : public KiwiError {
 public:
  FileSystemError(const std::string& message)
      : KiwiError(kiwiErrorToken, "FileSystemError", message) {}
  FileSystemError(const Token& token, const std::string& message)
      : KiwiError(token, "FileSystemError", message) {}
};

class FFIError : public KiwiError {
 public:
  FFIError(const Token& token,
           const std::string& message = "An FFI error occurred.")
      : KiwiError(token, "FFIError", message) {}
};

class SocketError : public KiwiError {
 public:
  SocketError(const Token& token,
              const std::string& message = "A socket error occurred.")
      : KiwiError(token, "SocketError", message) {}
};

class TokenStreamError : public KiwiError {
 public:
  TokenStreamError(const std::string& message)
      : KiwiError(kiwiErrorToken, "TokenStreamError", message) {}
  TokenStreamError(const Token& token, const std::string& message =
                                           "A token stream error occurred.")
      : KiwiError(token, "TokenStreamError", message) {}
};

#endif
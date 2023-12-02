#ifndef NOCTIS_ERROR_H
#define NOCTIS_ERROR_H

#include <iostream>
#include <unordered_map>

enum class ErrorCode {
    IS_NULL,
    BAD_LOAD,
    CONV_ERR,
    INVALID_OP,
    DIR_EXISTS,
    DIR_NOT_FOUND,
    FILE_EXISTS,
    FILE_NOT_FOUND,
    OUT_OF_BOUNDS,
    INVALID_RANGE_SEP,
    INVALID_SEQ,
    INVALID_SEQ_SEP,
    INVALID_VAR_DECL,
    LIST_UNDEFINED,
    METHOD_DEFINED,
    METHOD_UNDEFINED,
    NULL_NUMBER,
    NULL_STRING,
    CLS_METHOD_UNDEFINED,
    CLS_UNDEFINED,
    CLS_VAR_UNDEFINED,
    VAR_DEFINED,
    VAR_UNDEFINED,
    TARGET_UNDEFINED,
    CONST_DEFINED,
    INVALID_OPERATOR,
    IS_EMPTY,
    READ_FAIL,
    DIVIDED_BY_ZERO,
    UNDEFINED,
    MAKE_DIR_FAIL,
    CREATE_FILE_FAIL,
    REMOVE_DIR_FAIL,
    REMOVE_FILE_FAIL,
    UNKNOWN
};

class ErrorMessage {
public:
    static const std::unordered_map<ErrorCode, std::string> messages;
};

const std::unordered_map<ErrorCode, std::string> ErrorMessage::messages = {
    {ErrorCode::IS_NULL, "is null"},
    {ErrorCode::BAD_LOAD, "bad load"},
    {ErrorCode::CONV_ERR, "conversion error"},
    {ErrorCode::INVALID_OP, "invalid operation"},
    {ErrorCode::DIR_EXISTS, "directory already exists"},
    {ErrorCode::DIR_NOT_FOUND, "directory does not exist"},
    {ErrorCode::FILE_EXISTS, "file already exists"},
    {ErrorCode::FILE_NOT_FOUND, "file does not exist"},
    {ErrorCode::OUT_OF_BOUNDS, "index out of bounds"},
    {ErrorCode::INVALID_RANGE_SEP, "invalid range separator"},
    {ErrorCode::INVALID_SEQ, "invalid sequence"},
    {ErrorCode::INVALID_SEQ_SEP, "invalid sequence separator"},
    {ErrorCode::INVALID_VAR_DECL, "invalid variable declaration"},
    {ErrorCode::LIST_UNDEFINED, "list undefined"},
    {ErrorCode::METHOD_DEFINED, "method defined"},
    {ErrorCode::METHOD_UNDEFINED, "method undefined"},
    {ErrorCode::NULL_NUMBER, "null number"},
    {ErrorCode::NULL_STRING, "null string"},
    {ErrorCode::CLS_METHOD_UNDEFINED, "class method undefined"},
    {ErrorCode::CLS_UNDEFINED, "class undefined"},
    {ErrorCode::CLS_VAR_UNDEFINED, "class variable undefined"},
    {ErrorCode::VAR_DEFINED, "variable defined"},
    {ErrorCode::VAR_UNDEFINED, "variable undefined"},
    {ErrorCode::TARGET_UNDEFINED, "target undefined"},
    {ErrorCode::CONST_DEFINED, "constant defined"},
    {ErrorCode::INVALID_OPERATOR, "invalid operator"},
    {ErrorCode::IS_EMPTY, "is empty"},
    {ErrorCode::READ_FAIL, "read failure"},
    {ErrorCode::DIVIDED_BY_ZERO, "cannot divide by zero"},
    {ErrorCode::UNDEFINED, "undefined"},
    {ErrorCode::MAKE_DIR_FAIL, "could not create directory"},
    {ErrorCode::CREATE_FILE_FAIL, "could not create file"},
    {ErrorCode::REMOVE_DIR_FAIL, "could not remove directory"},
    {ErrorCode::REMOVE_FILE_FAIL, "could not remove file"},
    {ErrorCode::UNKNOWN, "unknown error"}
};

class Error {
public:
    static std::string getErrorString(ErrorCode errorType) {
        auto it = ErrorMessage::messages.find(errorType);
        if (it != ErrorMessage::messages.end()) {
            return it->second;
        }
        return ErrorMessage::messages.at(ErrorCode::UNKNOWN);
    }
};

#endif
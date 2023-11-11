#ifndef NOCTIS_ERROR_H
#define NOCTIS_ERROR_H

class ErrorMessage {
    public:
    static const int IS_NULL =	0;
    static const int BAD_LOAD = 1;
    static const int CONV_ERR = 2;
    static const int INVALID_OP = 4;
    static const int DIR_EXISTS = 5;
    static const int DIR_NOT_FOUND = 6;
    static const int FILE_EXISTS =	7;
    static const int FILE_NOT_FOUND = 8;
    static const int OUT_OF_BOUNDS = 9;
    static const int INVALID_RANGE_SEP = 10;
    static const int INVALID_SEQ = 11;
    static const int INVALID_SEQ_SEP = 12;
    static const int INVALID_VAR_DECL = 13;
    static const int LIST_UNDEFINED = 14;
    static const int METHOD_DEFINED = 15;
    static const int METHOD_UNDEFINED = 16;
    static const int NULL_NUMBER = 17;
    static const int NULL_STRING = 18;
    static const int CLS_METHOD_UNDEFINED = 19;
    static const int CLS_UNDEFINED = 20;
    static const int CLS_VAR_UNDEFINED = 21;
    static const int VAR_DEFINED = 22;
    static const int VAR_UNDEFINED = 23;
    static const int TARGET_UNDEFINED = 24;
    static const int CONST_UNDEFINED = 25;
    static const int CONST_DEFINED = 26;
    static const int INVALID_OPERATOR = 27;
    static const int IS_EMPTY = 28;
    static const int READ_FAIL = 29;
    static const int DIVIDED_BY_ZERO = 30;
    static const int UNDEFINED = 31;
    static const int UNDEFINED_OS = 32;
    static const int MAKE_DIR_FAIL = 33;
    static const int REMOVE_DIR_FAIL = 34;
    static const int REMOVE_FILE_FAIL = 35;
};

class Error {
    public:
    Error() {}
    ~Error() {}

    static string getErrorString(int errorType)
    {
        string errorString("");

        switch (errorType)
        {
            case ErrorMessage::IS_NULL:
                errorString.append("is null");
                break;
            case ErrorMessage::BAD_LOAD:
                errorString.append("bad load");
                break;
            case ErrorMessage::CONV_ERR:
                errorString.append("conversion error");
                break;
            case ErrorMessage::INVALID_OP:
                errorString.append("invalid operation");
                break;
            case ErrorMessage::DIR_EXISTS:
                errorString.append("directory already exists");
                break;
            case ErrorMessage::DIR_NOT_FOUND:
                errorString.append("directory does not exist");
                break;
            case ErrorMessage::FILE_EXISTS:
                errorString.append("file already exists");
                break;
            case ErrorMessage::FILE_NOT_FOUND:
                errorString.append("file does not exist");
                break;
            case ErrorMessage::OUT_OF_BOUNDS:
                errorString.append("index out of bounds");
                break;
            case ErrorMessage::INVALID_RANGE_SEP:
                errorString.append("invalid range separator");
                break;
            case ErrorMessage::INVALID_SEQ:
                errorString.append("invalid sequence");
                break;
            case ErrorMessage::INVALID_SEQ_SEP:
                errorString.append("invalid sequence separator");
                break;
            case ErrorMessage::INVALID_VAR_DECL:
                errorString.append("invalid variable declaration");
                break;
            case ErrorMessage::LIST_UNDEFINED:
                errorString.append("list undefined");
                break;
            case ErrorMessage::METHOD_DEFINED:
                errorString.append("method defined");
                break;
            case ErrorMessage::METHOD_UNDEFINED:
                errorString.append("method undefined");
                break;
            case ErrorMessage::NULL_NUMBER:
                errorString.append("null number");
                break;
            case ErrorMessage::NULL_STRING:
                errorString.append("null string");
                break;
            case ErrorMessage::CLS_METHOD_UNDEFINED:
                errorString.append("class method undefined");
                break;
            case ErrorMessage::CLS_UNDEFINED:
                errorString.append("class undefined");
                break;
            case ErrorMessage::CLS_VAR_UNDEFINED:
                errorString.append("class variable undefined");
                break;
            case ErrorMessage::VAR_DEFINED:
                errorString.append("variable defined");
                break;
            case ErrorMessage::VAR_UNDEFINED:
                errorString.append("variable undefined");
                break;
            case ErrorMessage::TARGET_UNDEFINED:
                errorString.append("target undefined");
                break;
            case ErrorMessage::CONST_UNDEFINED:
                errorString.append("constant defined");
                break;
            case ErrorMessage::INVALID_OPERATOR:
                errorString.append("invalid operator");
                break;
            case ErrorMessage::IS_EMPTY:
                errorString.append("is empty");
                break;
            case ErrorMessage::READ_FAIL:
                errorString.append("read failure");
                break;
            case ErrorMessage::DIVIDED_BY_ZERO:
                errorString.append("cannot divide by zero");
                break;
            case ErrorMessage::UNDEFINED:
                errorString.append("undefined");
                break;
            case ErrorMessage::UNDEFINED_OS:
                errorString.append("undefined_os");
                break;
            case ErrorMessage::MAKE_DIR_FAIL:
                errorString.append("could not create directory");
                break;
            case ErrorMessage::REMOVE_DIR_FAIL:
                errorString.append("could not remove directory");
                break;
            case ErrorMessage::REMOVE_FILE_FAIL:
                errorString.append("could not remove file");
                break;
        }

        return errorString;
    }
};

#endif
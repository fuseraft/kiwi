/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#ifndef ERRORS_H
#define ERRORS_H

void error(int errorType, string errorMessage, bool quit);
string getErrorString(int errorType);

const int IS_NULL =	0;
const int BAD_LOAD = 1;
const int CONV_ERR = 2;
const int INFINITE_LOOP = 3;
const int INVALID_OP = 4;
const int DIR_EXISTS = 5;
const int DIR_NOT_FOUND = 6;
const int FILE_EXISTS =	7;
const int FILE_NOT_FOUND = 8;
const int OUT_OF_BOUNDS = 9;
const int INVALID_RANGE_SEP = 10;
const int INVALID_SEQ = 11;
const int INVALID_SEQ_SEP = 12;
const int INVALID_VAR_DECL = 13;
const int LIST_UNDEFINED = 14;
const int METHOD_DEFINED = 15;
const int METHOD_UNDEFINED = 16;
const int NULL_NUMBER = 17;
const int NULL_STRING = 18;
const int OBJ_METHOD_UNDEFINED = 19;
const int OBJ_UNDEFINED = 20;
const int OBJ_VAR_UNDEFINED = 21;
const int VAR_DEFINED = 22;
const int VAR_UNDEFINED = 23;
const int TARGET_UNDEFINED = 24;
const int CONST_UNDEFINED = 25;
const int INVALID_OPERATOR = 26;
const int IS_EMPTY = 27;
const int READ_FAIL = 28;
const int DIVIDED_BY_ZERO = 29;
const int UNDEFINED = 30;
const int UNDEFINED_OS = 31;

string getErrorString(int errorType)
{
    string errorString("");

    switch (errorType)
    {
    case IS_NULL:
        errorString.append("is null");
        break;
    case BAD_LOAD:
        errorString.append("bad load");
        break;
    case CONV_ERR:
        errorString.append("conversion error");
        break;
    case INFINITE_LOOP:
        errorString.append("infinite loop");
    case INVALID_OP:
        errorString.append("invalid operation");
        break;
    case DIR_EXISTS:
        errorString.append("directory already exists");
        break;
    case DIR_NOT_FOUND:
        errorString.append("directory does not exist");
        break;
    case FILE_EXISTS:
        errorString.append("file already exists");
        break;
    case FILE_NOT_FOUND:
        errorString.append("file does not exist");
        break;
    case OUT_OF_BOUNDS:
        errorString.append("index out of bounds");
        break;
    case INVALID_RANGE_SEP:
        errorString.append("invalid range separator");
        break;
    case INVALID_SEQ:
        errorString.append("invalid sequence");
        break;
    case INVALID_SEQ_SEP:
        errorString.append("invalid sequence separator");
        break;
    case INVALID_VAR_DECL:
        errorString.append("invalid variable declaration");
        break;
    case LIST_UNDEFINED:
        errorString.append("list undefined");
        break;
    case METHOD_DEFINED:
        errorString.append("method defined");
        break;
    case METHOD_UNDEFINED:
        errorString.append("method undefined");
        break;
    case NULL_NUMBER:
        errorString.append("null number");
        break;
    case NULL_STRING:
        errorString.append("null string");
        break;
    case OBJ_METHOD_UNDEFINED:
        errorString.append("object method undefined");
        break;
    case OBJ_UNDEFINED:
        errorString.append("object undefined");
        break;
    case OBJ_VAR_UNDEFINED:
        errorString.append("object variable undefined");
        break;
    case VAR_DEFINED:
        errorString.append("variable defined");
        break;
    case VAR_UNDEFINED:
        errorString.append("variable undefined");
        break;
    case TARGET_UNDEFINED:
        errorString.append("target undefined");
        break;
    case CONST_UNDEFINED:
        errorString.append("constant defined");
        break;
    case INVALID_OPERATOR:
        errorString.append("invalid operator");
        break;
    case IS_EMPTY:
        errorString.append("is empty");
        break;
    case READ_FAIL:
        errorString.append("read failure");
        break;
    case DIVIDED_BY_ZERO:
        errorString.append("cannot divide by zero");
        break;
    case UNDEFINED:
        errorString.append("undefined");
        break;
    case UNDEFINED_OS:
        errorString.append("undefined_os");
        break;
    }

    return errorString;
}

#endif

#ifndef ASTRAL_DB_DATABASE_H
#define ASTRAL_DB_DATABASE_H

#ifdef EXPERIMENTAL_FEATURES
#include <iostream>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>
#include <sql.h>
#include <sqlext.h>
#include "errors.h"
#include "typing/value.h"

struct {
  const k_string HasError = "has_error";
  const k_string Connected = "connected";
  const k_string ErrorMessage = "error_msg";
  const k_string NativeError = "native_error";
  const k_string SqlState = "sql_state";
  const k_string Data = "data";
  const k_string InTransaction = "in_transaction";
} DatabaseResponseKeys;

class OdbcEnvironment {
 public:
  OdbcEnvironment() {
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_SUCCESS) {
      SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    } else {
      Thrower<OdbcError> thrower;
      thrower.throwError("Failed to allocate ODBC environment handle.");
    }
  }

  ~OdbcEnvironment() {
    if (hEnv != SQL_NULL_HENV) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
  }

  SQLHENV getHandle() const { return hEnv; }

 private:
  SQLHENV hEnv = SQL_NULL_HENV;
};

class OdbcConnection {
 public:
  // Deleted copy constructor and assignment operator for singleton pattern
  OdbcConnection(const OdbcConnection&) = delete;
  OdbcConnection& operator=(const OdbcConnection&) = delete;

  static OdbcConnection& getInstance(const k_string& connectionString) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    static OdbcConnection instance(connectionString);
    return instance;
  }

  ~OdbcConnection() { disconnect(); }

  k_value beginTransaction() {
    if (!connected) {
      return getConnectionError();
    }
    setAutoCommit(false);

    RETCODE retCode =
        SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT,
                          (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    SQLUINTEGER autoCommitMode;
    retCode =
        SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autoCommitMode, 0, NULL);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction,
                autoCommitMode == SQL_AUTOCOMMIT_OFF);
    return result;
  }

  k_value commitTransaction() {
    if (!connected) {
      return getConnectionError();
    }

    RETCODE retCode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
    setAutoCommit(true);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction, false);
    return result;
  }

  k_value rollbackTransaction() {
    if (!connected) {
      return getConnectionError();
    }

    RETCODE retCode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
    setAutoCommit(true);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction, false);
    return result;
  }

  k_value isInTransaction() {
    if (!connected) {
      return getConnectionError();
    }

    SQLUINTEGER autoCommitMode;
    RETCODE retCode =
        SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autoCommitMode, 0, NULL);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction,
                autoCommitMode == SQL_AUTOCOMMIT_OFF);
    return result;
  }

  k_value executeSql(const k_string& sql) {
    if (!connected) {
      return getConnectionError();
    }

    SQLHSTMT hStmt;
    RETCODE retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    retCode = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

    k_value result = processResultSet(hStmt, retCode);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return result;
  }

  k_value executeStoredProcedure(const k_string& sp, const k_list params) {
    if (!connected) {
      return getConnectionError();
    }

    SQLHSTMT hStmt;
    RETCODE retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_string query = "EXEC " + sanitizeString(sp) + " ";

    for (const auto& param : params->elements) {
      if (std::holds_alternative<k_int>(param)) {
        query += std::to_string(std::get<k_int>(param)) + ",";
      } else if (std::holds_alternative<double>(param)) {
        query += std::to_string(std::get<double>(param)) + ",";
      } else if (std::holds_alternative<bool>(param)) {
        query += std::to_string(std::get<bool>(param)) + ",";
      } else if (std::holds_alternative<k_string>(param)) {
        query += "'" + sanitizeString(std::get<k_string>(param)) + "',";
      }
    }

    query.pop_back();  // Remove the trailing comma

    retCode = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    k_value result = processResultSet(hStmt, retCode);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return result;
  }

  k_value connect() { return _connect(); }

  bool isConnected() const { return connected; }

 private:
  k_string connectionString;
  SQLHENV hEnv;
  SQLHDBC hDbc;
  bool connected = false;

  explicit OdbcConnection(const k_string& connStr)
      : connectionString(connStr), hEnv(NULL), hDbc(NULL) {}

  k_value _connect() {
    // Allocate environment handle
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) {
      return getConnectionError("Failed to allocate ODBC environment handle.");
    }

    // Set the ODBC version environment attribute
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0) !=
        SQL_SUCCESS) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      return getConnectionError("Failed to set ODBC version.");
    }

    // Allocate connection handle
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      return getConnectionError("Failed to allocate ODBC connection handle.");
    }

    // Connect to the data source
    SQLCHAR outConnectionString[1024];
    SQLSMALLINT outConnectionStringLength = 0;
    RETCODE retCode = SQLDriverConnect(
        hDbc, NULL,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(connectionString.c_str())),
        SQL_NTS, outConnectionString, sizeof(outConnectionString),
        &outConnectionStringLength, SQL_DRIVER_NOPROMPT);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    connected = (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO);

    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    return result;
  }

  void disconnect() {
    if (connected) {
      SQLDisconnect(hDbc);
      connected = false;
    }
    if (hDbc != SQL_NULL_HDBC) {
      SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    }
    if (hEnv != SQL_NULL_HENV) {
      SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
  }

  void setAutoCommit(bool autoCommit) {
    SQLSetConnectAttr(
        hDbc, SQL_ATTR_AUTOCOMMIT,
        autoCommit ? (void*)SQL_AUTOCOMMIT_ON : (void*)SQL_AUTOCOMMIT_OFF, 0);
  }

  k_string sanitizeString(const k_string& input) {
    k_string sanitized;
    for (char c : input) {
      switch (c) {
        case '\'':
          sanitized += "''";
          break;
        case '\\':
          sanitized += "\\\\";
          break;
        default:
          sanitized += c;
          break;
      }
    }
    return sanitized;
  }

  k_value processResultSet(SQLHSTMT hStmt, RETCODE retCode) {
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);

    SQLSMALLINT columns;
    SQLRETURN ret = SQLNumResultCols(hStmt, &columns);
    if (ret != SQL_SUCCESS) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    k_list resultList = std::make_shared<List>();
    auto& elements = resultList->elements;
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
      k_hash row = std::make_shared<Hash>();
      for (SQLSMALLINT i = 1; i <= columns; ++i) {
        SQLCHAR columnName[256];
        SQLLEN columnNameLength;
        SQLGetData(hStmt, i, SQL_C_CHAR, columnName, sizeof(columnName),
                   &columnNameLength);

        SQLCHAR columnData[256];
        SQLLEN columnDataLength;
        SQLGetData(hStmt, i, SQL_C_CHAR, columnData, sizeof(columnData),
                   &columnDataLength);

        row->add(
            k_string(reinterpret_cast<char*>(columnName), columnNameLength),
            k_string(reinterpret_cast<char*>(columnData), columnDataLength));
      }

      elements.emplace_back(row);
    }

    result->add(DatabaseResponseKeys.Data, resultList);
    return result;
  }

  k_value getLastErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle) {
    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, true);
    result->add(DatabaseResponseKeys.Connected, connected);

    SQLINTEGER i = 0;
    SQLINTEGER nativeError;
    SQLCHAR sqlState[7];
    SQLCHAR messageText[1024];
    SQLSMALLINT textLength;
    k_string fullErrorMessage;

    while (SQLGetDiagRec(handleType, handle, ++i, sqlState, &nativeError,
                         messageText, sizeof(messageText),
                         &textLength) == SQL_SUCCESS) {
      if (!fullErrorMessage.empty()) {
        fullErrorMessage += " ";
      }

      k_string currentMessage(reinterpret_cast<char*>(messageText), textLength);
      fullErrorMessage += currentMessage;
    }

    return result;
  }

  k_value getConnectionError(const k_string& message = "") {
    k_hash result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, true);
    result->add(DatabaseResponseKeys.Connected, false);
    result->add(DatabaseResponseKeys.ErrorMessage, message);
    return result;
  }
};

class OdbcStatement {
 public:
  explicit OdbcStatement(const SQLHDBC& hDbc) : hDbc(hDbc) {
    SQLAllocHandle(SQL_HANDLE_STMT, this->hDbc, &hStmt);
  }

  ~OdbcStatement() { SQLFreeHandle(SQL_HANDLE_STMT, hStmt); }

  void execute(const k_string& sql) {
    SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
  }

 private:
  SQLHDBC hDbc;
  SQLHSTMT hStmt;
};

#endif

#endif
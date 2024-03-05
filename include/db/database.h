#ifndef KIWI_DB_DATABASE_H
#define KIWI_DB_DATABASE_H

#include <iostream>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>
#include <sql.h>
#include <sqlext.h>
#include "k_int.h"
#include "typing/valuetype.h"

struct {
  const std::string HasError = "has_error";
  const std::string Connected = "connected";
  const std::string ErrorMessage = "error_msg";
  const std::string NativeError = "native_error";
  const std::string SqlState = "sql_state";
  const std::string Data = "data";
  const std::string InTransaction = "in_transaction";
} DatabaseResponseKeys;

class OdbcEnvironment {
 public:
  OdbcEnvironment() {
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_SUCCESS) {
      SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    } else {
      throw std::runtime_error("Failed to allocate ODBC environment handle.");
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

  static OdbcConnection& getInstance(const std::string& connectionString) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    static OdbcConnection instance(connectionString);
    return instance;
  }

  ~OdbcConnection() { disconnect(); }

  Value beginTransaction() {
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

    std::shared_ptr<Hash> result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction,
                autoCommitMode == SQL_AUTOCOMMIT_OFF);
    return result;
  }

  Value commitTransaction() {
    if (!connected) {
      return getConnectionError();
    }

    RETCODE retCode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
    setAutoCommit(true);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    std::shared_ptr<Hash> result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction, false);
    return result;
  }

  Value rollbackTransaction() {
    if (!connected) {
      return getConnectionError();
    }

    RETCODE retCode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
    setAutoCommit(true);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    std::shared_ptr<Hash> result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction, false);
    return result;
  }

  Value isInTransaction() {
    if (!connected) {
      return getConnectionError();
    }

    SQLUINTEGER autoCommitMode;
    RETCODE retCode =
        SQLGetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, &autoCommitMode, 0, NULL);

    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    std::shared_ptr<Hash> result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);
    result->add(DatabaseResponseKeys.InTransaction,
                autoCommitMode == SQL_AUTOCOMMIT_OFF);
    return result;
  }

  Value executeSql(const std::string& sql) {
    if (!connected) {
      return getConnectionError();
    }

    SQLHSTMT hStmt;
    RETCODE retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    retCode = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

    Value result = processResultSet(hStmt, retCode);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return result;
  }

  Value executeStoredProcedure(const std::string& sp,
                               const std::shared_ptr<List> params) {
    if (!connected) {
      return getConnectionError();
    }

    SQLHSTMT hStmt;
    RETCODE retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    std::string query = "EXEC " + sanitizeString(sp) + " ";

    for (const auto& param : params->elements) {
      if (std::holds_alternative<k_int>(param)) {
        query += std::to_string(std::get<k_int>(param)) + ",";
      } else if (std::holds_alternative<double>(param)) {
        query += std::to_string(std::get<double>(param)) + ",";
      } else if (std::holds_alternative<bool>(param)) {
        query += std::to_string(std::get<bool>(param)) + ",";
      } else if (std::holds_alternative<std::string>(param)) {
        query += "'" + sanitizeString(std::get<std::string>(param)) + "',";
      }
    }

    query.pop_back();  // Remove the trailing comma

    retCode = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    Value result = processResultSet(hStmt, retCode);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return result;
  }

  Value connect() { return _connect(); }

  bool isConnected() const { return connected; }

 private:
  std::string connectionString;
  SQLHENV hEnv;
  SQLHDBC hDbc;
  bool connected = false;

  explicit OdbcConnection(const std::string& connStr)
      : connectionString(connStr), hEnv(NULL), hDbc(NULL) {}

  Value _connect() {
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

    std::shared_ptr<Hash> result = std::make_shared<Hash>();
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

  std::string sanitizeString(const std::string& input) {
    std::string sanitized;
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

  Value processResultSet(SQLHSTMT hStmt, RETCODE retCode) {
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    std::shared_ptr<Hash> result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, false);
    result->add(DatabaseResponseKeys.Connected, connected);

    SQLSMALLINT columns;
    SQLRETURN ret = SQLNumResultCols(hStmt, &columns);
    if (ret != SQL_SUCCESS) {
      return getLastErrorMessage(SQL_HANDLE_DBC, hDbc);
    }

    std::shared_ptr<List> resultList = std::make_shared<List>();
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
      std::shared_ptr<Hash> row = std::make_shared<Hash>();
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
            std::string(reinterpret_cast<char*>(columnName), columnNameLength),
            std::string(reinterpret_cast<char*>(columnData), columnDataLength));
      }

      resultList->elements.push_back(row);
    }

    result->add(DatabaseResponseKeys.Data, resultList);
    return result;
  }

  Value getLastErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle) {
    std::shared_ptr<Hash> result = std::make_shared<Hash>();
    result->add(DatabaseResponseKeys.HasError, true);
    result->add(DatabaseResponseKeys.Connected, connected);

    SQLINTEGER i = 0;
    SQLINTEGER nativeError;
    SQLCHAR sqlState[7];
    SQLCHAR messageText[1024];
    SQLSMALLINT textLength;
    std::string fullErrorMessage;

    while (SQLGetDiagRec(handleType, handle, ++i, sqlState, &nativeError,
                         messageText, sizeof(messageText),
                         &textLength) == SQL_SUCCESS) {
      if (!fullErrorMessage.empty()) {
        fullErrorMessage += " ";
      }

      std::string currentMessage(reinterpret_cast<char*>(messageText),
                                 textLength);
      fullErrorMessage += currentMessage;
    }

    return result;
  }

  Value getConnectionError(const std::string& message = "") {
    std::shared_ptr<Hash> result = std::make_shared<Hash>();
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

  void execute(const std::string& sql) {
    SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
  }

 private:
  SQLHDBC hDbc;
  SQLHSTMT hStmt;
};

#endif
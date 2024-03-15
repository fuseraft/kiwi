#ifndef KIWI_BUILTINS_ODBCHANDLER_H
#define KIWI_BUILTINS_ODBCHANDLER_H

#include <vector>
#include "db/database.h"
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"

struct ConnectionInfo {
  std::string connectionString;
  bool hasConnection = false;

  static ConnectionInfo build(const Token& term, const Value& value,
                              const std::string& builtin) {
    ConnectionInfo conn;
    std::shared_ptr<Hash> hash;

    if (!std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      throw DbConnectionError(term,
                              "Expected a Hash with a `connection_string` key "
                              "in call to builtin `" +
                                  builtin + "`.");
    }

    hash = std::get<std::shared_ptr<Hash>>(value);

    if (hash->hasKey("connection_string")) {
      conn.connectionString = get_string(term, hash->get("connection_string"));
      conn.hasConnection = true;
    }

    if (!conn.hasConnection) {
      throw DbConnectionError(term,
                              "Connection hash does not contain a "
                              "`connection_string` key in call to builtin `" +
                                  builtin + "`.");
    }

    if (conn.connectionString.empty()) {
      throw DbConnectionError(
          term,
          "Connection string is empty in call to builtin `" + builtin + "`.");
    }

    return conn;
  }
};

class OdbcBuiltinHandler {
 public:
  static Value execute(const Token& term, const std::string& builtin,
                       const std::vector<Value>& args) {
    try {
      switch (builtin) {
        case OdbcBuiltins.Connect:
          return executeConnect(term, args);

        case OdbcBuiltins.Exec:
          return executeExec(term, args);

        case OdbcBuiltins.ExecSp:
          return executeExecSp(term, args);

        case OdbcBuiltins.IsConnected:
          return executeIsConnected(term, args);

        case OdbcBuiltins.InTransaction:
          return executeInTransaction(term, args);

        case OdbcBuiltins.BeginTransaction:
          return executeBeginTransaction(term, args);

        case OdbcBuiltins.CommitTransaction:
          return executeCommitTransaction(term, args);

        case OdbcBuiltins.RollbackTransaction:
          return executeRollbackTransaction(term, args);

        default:
          break;
      }
    } catch (const std::exception& e) {
      throw DbError(term, "ODBC handler error: " + std::string(e.what()));
    }

    throw UnknownBuiltinError(term, builtin);
  }

 private:
  static Value executeConnect(const Token& term,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.Connect);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.Connect);

    return OdbcConnection::getInstance(conn.connectionString).connect();
    ;
  }

  static Value executeExec(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.Exec);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.Exec);

    std::string sql = get_string(term, args.at(1));
    return OdbcConnection::getInstance(conn.connectionString).executeSql(sql);
  }

  static Value executeExecSp(const Token& term,
                             const std::vector<Value>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.ExecSp);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.ExecSp);

    std::string spName = get_string(term, args.at(1));

    if (!std::holds_alternative<std::shared_ptr<List>>(args.at(2))) {
      throw DbError(
          term, "Stored procedure parameters must be a List type in builtin `" +
                    OdbcBuiltins.ExecSp + "`.");
    }

    return OdbcConnection::getInstance(conn.connectionString)
        .executeStoredProcedure(spName,
                                std::get<std::shared_ptr<List>>(args.at(2)));
  }

  static Value executeIsConnected(const Token& term,
                                  const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.IsConnected);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.IsConnected);

    return OdbcConnection::getInstance(conn.connectionString).isConnected();
  }

  static Value executeInTransaction(const Token& term,
                                    const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.InTransaction);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.InTransaction);

    return OdbcConnection::getInstance(conn.connectionString).isInTransaction();
  }

  static Value executeBeginTransaction(const Token& term,
                                       const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.BeginTransaction);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.BeginTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .beginTransaction();
  }

  static Value executeCommitTransaction(const Token& term,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           OdbcBuiltins.CommitTransaction);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.CommitTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .commitTransaction();
  }

  static Value executeRollbackTransaction(const Token& term,
                                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           OdbcBuiltins.RollbackTransaction);
    }

    ConnectionInfo conn = ConnectionInfo::build(
        term, args.at(0), OdbcBuiltins.RollbackTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .rollbackTransaction();
  }
};

#endif
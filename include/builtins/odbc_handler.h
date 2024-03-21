#ifndef KIWI_BUILTINS_ODBCHANDLER_H
#define KIWI_BUILTINS_ODBCHANDLER_H

#include <vector>
#include "db/database.h"
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"

struct ConnectionInfo {
  k_string connectionString;
  bool hasConnection = false;

  static ConnectionInfo build(const Token& term, const k_value& value,
                              const k_string& builtin) {
    ConnectionInfo conn;
    k_hash hash;

    if (!std::holds_alternative<k_hash>(value)) {
      throw DbConnectionError(term,
                              "Expected a Hash with a `connection_string` key "
                              "in call to builtin `" +
                                  builtin + "`.");
    }

    hash = std::get<k_hash>(value);

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
  static k_value execute(const Token& term, const k_string& builtin,
                       const std::vector<k_value>& args) {
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
      throw DbError(term, "ODBC handler error: " + k_string(e.what()));
    }

    throw UnknownBuiltinError(term, builtin);
  }

 private:
  static k_value executeConnect(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.Connect);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.Connect);

    return OdbcConnection::getInstance(conn.connectionString).connect();
    ;
  }

  static k_value executeExec(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.Exec);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.Exec);

    k_string sql = get_string(term, args.at(1));
    return OdbcConnection::getInstance(conn.connectionString).executeSql(sql);
  }

  static k_value executeExecSp(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.ExecSp);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.ExecSp);

    k_string spName = get_string(term, args.at(1));

    if (!std::holds_alternative<k_list>(args.at(2))) {
      throw DbError(
          term, "Stored procedure parameters must be a List type in builtin `" +
                    OdbcBuiltins.ExecSp + "`.");
    }

    return OdbcConnection::getInstance(conn.connectionString)
        .executeStoredProcedure(spName,
                                std::get<k_list>(args.at(2)));
  }

  static k_value executeIsConnected(const Token& term,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.IsConnected);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.IsConnected);

    return OdbcConnection::getInstance(conn.connectionString).isConnected();
  }

  static k_value executeInTransaction(const Token& term,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.InTransaction);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.InTransaction);

    return OdbcConnection::getInstance(conn.connectionString).isInTransaction();
  }

  static k_value executeBeginTransaction(const Token& term,
                                       const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, OdbcBuiltins.BeginTransaction);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.BeginTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .beginTransaction();
  }

  static k_value executeCommitTransaction(const Token& term,
                                        const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           OdbcBuiltins.CommitTransaction);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(term, args.at(0), OdbcBuiltins.CommitTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .commitTransaction();
  }

  static k_value executeRollbackTransaction(const Token& term,
                                          const std::vector<k_value>& args) {
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
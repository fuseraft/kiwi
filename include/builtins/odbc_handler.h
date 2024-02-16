#ifndef KIWI_BUILTINS_ODBCHANDLER_H
#define KIWI_BUILTINS_ODBCHANDLER_H

#include <vector>
#include "db/database.h"
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

struct ConnectionInfo {
  std::string connectionString;
  bool hasConnection = false;

  static ConnectionInfo build(const Token& tokenTerm, const Value& value,
                              const std::string& builtin) {
    ConnectionInfo conn;
    std::shared_ptr<Hash> hash;

    if (!std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      throw DbConnectionError(tokenTerm,
                              "Expected a Hash with a `connection_string` key "
                              "in call to builtin `" +
                                  builtin + "`.");
    }

    hash = std::get<std::shared_ptr<Hash>>(value);

    if (hash->hasKey("connection_string")) {
      conn.connectionString =
          get_string(tokenTerm, hash->get("connection_string"));
      conn.hasConnection = true;
    }

    if (!conn.hasConnection) {
      throw DbConnectionError(tokenTerm,
                              "Connection hash does not contain a "
                              "`connection_string` key in call to builtin `" +
                                  builtin + "`.");
    }

    if (conn.connectionString.empty()) {
      throw DbConnectionError(
          tokenTerm,
          "Connection string is empty in call to builtin `" + builtin + "`.");
    }

    return conn;
  }
};

class OdbcBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    try {
      if (builtin == OdbcBuiltins.Connect) {
        return executeConnect(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.Exec) {
        return executeExec(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.ExecSp) {
        return executeExecSp(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.IsConnected) {
        return executeIsConnected(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.InTransaction) {
        return executeInTransaction(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.BeginTransaction) {
        return executeBeginTransaction(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.CommitTransaction) {
        return executeCommitTransaction(tokenTerm, args);
      } else if (builtin == OdbcBuiltins.RollbackTransaction) {
        return executeRollbackTransaction(tokenTerm, args);
      }
    } catch (const std::exception& e) {
      throw DbError(tokenTerm, "ODBC handler error: " + std::string(e.what()));
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeConnect(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, OdbcBuiltins.Connect);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(tokenTerm, args.at(0), OdbcBuiltins.Connect);

    return OdbcConnection::getInstance(conn.connectionString).connect();
    ;
  }

  static Value executeExec(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, OdbcBuiltins.Exec);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(tokenTerm, args.at(0), OdbcBuiltins.Exec);

    std::string sql = get_string(tokenTerm, args.at(1));
    return OdbcConnection::getInstance(conn.connectionString).executeSql(sql);
  }

  static Value executeExecSp(const Token& tokenTerm,
                             const std::vector<Value>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, OdbcBuiltins.ExecSp);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(tokenTerm, args.at(0), OdbcBuiltins.ExecSp);

    std::string spName = get_string(tokenTerm, args.at(1));

    if (!std::holds_alternative<std::shared_ptr<List>>(args.at(2))) {
      throw DbError(
          tokenTerm,
          "Stored procedure parameters must be a List type in builtin `" +
              OdbcBuiltins.ExecSp + "`.");
    }

    return OdbcConnection::getInstance(conn.connectionString)
        .executeStoredProcedure(spName,
                                std::get<std::shared_ptr<List>>(args.at(2)));
  }

  static Value executeIsConnected(const Token& tokenTerm,
                                  const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, OdbcBuiltins.IsConnected);
    }

    ConnectionInfo conn =
        ConnectionInfo::build(tokenTerm, args.at(0), OdbcBuiltins.IsConnected);

    return OdbcConnection::getInstance(conn.connectionString).isConnected();
  }

  static Value executeInTransaction(const Token& tokenTerm,
                                    const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           OdbcBuiltins.InTransaction);
    }

    ConnectionInfo conn = ConnectionInfo::build(tokenTerm, args.at(0),
                                                OdbcBuiltins.InTransaction);

    return OdbcConnection::getInstance(conn.connectionString).isInTransaction();
  }

  static Value executeBeginTransaction(const Token& tokenTerm,
                                       const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           OdbcBuiltins.BeginTransaction);
    }

    ConnectionInfo conn = ConnectionInfo::build(tokenTerm, args.at(0),
                                                OdbcBuiltins.BeginTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .beginTransaction();
  }

  static Value executeCommitTransaction(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           OdbcBuiltins.CommitTransaction);
    }

    ConnectionInfo conn = ConnectionInfo::build(tokenTerm, args.at(0),
                                                OdbcBuiltins.CommitTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .commitTransaction();
  }

  static Value executeRollbackTransaction(const Token& tokenTerm,
                                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           OdbcBuiltins.RollbackTransaction);
    }

    ConnectionInfo conn = ConnectionInfo::build(
        tokenTerm, args.at(0), OdbcBuiltins.RollbackTransaction);

    return OdbcConnection::getInstance(conn.connectionString)
        .rollbackTransaction();
  }
};

#endif
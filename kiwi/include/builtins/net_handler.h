#ifndef KIWI_BUILTINS_NETHANDLER_H
#define KIWI_BUILTINS_NETHANDLER_H

#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "net/socketmanager.h"
#include "globals.h"

class NetBuiltinHandler {
 public:
  static k_value execute(SocketManager& sockmgr, const Token& token,
                         const KName& builtin,
                         const std::vector<k_value>& args) {
    if (SAFEMODE) {
      return static_cast<k_int>(0);
    }

    switch (builtin) {
      case KName::Builtin_Socket_Accept:
        return executeAccept(sockmgr, token, args);
      case KName::Builtin_Socket_Bind:
        return executeBind(sockmgr, token, args);
      case KName::Builtin_Socket_Close:
        return executeClose(sockmgr, token, args);
      case KName::Builtin_Socket_Connect:
        return executeConnect(sockmgr, token, args);
      case KName::Builtin_Socket_Create:
        return executeCreate(sockmgr, token, args);
      case KName::Builtin_Socket_Listen:
        return executeListen(sockmgr, token, args);
      case KName::Builtin_Socket_Receive:
        return executeReceive(sockmgr, token, args);
      case KName::Builtin_Socket_Send:
        return executeSend(sockmgr, token, args);
      case KName::Builtin_Socket_Shutdown:
        return executeShutdown(sockmgr, token, args);
      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeCreate(SocketManager& sockmgr, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Create);
    }

    auto family = get_integer(token, args.at(0));
    auto type = get_integer(token, args.at(1));
    auto protocol = get_integer(token, args.at(2));

    return sockmgr.create_socket(token, family, type, protocol);
  }

  static k_value executeBind(SocketManager& sockmgr, const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Bind);
    }

    auto sockId = get_integer(token, args.at(0));
    auto address = get_string(token, args.at(1));
    auto port = get_integer(token, args.at(2));

    return sockmgr.bind(token, sockId, address, port);
  }

  static k_value executeListen(SocketManager& sockmgr, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Listen);
    }

    auto sockId = get_integer(token, args.at(0));
    auto backlog = get_integer(token, args.at(1));

    return sockmgr.listen(token, sockId, backlog);
  }

  static k_value executeAccept(SocketManager& sockmgr, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Accept);
    }

    auto sockId = get_integer(token, args.at(0));
    k_string client_address;
    k_int client_port;

    auto client_sock_id =
        sockmgr.accept(token, sockId, client_address, client_port);

    auto hash = std::make_shared<Hashmap>();
    hash->add("client_sock_id", client_sock_id);
    hash->add("client_address", client_address);
    hash->add("client_port", client_port);
    return hash;
  }

  static k_value executeConnect(SocketManager& sockmgr, const Token& token,
                                const std::vector<k_value>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Connect);
    }

    auto sockId = get_integer(token, args.at(0));
    auto address = get_string(token, args.at(1));
    auto port = get_integer(token, args.at(2));

    return sockmgr.connect(token, sockId, address, port);
  }

  static k_value executeSend(SocketManager& sockmgr, const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Send);
    }

    auto sockId = get_integer(token, args.at(0));

    return sockmgr.send(token, sockId, args.at(1));
  }

  static k_value executeReceive(SocketManager& sockmgr, const Token& token,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Receive);
    }

    auto sockId = get_integer(token, args.at(0));
    auto length = get_integer(token, args.at(1));

    return sockmgr.receive(token, sockId, length);
  }

  static k_value executeClose(SocketManager& sockmgr, const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Close);
    }

    auto sockId = get_integer(token, args.at(0));

    return sockmgr.close(token, sockId);
  }

  static k_value executeShutdown(SocketManager& sockmgr, const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Shutdown);
    }

    auto sockId = get_integer(token, args.at(0));
    auto how = get_integer(token, args.at(1));

    return sockmgr.shutdown(token, sockId, how);
  }
};

#endif
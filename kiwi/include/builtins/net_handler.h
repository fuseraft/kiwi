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
  static KValue execute(SocketManager& sockmgr, const Token& token,
                        const KName& builtin, const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
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
      case KName::Builtin_Socket_SendRaw:
        return executeSendRaw(sockmgr, token, args);
      case KName::Builtin_Socket_Shutdown:
        return executeShutdown(sockmgr, token, args);
      case KName::Builtin_Net_IsIPAddr:
        return executeIsIPAddr(sockmgr, token, args);
      case KName::Builtin_Net_ResolveHost:
        return executeResHost(sockmgr, token, args);
      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static KValue executeIsIPAddr(SocketManager& sockmgr, const Token& token,
                                const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.IsIPAddr);
    }

    auto ipAddress = get_string(token, args.at(0));
    auto family = 0;
    return KValue::createBoolean(sockmgr.isIPAddress(ipAddress, family));
  }

  static KValue executeResHost(SocketManager& sockmgr, const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.ResolveHost);
    }

    auto hostname = get_string(token, args.at(0));
    return sockmgr.resolveHostToIP(hostname);
  }

  static KValue executeCreate(SocketManager& sockmgr, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Create);
    }

    auto family = get_integer(token, args.at(0));
    auto type = get_integer(token, args.at(1));
    auto protocol = get_integer(token, args.at(2));

    return sockmgr.create(token, family, type, protocol);
  }

  static KValue executeBind(SocketManager& sockmgr, const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Bind);
    }

    auto sockId = get_integer(token, args.at(0));
    auto address = get_string(token, args.at(1));
    auto port = get_integer(token, args.at(2));

    return KValue::createBoolean(sockmgr.bind(token, sockId, address, port));
  }

  static KValue executeListen(SocketManager& sockmgr, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Listen);
    }

    auto sockId = get_integer(token, args.at(0));
    auto backlog = get_integer(token, args.at(1));

    return KValue::createBoolean(sockmgr.listen(token, sockId, backlog));
  }

  static KValue executeAccept(SocketManager& sockmgr, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Accept);
    }

    auto sockId = get_integer(token, args.at(0));
    k_string client_address;
    k_int client_port;

    auto client_sock_id =
        sockmgr.accept(token, sockId, client_address, client_port);

    auto hash = std::make_shared<Hashmap>();
    hash->add(KValue::createString("client_sock_id"), client_sock_id);
    hash->add(KValue::createString("client_address"),
              KValue::createString(client_address));
    hash->add(KValue::createString("client_port"),
              KValue::createInteger(client_port));
    return KValue::createHashmap(hash);
  }

  static KValue executeConnect(SocketManager& sockmgr, const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Connect);
    }

    auto sockId = get_integer(token, args.at(0));
    auto address = get_string(token, args.at(1));
    auto port = get_integer(token, args.at(2));

    return KValue::createBoolean(sockmgr.connect(token, sockId, address, port));
  }

  static KValue executeSend(SocketManager& sockmgr, const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Send);
    }

    auto sockId = get_integer(token, args.at(0));

    return sockmgr.send(token, sockId, args.at(1));
  }

  static KValue executeSendRaw(SocketManager& sockmgr, const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.SendRaw);
    }

    auto sockId = get_integer(token, args.at(0));
    auto destination = get_string(token, args.at(1));

    return sockmgr.sendRawPacket(token, sockId, destination, args.at(2));
  }

  static KValue executeReceive(SocketManager& sockmgr, const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Receive);
    }

    auto sockId = get_integer(token, args.at(0));
    auto length = get_integer(token, args.at(1));

    return sockmgr.receive(token, sockId, length);
  }

  static KValue executeClose(SocketManager& sockmgr, const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Close);
    }

    auto sockId = get_integer(token, args.at(0));

    return KValue::createBoolean(sockmgr.close(token, sockId));
  }

  static KValue executeShutdown(SocketManager& sockmgr, const Token& token,
                                const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, SocketBuiltins.Shutdown);
    }

    auto sockId = get_integer(token, args.at(0));
    auto how = get_integer(token, args.at(1));

    return KValue::createBoolean(sockmgr.shutdown(token, sockId, how));
  }
};

#endif
#ifndef KIWI_ENGINE_H
#define KIWI_ENGINE_H

#include <unordered_map>
#include <stack>
#include "concurrency/task.h"
#include "logging/logger.h"
#include "objects/class.h"
#include "objects/conditional.h"
#include "objects/method.h"
#include "objects/package.h"
#include "objects/sliceindex.h"
#include "parsing/builtins.h"
#include "parsing/lexer.h"
#include "parsing/parser.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"
#include "parsing/keywords.h"
#include "tracing/error.h"
#include "tracing/handler.h"
#include "tracing/state.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include "util/file.h"
#include "util/string.h"
#include "globals.h"
#include "builtin.h"
#include "interp_helper.h"
#include "stackframe.h"

#include "interpreter.h"

class Engine {
 public:
  Engine() {}
  ~Engine() {}

  void setKiwiArgs(const std::unordered_map<k_string, k_string>& args) {
    kiwiArgs = args;
  }

  int runStreamCollection() {
    auto ast = parser.parseTokenStreamCollection(streamCollection);
    auto result = interp.interpret(ast.get());
    if (std::holds_alternative<k_int>(result)) {
      return static_cast<int>(std::get<k_int>(result));
    }
    return 0;
  }

  int interpretKiwi(const k_string& kiwiCode) {
    Lexer lexer("", kiwiCode);
    auto tokenStream = lexer.getTokenStream();
    auto ast = parser.parseTokenStream(tokenStream);
    auto result = interp.interpret(ast.get());

    if (std::holds_alternative<k_int>(result)) {
      return static_cast<int>(std::get<k_int>(result));
    }

    return 0;
  }

  int interpretScript(const k_string& path) {
    auto content = File::readFile(path);
    if (content.empty()) {
      return -1;
    }

    Lexer lexer(path, content);

    auto tokenStream = lexer.getTokenStream();
    streamCollection.push_back(tokenStream);

    return 0;
  }

  void preserveMainStackFrame() { preservingMainStackFrame = true; }

 private:
  Parser parser;
  KInterpreter interp;
  bool preservingMainStackFrame = false;
  std::vector<k_stream> streamCollection;

  /* 
  void handleWebServerRequest(int webhookID, k_hash requestHash,
                              k_string& redirect, k_string& content,
                              k_string& contentType, int& status) {
    auto webhook = kiwiWebServerHooks[webhookID];
    auto webhookFrame = std::make_shared<CallStackFrame>();

    for (const auto& param : webhook.getParameters()) {
      webhookFrame->variables[param] = requestHash;
      break;
    }

    auto webhookStream = std::make_shared<TokenStream>(webhook.getCode());
    callStack.push(webhookFrame);
    streamStack.push(webhookStream);

    interpretStackFrame();

    if (!callStack.empty()) {
      auto retValue = callStack.top()->returnValue;
      if (std::holds_alternative<k_hash>(retValue)) {

        auto responseHash = std::get<k_hash>(retValue);
        if (responseHash->hasKey("content")) {
          auto responseHashContent = responseHash->get("content");
          content = Serializer::serialize(responseHashContent);
        }

        if (responseHash->hasKey("content-type")) {
          auto responseHashContent = responseHash->get("content-type");
          if (std::holds_alternative<k_string>(responseHashContent)) {
            contentType = std::get<k_string>(responseHashContent);
          }
        }

        if (responseHash->hasKey("status")) {
          auto responseHashContent = responseHash->get("status");
          if (std::holds_alternative<k_int>(responseHashContent)) {
            status = static_cast<int>(std::get<k_int>(responseHashContent));
          }
        }

        if (responseHash->hasKey("redirect")) {
          auto responseHashContent = responseHash->get("redirect");
          if (std::holds_alternative<k_string>(responseHashContent)) {
            redirect = std::get<k_string>(responseHashContent);
          }
        }
      }
    }
  }

  std::vector<k_string> getWebServerEndpointList(const Token& term,
                                                 k_value& arg) {
    std::vector<k_string> endpointList;

    if (std::holds_alternative<k_string>(arg)) {
      endpointList.emplace_back(get_string(term, arg));
    } else if (std::holds_alternative<k_list>(arg)) {
      for (const auto& el : std::get<k_list>(arg)->elements) {
        if (std::holds_alternative<k_string>(el)) {
          auto endpoint = get_string(term, el);
          if (std::find(endpointList.begin(), endpointList.end(), endpoint) ==
              endpointList.end()) {
            endpointList.emplace_back(endpoint);
          }
        }
      }
    }

    return endpointList;
  }

  int getNextWebServerHook(k_stream stream,
                           std::shared_ptr<CallStackFrame> frame,
                           k_value& arg) {
    if (!std::holds_alternative<k_lambda>(arg)) {
      throw InvalidOperationError(stream->current(),
                                  "Expected lambda for second parameter of `" +
                                      WebServerBuiltins.Get + "`.");
    }

    auto lambdaName = std::get<k_lambda>(arg)->identifier;
    auto method = getMethod(stream, frame, lambdaName);
    int webhookID = 0;

    if (!kiwiWebServerHooks.empty()) {
      webhookID = static_cast<int>(kiwiWebServerHooks.size());
    }

    kiwiWebServerHooks[webhookID] = std::move(method);
    return webhookID;
  }

  k_hash getWebServerRequestHash(const httplib::Request& req) {
    auto requestHash = std::make_shared<Hash>();
    auto headers = req.headers;
    auto params = req.params;

    for (auto it = headers.begin(); it != headers.end(); ++it) {
      const auto& x = *it;
      requestHash->add(x.first, x.second);
    }

    auto pathParamsHash = std::make_shared<Hash>();
    for (const auto& pair : req.path_params) {
      pathParamsHash->add(pair.first, pair.second);
    }

    auto paramsHash = std::make_shared<Hash>();
    for (auto it = params.begin(); it != params.end(); ++it) {
      const auto& x = *it;
      paramsHash->add(x.first, x.second);
    }

    auto filesHash = std::make_shared<Hash>();

    for (const auto& file : req.files) {
      auto fileHash = std::make_shared<Hash>();
      fileHash->add("content", file.second.content);
      fileHash->add("content_type", file.second.content_type);
      fileHash->add("filename", file.second.filename);
      fileHash->add("name", file.second.name);
      filesHash->add(file.first, fileHash);
    }

    requestHash->add("body", req.body);
    requestHash->add("files", filesHash);
    requestHash->add("path", req.path);
    requestHash->add("path_params", pathParamsHash);
    requestHash->add("params", paramsHash);

    return requestHash;
  }

  k_value interpretWebServerGet(k_stream stream,
                                std::shared_ptr<CallStackFrame> frame,
                                std::vector<k_value>& args) {
    auto term = stream->current();

    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, WebServerBuiltins.Get);
    }

    auto endpointList = getWebServerEndpointList(term, args.at(0));
    int webhookID = getNextWebServerHook(stream, frame, args.at(1));

    for (const auto& endpoint : endpointList) {
      kiwiWebServer.Get(endpoint, [this, webhookID](const httplib::Request& req,
                                                    httplib::Response& res) {
        auto requestHash = getWebServerRequestHash(req);

        k_string content, redirect;
        k_string contentType = "text/plain";
        int status = 500;
        handleWebServerRequest(webhookID, requestHash, redirect, content,
                               contentType, status);

        res.status = status;
        res.set_content(content, contentType);
      });
    }

    return static_cast<k_int>(0);
  }

  k_value interpretWebServerPost(k_stream stream,
                                 std::shared_ptr<CallStackFrame> frame,
                                 std::vector<k_value>& args) {
    auto term = stream->current();

    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, WebServerBuiltins.Get);
    }

    auto endpointList = getWebServerEndpointList(term, args.at(0));
    int webhookID = getNextWebServerHook(stream, frame, args.at(1));

    for (const auto& endpoint : endpointList) {
      kiwiWebServer.Post(
          endpoint, [this, webhookID](const httplib::Request& req,
                                      httplib::Response& res) {
            auto requestHash = getWebServerRequestHash(req);

            k_string content, redirect;
            k_string contentType = "text/plain";
            int status = 500;
            handleWebServerRequest(webhookID, requestHash, redirect, content,
                                   contentType, status);

            if (!redirect.empty()) {
              res.set_redirect(redirect);
            } else {
              res.status = status;
              res.set_content(content, contentType);
            }
          });
    }

    return static_cast<k_int>(0);
  }

  k_value interpretWebServerListen(k_stream stream,
                                   const std::vector<k_value>& args) {
    auto term = stream->current();

    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, WebServerBuiltins.Listen);
    }

    kiwiWebServerHost = get_string(term, args.at(0));
    kiwiWebServerPort = get_integer(term, args.at(1));

    kiwiWebServer.listen(kiwiWebServerHost,
                         static_cast<int>(kiwiWebServerPort));

    return static_cast<k_int>(kiwiWebServerPort);
  }

  k_value interpretWebServerPort(k_stream stream,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(stream->current(),
                                           WebServerBuiltins.Listen);
    }

    return kiwiWebServerPort;
  }

  k_value interpretWebServerPublic(k_stream stream,
                                   const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(stream->current(),
                                           WebServerBuiltins.Public);
    }

    auto endpoint = get_string(stream->current(), args.at(0));
    auto publicDir = get_string(stream->current(), args.at(1));

    if (!File::directoryExists(publicDir)) {
      return false;
    }

    kiwiWebServer.set_mount_point(endpoint, publicDir);

    return true;
  }

  k_value interpretWebServerHost(k_stream stream,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(stream->current(),
                                           WebServerBuiltins.Listen);
    }

    return kiwiWebServerHost;
  }

  k_value interpretWebServerBuiltin(k_stream stream,
                                    std::shared_ptr<CallStackFrame> frame,
                                    const KName& builtin,
                                    std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_WebServer_Get:
        return interpretWebServerGet(stream, frame, args);

      case KName::Builtin_WebServer_Post:
        return interpretWebServerPost(stream, frame, args);

      case KName::Builtin_WebServer_Listen:
        return interpretWebServerListen(stream, args);

      case KName::Builtin_WebServer_Host:
        return interpretWebServerHost(stream, args);

      case KName::Builtin_WebServer_Port:
        return interpretWebServerPort(stream, args);

      case KName::Builtin_WebServer_Public:
        return interpretWebServerPublic(stream, args);

      default:
        break;
    }

    return static_cast<k_int>(0);
  }

  k_value interpretSerializerDeserialize(k_stream stream,
                                         std::shared_ptr<CallStackFrame> frame,
                                         std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(stream->current(),
                                           SerializerBuiltins.Deserialize);
    }

    return interpolateString(frame, get_string(stream->current(), args.at(0)));
  }

  k_value interpretSerializerSerialize(k_stream stream,
                                       std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(stream->current(),
                                           SerializerBuiltins.Serialize);
    }

    return Serializer::serialize(args.at(0), true);
  }

  k_value interpretSerializerBuiltin(k_stream stream,
                                     std::shared_ptr<CallStackFrame> frame,
                                     const KName& builtin,
                                     std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Serializer_Deserialize:
        return interpretSerializerDeserialize(stream, frame, args);

      case KName::Builtin_Serializer_Serialize:
        return interpretSerializerSerialize(stream, args);

      default:
        break;
    }

    return static_cast<k_int>(0);
  }

  k_value interpretBuiltin(k_stream stream,
                           std::shared_ptr<CallStackFrame> frame,
                           const KName& builtin) {
    auto term = stream->current();
    stream->next();  // Skip the name.

    auto args = interpretArguments(stream, frame);

    if (PackageBuiltins.is_builtin(builtin)) {
      if (packageStack.empty()) {
        throw InvalidContextError(term, "Expected a package context.");
      }
      auto packageName = packageStack.top();
      interpretPackageBuiltin(stream, packageName, builtin, args);
      return static_cast<k_int>(0);
    } else if (WebServerBuiltins.is_builtin(builtin)) {
      return interpretWebServerBuiltin(stream, frame, builtin, args);
    } else if (SerializerBuiltins.is_builtin(builtin)) {
      return interpretSerializerBuiltin(stream, frame, builtin, args);
    }

    frame->returnValue =
        BuiltinDispatch::execute(term, builtin, args, kiwiArgs);
    return frame->returnValue;
  }

  k_string interpolateObject(k_stream stream,
                             std::shared_ptr<CallStackFrame> frame,
                             k_value& value) {
    auto object = std::get<k_object>(value);
    auto clazz = classes[object->className];

    if (!clazz.hasMethod(KiwiBuiltins.ToS)) {
      return Serializer::basic_serialize_object(object);
    }

    std::vector<k_value> parameters;

    // Should probably check that an overridden to_string() actually returns a string.
    return Serializer::serialize(interpretInstanceMethodInvocation(
        stream, frame, object, KiwiBuiltins.ToS, KName::Builtin_Kiwi_ToS,
        parameters));
  }

  k_value interpolateString(std::shared_ptr<CallStackFrame> frame,
                            const k_string& input) {
    Lexer lexer("", input);
    auto tempStream = std::make_shared<TokenStream>(lexer.getAllTokens());
    auto tempFrame = buildSubFrame(frame);

    return parseExpression(tempStream, tempFrame);
  }

  k_string interpolateString(k_stream stream,
                             std::shared_ptr<CallStackFrame> frame) {
    auto input = stream->current().getText();
    std::ostringstream sv;

    for (size_t i = 0; i < input.length(); ++i) {
      char c = input[i];

      if (c == '$' && i + 1 < input.length() && input[i + 1] == '{') {
        i += 2;  // Skip "${"
        size_t start = i;
        int braceCount = 1;
        while (i < input.length() && braceCount > 0) {
          if (input[i] == '{') {
            ++braceCount;
          } else if (input[i] == '}') {
            --braceCount;
          }
          ++i;
        }

        if (braceCount != 0) {
          throw SyntaxError(
              stream->current(),
              "Unmatched braces in string interpolation: `" + input + "`");
        }

        --i;  // Go back to the closing brace
        auto value = interpolateString(frame, input.substr(start, i - start));
        if (!std::holds_alternative<k_object>(value)) {
          sv << Serializer::serialize(value);
        } else {
          sv << interpolateObject(stream, frame, value);
        }
      } else if (c == '\\') {
        // Handle escape sequences
        if (i + 1 < input.length()) {
          switch (input[i + 1]) {
            case 't':
              sv << '\t';
              break;
            case 'n':
              sv << '\n';
              break;
            case 'r':
              sv << '\r';
              break;
            case 'b':
              sv << '\b';
              break;
            case 'f':
              sv << '\f';
              break;
            case '\\':
              sv << '\\';
              break;

            default:
              sv << input[i + 1];
              break;
          }
          i++;
        }
      } else {
        sv << c;
      }
    }

    return sv.str();
  }

  k_string mangleString(k_stream stream, const k_string& input,
                        std::unordered_map<k_string, k_string>& mangledNames) {
    std::ostringstream sv;

    for (size_t i = 0; i < input.length(); ++i) {
      char c = input[i];

      if (c == '$' && i + 1 < input.length() && input[i + 1] == '{') {
        i += 2;  // Skip "${"
        size_t start = i;
        int braceCount = 1;
        while (i < input.length() && braceCount > 0) {
          if (input[i] == '{') {
            ++braceCount;
          } else if (input[i] == '}') {
            --braceCount;
          }
          ++i;
        }

        if (braceCount != 0) {
          throw SyntaxError(
              stream->current(),
              "Unmatched braces in string interpolation: `" + input + "`");
        }

        --i;  // Go back to the closing brace

        Lexer lexer("", input.substr(start, i - start));
        std::ostringstream mangler;
        for (const auto& token : lexer.getAllTokens()) {
          auto tokenText = token.getText();
          if (token.getType() == KTokenType::IDENTIFIER) {
            if (mangledNames.find(tokenText) != mangledNames.end()) {
              mangler << mangledNames[tokenText];
            } else {
              mangler << tokenText;
            }
          } else {
            mangler << tokenText;
          }
        }

        sv << mangler.str();
      } else if (c == '\\') {
        // Handle escape sequences
        if (i + 1 < input.length()) {
          switch (input[i + 1]) {
            case 't':
              sv << '\t';
              break;
            case 'n':
              sv << '\n';
              break;
            case 'r':
              sv << '\r';
              break;
            case 'b':
              sv << '\b';
              break;
            case 'f':
              sv << '\f';
              break;
            case '\\':
              sv << '\\';
              break;

            default:
              sv << input[i + 1];
              break;
          }
          i++;
        }
      } else {
        sv << c;
      }
    }

    return sv.str();
  }*/
};

#endif

#ifndef KIWI_CONTEXT_H
#define KIWI_CONTEXT_H

#include <unordered_map>
#include "callable.h"
#include "typing/value.h"
#include "web/httplib.h"

class KContext {
 private:
  std::unordered_map<k_string, std::unique_ptr<KPackage>> packages;
  std::unordered_map<k_string, std::unique_ptr<KFunction>> functions;
  std::unordered_map<k_string, std::unique_ptr<KFunction>> methods;
  std::unordered_map<k_string, std::unique_ptr<KLambda>> lambdas;
  std::unordered_map<k_string, std::unique_ptr<KStruct>> structs;
  std::unordered_map<k_string, k_string> lambdaTable;
  std::unordered_map<k_string, KValue> constants;
  httplib::Server server;
  std::unordered_map<int, k_string> serverHooks;

 public:
  KContext()
      : packages(),
        functions(),
        methods(),
        lambdas(),
        structs(),
        lambdaTable(),
        constants(),
        server(),
        serverHooks() {}

  std::unique_ptr<KContext> clone() {
    auto cloned = std::make_unique<KContext>();

    for (const auto& pair : packages) {
      cloned->addPackage(pair.first, pair.second->clone());
    }

    for (const auto& pair : functions) {
      cloned->addFunction(pair.first, pair.second->clone());
    }

    for (const auto& pair : methods) {
      cloned->addMethod(pair.first, pair.second->clone());
    }

    for (const auto& pair : lambdas) {
      cloned->addLambda(pair.first, pair.second->clone());
    }

    for (const auto& pair : structs) {
      cloned->addStruct(pair.first, pair.second->clone());
    }

    for (const auto& pair : constants) {
      cloned->constants[pair.first] = pair.second;
    }

    cloned->lambdaTable = lambdaTable;
    cloned->serverHooks = serverHooks;

    return cloned;
  }

  bool hasConstant(const k_string& name) const {
    return constants.find(name) != constants.end();
  }

  void addConstant(const k_string& name, const KValue& value) {
    constants[name] = clone_value(value);
  }

  std::unordered_map<k_string, KValue>& getConstants() { return constants; }

  bool hasPackage(const k_string& name) const {
    return packages.find(name) != packages.end();
  }

  void addPackage(const k_string& name, std::unique_ptr<KPackage> package) {
    packages[name] = std::move(package);
  }

  std::unordered_map<k_string, std::unique_ptr<KPackage>>& getPackages() {
    return packages;
  }

  bool hasFunction(const k_string& name) const {
    return functions.find(name) != functions.end();
  }

  void addFunction(const k_string& name, std::unique_ptr<KFunction> function) {
    functions[name] = std::move(function);
  }

  std::unordered_map<k_string, std::unique_ptr<KFunction>>& getFunctions() {
    return functions;
  }

  bool hasMethod(const k_string& name) const {
    return methods.find(name) != methods.end();
  }

  void addMethod(const k_string& name, std::unique_ptr<KFunction> method) {
    methods[name] = std::move(method);
  }

  std::unordered_map<k_string, std::unique_ptr<KFunction>>& getMethods() {
    return methods;
  }

  bool hasLambda(const k_string& name) const {
    return lambdas.find(name) != lambdas.end();
  }

  void addLambda(const k_string& name, std::unique_ptr<KLambda> lambda) {
    lambdas[name] = std::move(lambda);
  }

  std::unordered_map<k_string, std::unique_ptr<KLambda>>& getLambdas() {
    return lambdas;
  }

  bool hasStruct(const k_string& name) const {
    return structs.find(name) != structs.end();
  }

  void addStruct(const k_string& name, std::unique_ptr<KStruct> struc) {
    structs[name] = std::move(struc);
  }

  const std::unordered_map<k_string, std::unique_ptr<KStruct>>& getStructs()
      const {
    return structs;
  }

  bool hasMappedLambda(const k_string& name) const {
    return lambdaTable.find(name) != lambdaTable.end();
  }

  void addMappedLambda(const k_string& name, const k_string& mappedName) {
    lambdaTable[name] = mappedName;
  }

  const k_string& getMappedLambda(const k_string& name) {
    return lambdaTable.at(name);
  }

  void addWebHook(const k_int& webhookId, const k_string& lambda) {
    serverHooks[webhookId] = lambda;
  }

  const k_string& getWebHook(const k_int& webhookId) {
    return serverHooks[webhookId];
  }

  std::unordered_map<int, k_string>& getWebHooks() { return serverHooks; }

  httplib::Server& getServer() { return server; }
};

#endif
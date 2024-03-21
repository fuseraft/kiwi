#ifndef KIWI_NET_HTTPCLIENT_H
#define KIWI_NET_HTTPCLIENT_H

#include <iostream>
#include <curl/curl.h>
#include <list>
#include <mutex>
#include <variant>
#include "typing/value.h"

enum HttpMethod { GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS };

class HttpClient {
 public:
  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  static HttpClient& getInstance() {
    static HttpClient instance;
    return instance;
  }

  std::shared_ptr<Hash> get(const k_string& url,
                            const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::GET, "", headers);
  }

  std::shared_ptr<Hash> post(const k_string& url, const k_string& body,
                             const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::POST, body, headers);
  }

  std::shared_ptr<Hash> put(const k_string& url, const k_string& body,
                            const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::PUT, body, headers);
  }

  std::shared_ptr<Hash> patch(const k_string& url, const k_string& body,
                              const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::PATCH, body, headers);
  }

  std::shared_ptr<Hash> del(const k_string& url,
                            const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::DELETE, "", headers);
  }

  std::shared_ptr<Hash> head(const k_string& url,
                             const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::HEAD, "", headers);
  }

  std::shared_ptr<Hash> options(const k_string& url,
                                const std::shared_ptr<List>& headers = {}) {
    return performRequest(url, HttpMethod::OPTIONS, "", headers);
  }

 private:
  std::list<CURL*> pool;
  std::mutex poolMutex;

  HttpClient() { curl_global_init(CURL_GLOBAL_ALL); }

  ~HttpClient() {
    for (auto handle : pool) {
      curl_easy_cleanup(handle);
    }
    curl_global_cleanup();
  }

  static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                              k_string* data) {
    data->append((char*)contents, size * nmemb);
    return size * nmemb;
  }

  static size_t HeaderCallback(char* buffer, size_t size, size_t nitems,
                               k_string* data) {
    size_t realSize = nitems * size;
    data->append(buffer, realSize);
    return realSize;
  }

  CURL* acquireHandle() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (pool.empty()) {
      return curl_easy_init();
    } else {
      CURL* handle = pool.front();
      pool.pop_front();
      return handle;
    }
  }

  void releaseHandle(CURL* handle) {
    std::lock_guard<std::mutex> lock(poolMutex);
    curl_easy_reset(handle);  // Reset handle state
    pool.push_back(handle);
  }

  std::shared_ptr<Hash> performRequest(
      const k_string& url, const HttpMethod& method,
      const k_string& body = "", const std::shared_ptr<List>& headers = {}) {
    CURL* curl = acquireHandle();
    if (!curl) {
      auto response = std::make_shared<Hash>();
      response->add("error", "CURL initialization failed");
      response->add("status", 0);
      return response;
    }

    struct curl_slist* curlHeaders = nullptr;
    bool addHeaders = false;
    for (const auto& header : headers->elements) {
      if (std::holds_alternative<k_string>(header)) {
        curlHeaders = curl_slist_append(curlHeaders,
                                        std::get<k_string>(header).c_str());
        addHeaders = true;
      }
    }

    // Clearing existing options from previous uses
    curl_easy_reset(curl);

    k_string responseBody;
    k_string responseHeaders;
    long statusCode = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);

    if (addHeaders) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
    }

    if (method == HttpMethod::POST) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == HttpMethod::PUT || method == HttpMethod::PATCH) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,
                       method == HttpMethod::PUT ? "PUT" : "PATCH");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == HttpMethod::DELETE) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (method == HttpMethod::HEAD) {
      curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    } else if (method == HttpMethod::OPTIONS) {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    }

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

    auto response = std::make_shared<Hash>();
    if (res == CURLE_OK) {
      response->add("status", static_cast<int>(statusCode));
      response->add("body", responseBody);
      response->add("headers", responseHeaders);
    } else {
      response->add("status",
                    (statusCode > 0) ? static_cast<int>(statusCode) : -1);
      response->add("error", curl_easy_strerror(res));
      response->add("headers", responseHeaders);
    }

    releaseHandle(curl);
    return response;
  }
};

#endif
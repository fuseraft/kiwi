#ifndef KIWI_NET_HTTPCLIENT_H
#define KIWI_NET_HTTPCLIENT_H

#include <iostream>
#include <curl/curl.h>
#include <string>

class HttpClient {
 public:
  HttpClient() { curl_global_init(CURL_GLOBAL_ALL); }

  ~HttpClient() { curl_global_cleanup(); }

  std::string get(const std::string& url) {
    return performRequest(url, "GET", "");
  }

  std::string post(const std::string& url, const std::string& body) {
    return performRequest(url, "POST", body);
  }

 private:
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                              std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
  }

  std::string performRequest(const std::string& url, const std::string& method,
                             const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) {
      return "CURL initialization failed";
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (method == "POST") {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      curl_easy_cleanup(curl);
      return "CURL request failed: " + std::string(curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return response;
  }
};

#endif
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "HTTPClient.h"
#include "NetworkClient.h"
#include "WString.h"

namespace freeink {

class SecureHttpClient {
public:
  using DataCallback = std::function<bool(const uint8_t *data, size_t len)>;
  using AbortCallback = std::function<bool()>;

  void setCACert(const char *) {}
  void setInsecure() {}

  bool begin(const String &url) {
    http_.begin(client_, url.c_str());
    return !url.isEmpty();
  }

  bool begin(const std::string &url) { return begin(String(url)); }
  bool begin(const char *url) { return begin(String(url)); }

  void end() { http_.end(); }

  void addHeader(const char *name, const String &value) {
    http_.addHeader(name, value);
  }

  void addHeader(const char *name, const std::string &value) {
    http_.addHeader(name, value.c_str());
  }

  void addHeader(const char *name, const char *value) {
    http_.addHeader(name, value);
  }

  void setTimeout(uint16_t ms) { http_.setTimeout(ms); }
  void setReuse(bool reuse) { http_.setReuse(reuse); }
  // The simulator's HTTPClient has no redirect or User-Agent control.
  void setUserAgent(const std::string &) {}
  void setFollowRedirects(int) {}

  int GET() { return http_.GET(); }
  int POST(const String &payload) { return http_.POST(payload.c_str()); }
  int sendRequest(const char *method, const String &payload) {
    if (method && std::string(method) == "PUT") {
      return http_.PUT(payload);
    }
    if (method && std::string(method) == "POST") {
      return http_.POST(payload.c_str());
    }
    return http_.GET();
  }
  int sendRequest(const char *method, const std::string &payload) {
    return sendRequest(method, String(payload));
  }

  // Streaming variants: the simulator buffers the whole body, then hands it to
  // the callback in one piece. shouldAbort is polled once before delivery.
  int GET(const DataCallback &onData, const AbortCallback &shouldAbort = nullptr) {
    return deliver(http_.GET(), onData, shouldAbort);
  }
  int sendRequest(const char *method, const uint8_t *payload, size_t payloadLen,
                  const DataCallback &onData = nullptr,
                  const AbortCallback &shouldAbort = nullptr) {
    const std::string body(reinterpret_cast<const char *>(payload),
                           payload ? payloadLen : 0);
    return deliver(sendRequest(method, String(body)), onData, shouldAbort);
  }

  bool aborted() const { return aborted_; }
  bool callbackAborted() const { return callbackAborted_; }
  bool responseComplete() const { return responseComplete_; }

  String getString() { return http_.getString(); }
  int getSize() { return http_.getSize(); }

  static bool tls13Available() { return true; }

private:
  int deliver(int code, const DataCallback &onData, const AbortCallback &shouldAbort) {
    aborted_ = false;
    callbackAborted_ = false;
    responseComplete_ = false;
    if (code <= 0 || !onData) {
      return code;
    }
    if (shouldAbort && shouldAbort()) {
      aborted_ = true;
      return code;
    }
    const String body = http_.getString();
    if (onData(reinterpret_cast<const uint8_t *>(body.c_str()), body.length())) {
      responseComplete_ = true;
    } else {
      callbackAborted_ = true;
    }
    return code;
  }

  NetworkClientSecure client_;
  HTTPClient http_;
  bool aborted_ = false;
  bool callbackAborted_ = false;
  bool responseComplete_ = false;
};

} // namespace freeink

//
// Created by maik2 on 14.12.2021.
//

#ifndef RN_HTTPREQUEST_H
#define RN_HTTPREQUEST_H

#include <string>

class HttpRequest {
public:
    explicit HttpRequest(const std::string &data);
    enum class Method { GET, POST, PUT };

    Method get_method() const;
    std::string get_version() const;
    std::string get_uri() const;
    std::string get_headers() const;

private:
    Method method = Method::GET;
    std::string uri;
    std::string http_version;
    std::string headers;
};

#endif //RN_HTTPREQUEST_H


#include <regex>
#include "url_info.hpp"

UrlInfo UrlParser::parse(const std::string &url) {
    std::regex regex(".*?(?:http:[/][/])?(?:www[.])?([^\\.]*[.][^/]*)[/](.*)");
    std::smatch matches;
    if (!std::regex_match(url, matches, regex) || matches.size() != 3) {
        throw std::invalid_argument("url");
    }

    std::string host = matches[1];
    std::string file = matches[2];

    return UrlInfo{matches[1], matches[2]};

}

UrlInfo::UrlInfo(const std::string &host, const std::string &file) : host(host), file(file) {}

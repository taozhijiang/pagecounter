/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#include <string>
#include <json/json.h>

#include <string/StrUtil.h>
#include <tzhttpd/HttpParser.h>

#include "Captain.h"
#include "StoreIf.h"

// js直接嵌入到代码中，返回给客户端
int jssource_get_handler(const tzhttpd::HttpParser& http_parser,
                         std::string& response, std::string& status_line, std::vector<std::string>& add_header) {

    const std::string js_source =
        " // url   POST destination address                 \n "
        " // id    account_id                               \n "
        " // elem  element, update its innerTHML            \n "
        " function counter_report(url, id, elem) {          \n "
        "                                                   \n "
        "     var post_data = {                             \n "
        "         'id'    : id,                             \n "
        "         'host'  : window.location.host,           \n "
        "         'uri'   : window.location.pathname,       \n "
        "         'proto' : window.location.protocol,       \n "
        "         'origin': window.location.origin,         \n "
        "         'browser'  : window.navigator.userAgent,  \n "
        "         'platform' : window.navigator.platform,   \n "
        "         'language' : window.navigator.language,   \n "
        "     };                                            \n "
        "     var content = JSON.stringify(post_data);      \n "
        "                                                   \n "
        "     // XMLHttpRequest better compatiable with legacy-browser \n "
        "     var xhr = new XMLHttpRequest();               \n "
        "     xhr.open('POST', url, true);                  \n "
        "     xhr.setRequestHeader('Content-type','application/json; charset=utf-8'); \n "
        "                                                   \n "
        "     xhr.onload = function () {                    \n "
        "         if (xhr.readyState == 4 && xhr.status == '200') { \n "
        "             page_counter.innerHTML = xhr.responseText; \n "
        "         } else {                                  \n "
        "             console.error(xhr.responseText);      \n "
        "         }                                         \n "
        "     }                                             \n "
        "     xhr.send(content);                            \n "
        " }                                                    ";

    response = js_source;
    add_header.emplace_back("Content-Type: text/javascript");

    std::string http_ver = http_parser.get_version();
    status_line = tzhttpd::http_proto::generate_response_status_line(http_ver, tzhttpd::http_proto::StatusCode::success_ok);

    return 0;
}

int counter_post_handler(const tzhttpd::HttpParser& http_parser, const std::string& post_body,
                         std::string& response, std::string& status_line, std::vector<std::string>& add_header) {

    // roo::log_info("post content: %s", post_body.c_str());

    do {

        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(post_body, root) || root.isNull()) {
            roo::log_err("Json parse error for: %s", post_body.c_str());
            return 1;
        }

        if (root.isMember("id") && root["id"].isInt64() &&
            root.isMember("host") && root["host"].isString() &&
            root.isMember("uri") && root["uri"].isString()) {
            // OK
        } else {
            roo::log_err("Post parameter check failed of %s.", post_body.c_str());
            break;
        }

        visit_info info{};
        info.id_   = root["id"].asInt64();
        info.host_ = root["host"].asString();
        info.uri_  = root["uri"].asString();

        // 扩展字段
        if (root.isMember("proto") && root["proto"].isString())
            info.proto_ = root["proto"].asString();

        if (root.isMember("origin") && root["origin"].isString())
            info.origin_ = root["origin"].asString();

        if (root.isMember("browser") && root["browser"].isString())
            info.browser_ = root["browser"].asString();

        if (root.isMember("platform") && root["platform"].isString())
            info.platf_ = root["platform"].asString();

        if (root.isMember("language") && root["language"].isString())
            info.lang_ = root["language"].asString();

        // 记录RemoteClient
        // 如果经过了反向代理，则取 X-Real-IP，否则就取remote_addr
        info.remote_ = http_parser.find_request_header("X-Real-IP");
        if (info.remote_.empty()) {
            boost::system::error_code ignore_ec;
            info.remote_ = http_parser.remote_.address().to_string(ignore_ec);
        }

        Captain::instance().store_ptr_->insert_visit_event(info);
        int64_t c_counter = 0;
        int64_t t_counter = Captain::instance().store_ptr_->select_visit_stat(info.id_, info.host_, info.uri_, c_counter);
        if (t_counter >= 0) {
            response = roo::va_format("访问统计 - 本站：%ld, 本页：%ld.",
                                      t_counter, c_counter);
            add_header.emplace_back("Content-Type: application/json");
        } else {
            roo::log_err("retrive visit stat failed.");
        }


    } while (0);

    std::string http_ver = http_parser.get_version();
    status_line = tzhttpd::http_proto::generate_response_status_line(http_ver, tzhttpd::http_proto::StatusCode::success_ok);

    return 0;
}



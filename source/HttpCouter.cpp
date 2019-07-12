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

        Captain::instance().store_ptr_->insert_visit_event(info);
        int64_t c_counter = 0;
        int64_t t_counter = Captain::instance().store_ptr_->select_visit_stat(info.id_, info.host_, info.uri_, c_counter);
        if (t_counter >= 0) {
            response = roo::va_format("<p>本站阅读：%ld，本页阅读：%ld</p>",
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



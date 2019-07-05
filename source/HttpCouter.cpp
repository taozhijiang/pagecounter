/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#include <string>
#include <tzhttpd/HttpParser.h>


int counter_post_handler(const tzhttpd::HttpParser& http_parser, const std::string& post_body,
                         std::string& response, std::string& status_line, std::vector<std::string>& add_header ) {

    std::string http_ver = http_parser.get_version();
    status_line = tzhttpd::http_proto::generate_response_status_line(http_ver, tzhttpd::http_proto::StatusCode::success_ok);

    response = "{\"total\":234, \"current_page\": 22 }";

    add_header.emplace_back("Content-Type: application/json");
    return 0;
}



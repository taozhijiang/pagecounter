/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __STORE_IF_H__
#define __STORE_IF_H__

#include <xtra_rhel.h>

struct visit_info {
    int64_t id_;
    std::string host_;
    std::string uri_;
    std::string proto_;
    std::string origin_;
    std::string browser_;
    std::string platf_;
    std::string lang_;

    std::string str() const {
        std::stringstream ss;
        ss  << "id: " << id_ << std::endl
            << "host: " << host_ << std::endl
            << "uri: " << uri_ << std::endl
            << "proto: " << proto_ << std::endl
            << "origin: " << origin_ << std::endl
            << "browser: " << browser_ << std::endl
            << "platform: " << platf_ << std::endl
            << "language: " << lang_ << std::endl;

        return ss.str();
    }
};

class StoreIf {

public:

    virtual bool init() = 0;

    virtual int insert_visit_event(const struct visit_info& info) = 0;

    // 返回当前站点总访问量
    // <0 表示异常
    virtual int64_t select_visit_stat(int64_t id, const std::string& host) = 0;
    virtual int64_t select_visit_stat(int64_t id, const std::string& host, const std::string& uri,
                                      int64_t& cur_count) = 0;

};


#endif // __STORE_IF_H__

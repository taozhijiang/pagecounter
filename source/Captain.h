/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __CAPTAIN_H__
#define __CAPTAIN_H__

#include <xtra_rhel.h>

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace roo {
class Setting;
class Status;
class Timer;
}

namespace tzhttpd {
class HttpServer;
}

class StoreIf;

class Captain {

    __noncopyable__(Captain)

public:
    static Captain& instance();

public:
    bool init(const std::string& cfgFile);

    bool service_joinall();
    bool service_graceful();
    void service_terminate();

public:

    // 实例仍然会调度，这里控制是否执行函数

    volatile bool running_;

    std::shared_ptr<roo::Setting> setting_ptr_;
    std::shared_ptr<roo::Status> status_ptr_;

    std::shared_ptr<roo::Timer> timer_ptr_;

    std::shared_ptr<tzhttpd::HttpServer> http_server_ptr;

    std::shared_ptr<StoreIf> store_ptr_;

private:
    Captain();

    ~Captain() {
        // Singleton should not destoried normally,
        // if happens, just terminate quickly
        ::exit(EXIT_SUCCESS);
    }


    bool initialized_;

};



#endif // __CAPTAIN_H__

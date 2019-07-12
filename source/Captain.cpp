/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#include <syslog.h>
#include <cstdlib>

#include <memory>
#include <string>
#include <map>


#include <concurrency/Timer.h>

#include <scaffold/Setting.h>
#include <scaffold/Status.h>

#include <tzhttpd/HttpParser.h>
#include <tzhttpd/HttpServer.h>

#include "StoreSql.h"
#include "Captain.h"

// HttpCouter中定义
extern
int counter_post_handler(const tzhttpd::HttpParser& http_parser, const std::string& post_body,
                         std::string& response, std::string& status_line, std::vector<std::string>& add_header);

// 在主线程中最先初始化，所以不考虑竞争条件问题
Captain& Captain::instance() {
    static Captain service;
    return service;
}

Captain::Captain() :
    running_(true),
    initialized_(false) {
}

bool Captain::init(const std::string& cfgFile) {

    if (initialized_) {
        roo::log_err("Captain already initlialized before ...");
        return false;
    }

    timer_ptr_ = std::make_shared<roo::Timer>();
    if (!timer_ptr_ || !timer_ptr_->init()) {
        roo::log_err("Create and init roo::Timer service failed.");
        return false;
    }

    setting_ptr_ = std::make_shared<roo::Setting>();
    if (!setting_ptr_ || !setting_ptr_->init(cfgFile)) {
        roo::log_err("Create and init roo::Setting with cfg %s failed.", cfgFile.c_str());
        return false;
    }

    auto setting_ptr = setting_ptr_->get_setting();
    if (!setting_ptr) {
        roo::log_err("roo::Setting return null pointer, maybe your conf file ill???");
        return false;
    }

    int log_level = 0;
    setting_ptr->lookupValue("log_level", log_level);
    if (log_level <= 0 || log_level > 7) {
        roo::log_warning("Invalid log_level %d, reset to default 7(DEBUG).", log_level);
        log_level = 7;
    }

    std::string log_path;
    setting_ptr->lookupValue("log_path", log_path);
    if (log_path.empty())
        log_path = "./log";

    roo::log_init(log_level, "", log_path, LOG_LOCAL6);
    roo::log_warning("Initialized roo::Log with level %d, path %s.", log_level, log_path.c_str());

    status_ptr_ = std::make_shared<roo::Status>();
    if (!status_ptr_) {
        roo::log_err("Create roo::Status failed.");
        return false;
    }

    // MySQL
    store_ptr_.reset(new StoreSql());
    if (!store_ptr_ || !store_ptr_->init()) {
        roo::log_err("Create or Init StoreSql failed.");
        return false;
    }

    // http server
    http_server_ptr.reset(new tzhttpd::HttpServer(cfgFile, "example_main"));
    if (!http_server_ptr || !http_server_ptr->init()) {
        roo::log_err("create or init HttpServer failed!");
        return false;
    }

    // 注册处理句柄
    http_server_ptr->add_http_post_handler("^/counter$", counter_post_handler);

    http_server_ptr->io_service_threads_.start_threads();
    http_server_ptr->service();

    roo::log_warning("Captain makes all initialized...");
    initialized_ = true;

    return true;
}


bool Captain::service_graceful() {

    timer_ptr_->threads_join();
    http_server_ptr->io_service_threads_.join_threads();

    return true;
}

void Captain::service_terminate() {

    ::sleep(1);
    ::_exit(0);
}

bool Captain::service_joinall() {

    timer_ptr_->threads_join();
    http_server_ptr->io_service_threads_.join_threads();

    return true;
}


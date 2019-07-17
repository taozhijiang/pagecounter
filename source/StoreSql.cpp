/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#include <sstream>

#include <other/Log.h>
#include <string/StrUtil.h>
#include <scaffold/Setting.h>

#include "StoreSql.h"
#include "Captain.h"

bool StoreSql::init() {

    auto setting_ptr = Captain::instance().setting_ptr_->get_setting();
    if (!setting_ptr) {
        roo::log_err("roo::Setting return null pointer, maybe your conf file ill???");
        return false;
    }

    std::string mysql_hostname;
    int mysql_port;
    std::string mysql_username;
    std::string mysql_passwd;
    std::string mysql_database;
    if (!setting_ptr->lookupValue("mysql.host_addr", mysql_hostname) ||
        !setting_ptr->lookupValue("mysql.host_port", mysql_port) ||
        !setting_ptr->lookupValue("mysql.username", mysql_username) ||
        !setting_ptr->lookupValue("mysql.passwd", mysql_passwd) ||
        !setting_ptr->lookupValue("mysql.database", mysql_database)) {
        roo::log_err("Error, get mysql required config error");
        return false;
    }

    database_ = mysql_database;

    int conn_pool_size = 0;
    if (!setting_ptr->lookupValue("rpc.business.mysql.conn_pool_size", conn_pool_size)) {
        conn_pool_size = 20;
        roo::log_info("Using default conn_pool size: 20");
    }

//    mysql_passwd = Security::DecryptSelfParam(mysql_passwd.c_str());
    roo::SqlConnPoolHelper helper(mysql_hostname, mysql_port,
                                  mysql_username, mysql_passwd, mysql_database);
    sql_pool_ptr_.reset(new roo::ConnPool<roo::SqlConn, roo::SqlConnPoolHelper>("MySQLPool", conn_pool_size, helper));
    if (!sql_pool_ptr_ || !sql_pool_ptr_->init()) {
        roo::log_err("Init SqlConnPool failed!");
        return false;
    }

    // 加载数据
    std::string sql;
    roo::sql_conn_ptr conn;
    std::vector<std::string> result;

    sql_pool_ptr_->request_scoped_conn(conn);
    if (!conn) {
        roo::log_err("request sql conn failed!");
        return false;
    }

    // user_agent 缓存
    sql = roo::va_format("SELECT F_user_agent_real FROM %s.t_user_agent_map;", database_.c_str());
    if (!conn->select_multi(sql, result)) {
        roo::log_err("select %s failed.", sql.c_str());
        return false;
    }
    std::set<std::string> tmp_set(result.cbegin(), result.cend());
    user_agent_cache_.swap(tmp_set);
    roo::log_warning("total load %lu count user_agents.", user_agent_cache_.size());
    result.clear();

    // host uri 缓存
    sql = roo::va_format("SELECT DISTINCT(F_host) FROM %s.t_uri_map", database_.c_str());
    if (!conn->select_multi(sql, result)) {
        roo::log_err("select %s failed.", sql.c_str());
        return false;
    }
    for (size_t i=0; i<result.size(); ++i) {

        std::string host = result[i];
        std::vector<std::string> uris;

        sql = roo::va_format("SELECT F_uri_real FROM %s.t_uri_map WHERE F_host='%s';",
                             database_.c_str(), host.c_str());
        if (!conn->select_multi(sql, uris)) {
            roo::log_err("select %s failed.", sql.c_str());
            return false;
        }

        uri_cache_[host] = HashSetType(uris.cbegin(), uris.cend());
        roo::log_warning("total load %lu uris for host %s.",uri_cache_[host].size() ,host.c_str());
    }

    return true;
}

void StoreSql::check_uri_digest(const std::string& host, const std::string& uri) {

    std::lock_guard<std::mutex> lock(lock_);
    auto iter = uri_cache_.find(host);
    if (iter == uri_cache_.cend()) {
        uri_cache_[host] = HashSetType();
        iter = uri_cache_.find(host);
    }

    if (iter->second.find(uri) != iter->second.end())
        return;

    // 执行插入操作
    std::string sql;
    roo::sql_conn_ptr conn;

    sql_pool_ptr_->request_scoped_conn(conn);
    if (!conn) {
        roo::log_err("request sql conn failed!");
        return;
    }

    // 插入
    sql = roo::va_format("INSERT INTO %s.t_uri_map SET "
                         "F_host = '%s', F_uri_digest = UNHEX(MD5('%s')), F_uri_real = '%s', F_create_time=NOW()",
                         database_.c_str(), host.c_str(), uri.c_str(), uri.c_str());
    conn->execute_update(sql);
    roo::log_info("insert %s for host %s", uri.c_str(), host.c_str());

    iter->second.insert(uri);
}

void StoreSql::check_user_agent_digest(const std::string& user_agent) {

    std::lock_guard<std::mutex> lock(lock_);
    if (user_agent_cache_.find(user_agent) != user_agent_cache_.end())
        return;

    // 执行插入
    std::string sql;
    roo::sql_conn_ptr conn;

    sql_pool_ptr_->request_scoped_conn(conn);
    if (!conn) {
        roo::log_err("request sql conn failed!");
        return;
    }

    sql = roo::va_format("INSERT INTO %s.t_user_agent_map SET "
                         "F_user_agent_digest = UNHEX(MD5('%s')), F_user_agent_real = '%s', F_create_time=NOW()",
                         database_.c_str(), user_agent.c_str(), user_agent.c_str());
    conn->execute_update(sql);
    roo::log_info("insert user_agent %s", user_agent.c_str());

    user_agent_cache_.insert(user_agent);
}



int StoreSql::insert_visit_event(const struct visit_info& stat) {

    std::string sql;
    roo::sql_conn_ptr conn;

    sql_pool_ptr_->request_scoped_conn(conn);
    if (!conn) {
        roo::log_err("request sql conn failed!");
        return -1;
    }

    // 假定参数外层已经检查OK
    check_uri_digest(stat.host_, stat.uri_);
    check_user_agent_digest(stat.browser_);

    // 插入明细表
    sql = roo::va_format(
        " INSERT INTO %s.t_visit_detail "
        " SET F_id=%ld, F_host='%s', F_uri_digest=UNHEX(MD5('%s')), F_proto='%s',"
        " F_origin='%s', F_browser=UNHEX(MD5('%s')), F_platf='%s', F_lang='%s', "
        " F_ip_addr=inet_aton('%s'), F_visit_time=NOW(); ",
        database_.c_str(),
        stat.id_, stat.host_.c_str(), stat.uri_.c_str(), stat.proto_.c_str(),
        stat.origin_.c_str(), stat.browser_.c_str(), stat.platf_.c_str(),
        stat.lang_.c_str(), stat.remote_.c_str());

    int affected = conn->execute_update(sql);
    if (affected != 1) {
        roo::log_err("insert visit event failed: %s.", stat.str().c_str());
    }

    // 增加统计表
    sql = roo::va_format(
        " INSERT INTO %s.t_visit_summary "
        " SET F_id=%ld, F_host='%s', F_uri_digest=UNHEX(MD5('%s')), F_count=1 "
        " ON DUPLICATE KEY UPDATE F_count = F_count + 1",
        database_.c_str(), stat.id_, stat.host_.c_str(), stat.uri_.c_str());
    affected = conn->execute_update(sql);

    if (affected != 1 && affected != 2) {
        roo::log_err("update %s.t_visit_summary failed.", database_.c_str());
    }

    return 0;
}


int64_t StoreSql::select_visit_stat(int64_t id, const std::string& host) {

    std::string sql;
    roo::sql_conn_ptr conn;
    int64_t ret_count = -1;

    sql_pool_ptr_->request_scoped_conn(conn);
    if (!conn) {
        roo::log_err("request sql conn failed!");
        return -1;
    }

    sql = roo::va_format(" SELECT SUM(F_count) FROM %s.t_visit_summary "
                         " WHERE F_id=%ld AND F_host='%s'; ",
                         database_.c_str(), id, host.c_str());

    if (!conn->select_one(sql, ret_count)) {
        roo::log_err("select %s failed.", sql.c_str());
        return -1;
    }

    return ret_count;
}



int64_t StoreSql::select_visit_stat(int64_t id, const std::string& host, const std::string& uri,
                                    int64_t& cur_count) {

    std::string sql;
    roo::sql_conn_ptr conn;

    int64_t ret_count = -1;

    sql_pool_ptr_->request_scoped_conn(conn);
    if (!conn) {
        roo::log_err("request sql conn failed!");
        return -1;
    }

    sql = roo::va_format(" SELECT F_count FROM %s.t_visit_summary "
                         " WHERE F_id=%ld AND F_host='%s' AND F_uri_digest=UNHEX(MD5('%s')); ",
                         database_.c_str(), id, host.c_str(), uri.c_str());

    if (!conn->select_one(sql, cur_count)) {
        cur_count = 0;
    }

    // 总数
    sql = roo::va_format(" SELECT IFNULL(SUM(F_count), 0) FROM %s.t_visit_summary "
                         " WHERE F_id=%ld AND F_host='%s'; ",
                         database_.c_str(), id, host.c_str());

    if (!conn->select_one(sql, ret_count)) {
        roo::log_err("select %s failed.", sql.c_str());
        ret_count = -1;
    }

    return ret_count;
}

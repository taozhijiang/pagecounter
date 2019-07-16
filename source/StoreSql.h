/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __STORE_SQL_H__
#define __STORE_SQL_H__

#include <unordered_set>

#include <connect/SqlConn.h>

#include "StoreIf.h"

class StoreSql : public StoreIf {
public:

    StoreSql():
        lock_(),
        user_agent_cache_(),
        uri_cache_() {
    }

    ~StoreSql() = default;

    bool init()override;

    int insert_visit_event(const struct visit_info& info)override;

    int64_t select_visit_stat(int64_t id, const std::string& host)override;
    int64_t select_visit_stat(int64_t id, const std::string& host, const std::string& uri,
                              int64_t& cur_count)override;

private:

    void check_user_agent_digest(const std::string& user_agent);
    void check_uri_digest(const std::string& host, const std::string& uri);

    // 数据库连接池
    std::shared_ptr<roo::ConnPool<roo::SqlConn, roo::SqlConnPoolHelper>> sql_pool_ptr_;
    std::string database_;

    // in-mem for fast hit
    std::mutex lock_;

    std::set<std::string> user_agent_cache_;

    typedef std::unordered_set<std::string> HashSetType;
    std::map<std::string, HashSetType> uri_cache_;
};

#endif // __STORE_SQL_H__

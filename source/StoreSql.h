/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __STORE_SQL_H__
#define __STORE_SQL_H__

#include <connect/SqlConn.h>

#include "StoreIf.h"

class StoreSql : public StoreIf {
public:

    bool init()override;

    int insert_visit_event(const struct visit_info& info)override;

    int64_t select_visit_stat(int64_t id, const std::string& host)override;
    int64_t select_visit_stat(int64_t id, const std::string& host, const std::string& uri,
                              int64_t& cur_count)override;

private:

    void check_uri_digest(const std::string& host, const std::string& uri);

    // 数据库连接
    std::shared_ptr<roo::ConnPool<roo::SqlConn, roo::SqlConnPoolHelper>> sql_pool_ptr_;
    std::string database_;
};

#endif // __STORE_SQL_H__


// 访问汇总表
CREATE TABLE `t_page_counter_uri_map` (
  `F_host`          varchar(128) NOT NULL COMMENT '访问域名',
  `F_uri_digest`    binary(16) NOT NULL COMMENT '访问路径摘要',
  `F_uri_real`      varchar(512) NOT NULL DEFAULT '' COMMENT '真实路径',
  `F_create_time`   timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  
  PRIMARY KEY (`F_host`, `F_uri_digest`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

// 访问汇总表
CREATE TABLE `t_page_counter_visit_summary` (

  `F_id`           bigint(20) NOT NULL COMMENT '账号ID',
  `F_host`         varchar(128) NOT NULL COMMENT '访问域名',
  `F_uri_digest`   binary(16) NOT NULL COMMENT '访问路径摘要',
  `F_count`        bigint(20) NOT NULL DEFAULT '0' COMMENT '访问次数',
  `F_update_time`  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '最后更新时间',
  
  PRIMARY KEY (`F_id`, `F_host`, `F_uri_digest`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

// 访问明细记录表
CREATE TABLE `t_page_counter_visit_detail` (
  `F_increment_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  
  `F_id`           bigint(20) NOT NULL COMMENT '账号ID',
  `F_host`         varchar(128) NOT NULL COMMENT '访问域名',
  `F_uri_digest`   binary(16) NOT NULL COMMENT '访问路径摘要',
  `F_proto`        varchar(20) NOT NULL COMMENT '访问协议',
  `F_origin`       varchar(128) NOT NULL DEFAULT '',
  `F_browser`      varchar(128) NOT NULL DEFAULT '' COMMENT 'UserAgent',
  `F_platf`        varchar(64) NOT NULL DEFAULT '' COMMENT '客户端平台',
  `F_lang`         varchar(64) NOT NULL DEFAULT '' COMMENT '客户端语言',
  `F_visit_time`   timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '访问时间',
    
  PRIMARY KEY (`F_increment_id`),
  KEY `F_index` (`F_id`, `F_host`, `F_uri_digest`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
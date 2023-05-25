/*
Navicat MySQL Data Transfer

Source Server         : localhost
Source Server Version : 50720
Source Host           : localhost:3306
Source Database       : apie

Target Server Type    : MYSQL
Target Server Version : 50720
File Encoding         : 65001

Date: 2021-04-08 09:38:37
*/

SET FOREIGN_KEY_CHECKS=0;

--
-- Table structure for table `role_base`
--

DROP TABLE IF EXISTS `role_base`;
CREATE TABLE `role_base` (
  `user_id` bigint(20) unsigned NOT NULL,
  `game_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `level` int(4) unsigned NOT NULL DEFAULT '0' COMMENT '璐﹀彿绛夌骇',
  `register_time` bigint(20) NOT NULL DEFAULT '0',
  `login_time` bigint(20) NOT NULL,
  `offline_time` bigint(20) NOT NULL,
  `name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `role_extra`
--

DROP TABLE IF EXISTS `role_extra`;
CREATE TABLE `role_extra` (
  `user_id` bigint(20) unsigned NOT NULL,
  `extra_info` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `tbl_mail_recoverer`
--

DROP TABLE IF EXISTS `tbl_mail_recoverer`;
CREATE TABLE `tbl_mail_recoverer` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'Id',
  `user_id` bigint(20) NOT NULL DEFAULT '0' COMMENT 'UserId',
  `title` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '邮件标题',
  `title_id` int(11) NOT NULL DEFAULT '0' COMMENT '邮件标题模板ID',
  `title_param` varchar(255) DEFAULT NULL COMMENT '内容参数列表',
  `type` smallint(5) unsigned DEFAULT NULL COMMENT '邮件分类标',
  `guild_id` int(11) NOT NULL DEFAULT '0' COMMENT '颁布者归属联盟ID',
  `src_user_id` bigint(20) DEFAULT NULL COMMENT '颁布者玩家id',
  `src_user_job_id` int(11) NOT NULL DEFAULT '0' COMMENT '颁布者玩家id',
  `user_name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '玩家名称',
  `memo` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '邮件内容',
  `memo_id` int(11) NOT NULL DEFAULT '0' COMMENT '邮件内容模板ID',
  `memo_param` varchar(1024) DEFAULT NULL COMMENT '内容参数列表',
  `reward_param` varchar(2048) DEFAULT NULL COMMENT '物品列表',
  `create_time` bigint(20) NOT NULL DEFAULT '0' COMMENT '创建时间',
  `is_multi_lang` int(11) NOT NULL DEFAULT '0' COMMENT '是否多语言，0：不是，1：是',
  `title_lang_cn` varchar(1024) NOT NULL DEFAULT '' COMMENT '中文标题',
  `title_lang_en` varchar(1024) NOT NULL DEFAULT '' COMMENT '英文标题',
  `content_lang_cn` varchar(2048) NOT NULL DEFAULT '' COMMENT '中文内容',
  `content_lang_en` varchar(2048) NOT NULL DEFAULT '' COMMENT '英文内容',
  `save_flag` int(11) NOT NULL DEFAULT '0' COMMENT '是否跨赛季保存，0：不保存，1：保存',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for table `varchars1`
--

DROP TABLE IF EXISTS `varchars1`;
CREATE TABLE `varchars1` (
  `a` int(11) NOT NULL DEFAULT '0',
  `b` int(10) unsigned NOT NULL DEFAULT '0',
  `c` bigint(20) NOT NULL DEFAULT '0',
  `te` varchar(100) NOT NULL DEFAULT 'hello world',
  `aa` char(1) DEFAULT NULL,
  `aaa` blob,
  `aaa1` text,
  `Column1aaa` tinyblob,
  `Column1` tinytext,
  `Column2` longblob,
  `Column3` longtext,
  `Column4` mediumblob,
  `Column5` mediumtext,
  `Column6` datetime DEFAULT NULL,
  PRIMARY KEY (`a`,`b`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
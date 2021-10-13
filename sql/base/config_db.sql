-- MySQL dump 10.13  Distrib 5.7.20, for Win64 (x86_64)
--
-- Host: localhost    Database: config_db
-- ------------------------------------------------------
-- Server version	5.7.20-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `service_node`
--

DROP TABLE IF EXISTS `service_node`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `service_node` (
  `service_realm` int(10) unsigned NOT NULL,
  `service_type` int(10) unsigned NOT NULL,
  `service_id` int(10) unsigned NOT NULL,
  `ip` varchar(100) NOT NULL DEFAULT '',
  `port` int(10) unsigned NOT NULL DEFAULT '0',
  `listeners_config` varchar(512) NOT NULL DEFAULT '',
  `mysql_config` varchar(512) NOT NULL DEFAULT '',
  `nats_config` varchar(512) NOT NULL DEFAULT '',
  PRIMARY KEY (`service_realm`,`service_type`,`service_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `service_node`
--

LOCK TABLES `service_node` WRITE;
/*!40000 ALTER TABLE `service_node` DISABLE KEYS */;
INSERT INTO `service_node` VALUES (1,1,1,'',0,'{\r\n	\"bind\": [{\r\n		\"ip\": \"127.0.0.1\",\r\n		\"port\": 5007,\r\n		\"type\": 1,\r\n		\"mask_flag\": 0\r\n	}]\r\n}','',''),(1,2,1,'127.0.0.1',16007,'{\r\n	\"bind\": [{\r\n		\"ip\": \"127.0.0.1\",\r\n		\"port\": 16007,\r\n		\"type\": 1,\r\n		\"mask_flag\": 0\r\n	}]\r\n}','','{\r\n	\"connections\": [{\r\n		\"type\": 1,\r\n		\"nats_server\": \"nats://127.0.0.1:4222\",\r\n		\"channel_domains\": \"sub_topic\"\r\n	}]\r\n}'),(1,3,1,'127.0.0.1',14007,'{\r\n	\"bind\": [{\r\n		\"ip\": \"127.0.0.1\",\r\n		\"port\": 14007,\r\n		\"type\": 1,\r\n		\"mask_flag\": 0\r\n	}]\r\n}','','{\r\n	\"connections\": [{\r\n		\"type\": 1,\r\n		\"nats_server\": \"nats://127.0.0.1:4222\",\r\n		\"channel_domains\": \"sub_topic\"\r\n	}]\r\n}'),(1,4,1,'',0,'','','{\r\n	\"connections\": [{\r\n		\"type\": 1,\r\n		\"nats_server\": \"nats://127.0.0.1:4222\",\r\n		\"channel_domains\": \"sub_topic\"\r\n	}]\r\n}'),(1,5,1,'',0,'','{\r\n	\"host\": \"127.0.0.1\",\r\n	\"port\": 3306,\r\n	\"user\": \"root\",\r\n	\"passwd\": \"root\",\r\n	\"db\": \"apie_account\"\r\n}','{\r\n	\"connections\": [{\r\n		\"type\": 1,\r\n		\"nats_server\": \"nats://127.0.0.1:4222\",\r\n		\"channel_domains\": \"sub_topic\"\r\n	}]\r\n}'),(1,6,1,'',0,'','{\r\n	\"host\": \"127.0.0.1\",\r\n	\"port\": 3306,\r\n	\"user\": \"root\",\r\n	\"passwd\": \"root\",\r\n	\"db\": \"apie\"\r\n}','{\r\n	\"connections\": [{\r\n		\"type\": 1,\r\n		\"nats_server\": \"nats://127.0.0.1:4222\",\r\n		\"channel_domains\": \"sub_topic\"\r\n	}]\r\n}');
/*!40000 ALTER TABLE `service_node` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'config_db'
--
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-10-13 14:26:09

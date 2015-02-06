// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-18


#include "gtest/gtest.h"
#include <string>
#include "stdio.h"
#include "log/logger.h"
#include "db/mysql_conf.h"


using namespace std;

class CMysqlConfUnittest: public testing::Test
{
public:
    virtual void SetUp()
    {
        INIT_LOG("./", __FILE__,"/home/david.luan/webgame/server/workerserver/conf/log.conf" );

        INIT_MYSQLCONF("/home/david.luan/webgame/server/workerserver/conf/fnf_worker_mysql.xml");
    }

    virtual void TearDown()
    {

    }

private:
    /* data */
};
TEST_F(CMysqlConfUnittest, query_uid_23)
{
    unsigned long long uid = 23;
    MYSQLNETPOINT stPoint;
    int ret = QUERY_MYSQLDEST(uid,stPoint);
    EXPECT_EQ(0 , ret);
    EXPECT_EQ(3306, stPoint.port);
    EXPECT_STREQ("192.168.2.11", stPoint.ip.c_str());
    EXPECT_STREQ("t_user_", stPoint.tblprefix.c_str());
    EXPECT_STREQ("fnf_db_2", stPoint.dbname.c_str());
}

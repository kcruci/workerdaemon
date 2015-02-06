// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-18


#include "gtest/gtest.h"
#include <string>
#include "stdio.h"
#include "log/logger.h"
#include "db/redis_conf.h"


using namespace std;

class CRedisConfUnittest: public testing::Test
{
public:
    virtual void SetUp()
    {
        INIT_LOG("./", __FILE__,"/home/david.luan/webgame/server/workerserver/conf/log.conf" );
    }

    virtual void TearDown()
    {

    }

	CRedisConf _conf;
private:
    /* data */
};
TEST_F(CRedisConfUnittest, query_uid_23)
{
		_conf.LoadConf("/home/achilsh.hu/works/persistent_cache/workerserver/conf/fnf_worker_redis.xml");
		std::vector<NETPOINT> allPoint;
		int ret =  _conf.GetQueenAllIpPort(allPoint);
		EXPECT_EQ(0 , ret);
		EXPECT_EQ(10, allPoint.size());
		EXPECT_STREQ("192.168.2.15", allPoint[0].ip.c_str());
		EXPECT_STREQ("10100", allPoint[0].port.c_str());


}

// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-19


#include "gtest/gtest.h"
#include "db/redis_wrapper.h"
#include <string>
#include "stdio.h"
#include "log/logger.h"
#include "basic/trans.h"
#include "db/redis_pool.h"
#include "macrodef.h"



using namespace std;

class CRedisUnittest: public testing::Test
{
public:
    virtual void SetUp()
    {
        INIT_LOG("./", __FILE__,"/home/david.luan/webgame/server/workerserver/conf/log.conf" );
        ip = "192.168.0.21";
        port=30400;
        userbitmapkey = "userbitmap";
    }

    virtual void TearDown()
    {

    }
    std::string ip;
    int port;
    std::string userbitmapkey;

private:
    /* data */
};
TEST_F(CRedisUnittest, Connect)
{
    bool c =true;
    int iRet (0);
    CRedis* redis = GET_REDIS(ip, port);

  //  redis->DelKey(userbitmapkey);

  //  redis->ZADD(userbitmapkey, 1  , "test1");
  //  redis->ZADD(userbitmapkey, 12 , "test2");
  //  redis->ZADD(userbitmapkey, 3  , "test3");
  //  redis->ZADD(userbitmapkey, 4  , "test4");
  //  redis->ZADD(userbitmapkey, 5  , "test5");



    std::vector<std::string> vecVal;
    std::vector<std::string> vecScore;

    iRet = redis->ZRANGEWITHSCORE(userbitmapkey, 0 , -1, vecVal, vecScore);
    EXPECT_EQ(0, iRet);
  //  EXPECT_EQ(5, vecScore.size());
  //  EXPECT_EQ(5, vecVal.size());
  //  EXPECT_EQ(5, vecVal.size());
  //  EXPECT_STREQ("test2", vecVal[4].c_str());
  //  EXPECT_EQ(12, CTrans::STOI(vecScore[4]));
    FOR_EACH(itscore, vecScore)
    {
        fprintf(stderr, "%s\n", (*itscore).c_str());
    }

    std::vector<std::string> vecData2;

    iRet = redis->ZRANGE(userbitmapkey, 0 , -1, vecData2);
    EXPECT_EQ(0, iRet);
    EXPECT_EQ(5, vecData2.size());



}

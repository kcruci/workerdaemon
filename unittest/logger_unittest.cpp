// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-19


#include "gtest/gtest.h"
#include <string>
#include "stdio.h"
#include "log/logger.h"


using namespace std;

class CLoggerUnittest: public testing::Test
{
public:
    virtual void SetUp()
    {
        INIT_LOG("./", __FILE__,"/home/david.luan/webgame/server/workerserver/conf/log.conf" );
    }

    virtual void TearDown()
    {

    }

private:
    /* data */
};
TEST_F(CLoggerUnittest, log)
{
    int ret = 0;
    LOG_DEBUG("test");
    LOG_ERROR("error");
    EXPECT_EQ(0, ret);
}

// Copyright (C) 2014  .
// Author: davidluan
// CreateTime: 2014-02-15

#include "gtest/gtest.h"
#include "db/mysql_wrapper.h"
#include <string>
#include "stdio.h"
#include "log/logger.h"



using namespace std;

class CMysqlUnittest: public testing::Test
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
TEST_F(CMysqlUnittest, Connect)
{
    CMysql mysql("192.168.2.11", 3306,"idreamsky","idreamsky", 1 );
    bool c = mysql.Connect();
    EXPECT_TRUE(c);
}

TEST_F(CMysqlUnittest, ShowDb)
{
    CMysql mysql("192.168.2.11", 3306,"idreamsky","idreamsky", 1 );
    mysql.Connect();

    std::string sql ="show databases;";
    bool c= mysql.Query(sql.c_str(), sql.size());


    EXPECT_TRUE(c);

}

TEST_F(CMysqlUnittest, query_empty )
{
    CMysql mysql("192.168.2.11", 3306,"idreamsky","idreamsky", 1 );
    bool c = mysql.Connect();
    EXPECT_TRUE(c);

    std::string sql ="select * from db_fnf_0.t_user_1;";

    c= mysql.Query(sql.c_str(), sql.size());
    EXPECT_TRUE(c);

    //empyt result
    ResultSet result;
    mysql.FetchResult(result);


    //empty select
    int rows = result.GetRowsNum();
    EXPECT_EQ(0, rows);


    EXPECT_FALSE(result.FetchRow());
}

TEST_F(CMysqlUnittest, insert_query)
{
    CMysql mysql("192.168.2.11", 3306,"idreamsky","idreamsky", 1 );
    bool c = mysql.Connect();
    EXPECT_TRUE(c);
    int uid = time(NULL);

    char chsql[100];
    snprintf(chsql, sizeof(chsql), "insert into db_fnf_1.t_user_9 (uid, profile) values (%d, 'dddd');", uid);

    std::string sql =chsql;

    c= mysql.Query(sql.c_str(), sql.size());
    EXPECT_TRUE(c);


    sql ="select * from db_fnf_1.t_user_9;";

    c= mysql.Query(sql.c_str(), sql.size());
    EXPECT_TRUE(c);

    //empyt result
    ResultSet result;
    mysql.FetchResult(result);


    //not empty select
    int rows = result.GetRowsNum();
    EXPECT_NE(0, rows);

}


TEST_F(CMysqlUnittest, update_query)
{
    CMysql mysql("192.168.2.11", 3306,"idreamsky","idreamsky", 1 );
    bool c = mysql.Connect();
    EXPECT_TRUE(c);
    int ts= time(NULL);

    //更新
    char chsql[100];
    snprintf(chsql, sizeof(chsql), "update db_fnf_1.t_user_9 set profile='dddd%d' where uid=1392725453;", ts);

    std::string sql =chsql;

    c= mysql.Query(sql.c_str(), sql.size());
    EXPECT_TRUE(c);

    unsigned int affectrow(0);
    c= mysql.AffectedRows(affectrow);

    EXPECT_EQ(1, affectrow);


    //查询
    sql ="select * from db_fnf_1.t_user_9;";

    c= mysql.Query(sql.c_str(), sql.size());
    EXPECT_TRUE(c);

    //result
    ResultSet result;
    mysql.FetchResult(result);

    //not empty select
    int rows = result.GetRowsNum();
    EXPECT_NE(0, rows);
}

TEST_F(CMysqlUnittest, query_all)
{
    CMysql mysql("192.168.2.11", 3306,"idreamsky","idreamsky", 1 );
    bool c = mysql.Connect();
    EXPECT_TRUE(c);

    char chsql[100];
    snprintf(chsql, sizeof(chsql), "select * from db_fnf_1.t_user_9 where uid=1392725453;");

    std::string sql =chsql;

    c= mysql.Query(sql.c_str(), sql.size());
    EXPECT_TRUE(c);

    ResultSet result;
    mysql.FetchResult(result);
    //not empty select
    int rows = result.GetRowsNum();
    EXPECT_EQ(1, rows);

}

#! /bin/bash

usage()
{
        echo "usage: `basename $0` start|stop "
}

fstPara=$1
MCURDIR=`pwd`
CONFDIR=/data/webgame/conf
proccName="worker_server"
RUN_EXE="/data/webgame/bin/$proccName"


if [ $# -eq 2 ];then
    usage
    exit 1
fi

# return 0:进程没有运行 1:进程已运行
# @param $1:要监控的程序的目录
monitor_process()
{
    if [ $# -ne 1 ]; then
        echo "usage: monitor_process /dir/file"
        return 1
    fi

    process_exists=$(ps -ef | grep "$1"  | grep -v grep | wc -l)

    if [ $process_exists -eq 0 ]; then
        return 0
    else
        return $process_exists
    fi
}

case  $fstPara in
        start|Start)
            monitor_process $proccName
            pro_cnt=$?

            if [ $pro_cnt -ne 0 ]; then
                echo "$proccName already running...process count:$pro_cnt"
                exit
            fi
            echo "###########################################################################################"
            echo "$RUN_EXE ${CONFDIR}/fnf_worker_redis.xml ${CONFDIR}/fnf_worker_mysql.xml ${CONFDIR}/keylist.xml ${CONFDIR}/log.conf 1 900  9-16"
            echo "###########################################################################################"
            $RUN_EXE ${CONFDIR}/fnf_worker_redis.xml ${CONFDIR}/fnf_worker_mysql.xml ${CONFDIR}/keylist.xml ${CONFDIR}/log.conf 1 900  9-16
        ;;
        stop|Stop) echo "stopping $proccName"
        killall -9 worker_server
        ;;
        *)usage
        ;;
esac


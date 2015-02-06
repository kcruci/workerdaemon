#! /bin/bash
usage()
{
		echo "usage: `basename $0` start|stop "
}

fstPara=$1
proccName=$2
MCURDIR=$PWD
CONFDIR=$MCURDIR/../conf
QUEEN_IP="192.168.0.21"
QUEEN_PORT="44444"

if [ $# -eq 2 ];then
			usage
			exit 1
fi


case  $fstPara in
		start|Start) echo "starting  cli_exe"
            echo "clien run on queen $QUEEN_IP $QUEEN_PORT"
		./cli_exe ../conf/fnf_worker_redis.xml ../conf/fnf_worker_mysql.xml ../conf/keylist.xml $QUEEN_IP $QUEEN_PORT
		;;
		stop|Stop) echo "stopping cli_exe"
		killall -9 cli_exe
		;;
		*)usage
		;;
esac


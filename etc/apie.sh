#! /bin/bash

#set -e   # Exit immediately if a simple command exits with a non-zero status.
#set -x   # activate debugging from here

OS_VER=`cat /etc/redhat-release | tr -cd "[0-9]{.}" | awk -F'.' '{print $1}'`
if [ "x$OS_VER" == "x7" ]; then
  SYSTEMCTL_SKIP_REDIRECT=1
fi

. /etc/rc.d/init.d/functions

RETVAL=0

SERVER[0]=/usr/local/apie/bin/db_proxy_server 
SERVER_ARGS[0]=/usr/local/apie/conf/db_account_proxy.yaml

SERVER[1]=/usr/local/apie/bin/db_proxy_server 
SERVER_ARGS[1]=/usr/local/apie/conf/db_role_proxy.yaml

SERVER[2]=/usr/local/apie/bin/gateway_server 
SERVER_ARGS[2]=/usr/local/apie/conf/gateway_server.yaml

SERVER[3]=/usr/local/apie/bin/login_server 
SERVER_ARGS[3]=/usr/local/apie/conf/login_server.yaml

SERVER[4]=/usr/local/apie/bin/scene_server 
SERVER_ARGS[4]=/usr/local/apie/conf/scene_server.yaml

SERVER[5]=/usr/local/apie/bin/service_registry 
SERVER_ARGS[5]=/usr/local/apie/conf/service_registry.yaml


# This is our service name
LOCKFILE=/var/lock/subsys/apie

echo_active() {
  [ "$BOOTUP" = "color" ] && $MOVE_TO_COL
  echo -n "["
  [ "$BOOTUP" = "color" ] && $SETCOLOR_SUCCESS
  echo -n $"  ACTIVE  "
  [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
  echo -n "]"
  echo -ne "\r"
  return 0
}

echo_inactive() {
  [ "$BOOTUP" = "color" ] && $MOVE_TO_COL
  echo -n "["
  [ "$BOOTUP" = "color" ] && $SETCOLOR_FAILURE
  echo -n $"  INACTIVE  "
  [ "$BOOTUP" = "color" ] && $SETCOLOR_NORMAL
  echo -n "]"
  echo -ne "\r"
  return 1
}

start() {
  # pid=`pidof -o $$ -o $PPID -o %PPID -x ${PROG}`
  # if [ -n "$pid" ]; then        
  #   failure
  #   echo
  #   echo $"${PROG} (pid $pid) is running..."
  #   return 0 
  # fi

  # `$SERVER` && success || failure
  # RETVAL=$?
  # echo
  # [ $RETVAL -eq 0 ] && touch $LOCKFILE

  length=${#SERVER[@]}
  echo "length:$length"

  curValue=0
  while(( $curValue<$length ))
  do
      echo "curValue:$curValue"
      echo "start:${SERVER[$curValue]} ${SERVER_ARGS[$curValue]}"
      `${SERVER[$curValue]} ${SERVER_ARGS[$curValue]}` && success || failure
      
      RETVAL=$?
      echo "RETVAL:$RETVAL"

      let "curValue++"
  done
  echo ""
}

stop() {
  length=${#SERVER[@]}
  echo "length:$length"

  curValue=0
  while(( $curValue<$length ))
  do
      echo "curValue:$curValue ${SERVER[$curValue]} ${SERVER_ARGS[$curValue]}"
      
      #killproc ${SERVER[$curValue]} -9 
      #RETVAL=$?
      #echo "RETVAL:$RETVAL"
      /usr/bin/ps -ef --no-headers | grep -v grep | grep ${SERVER[$curValue]}
      if [ $? -eq 0 ];then
        echo "killproc:${SERVER[$curValue]}"
        killproc ${SERVER[$curValue]} -9 
        RETVAL=$?
        echo "RETVAL:$RETVAL"
      else
        echo "inactive"
      fi

      let "curValue++"
  done
  echo ""
}

# See how we were called.
case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  restart)
    stop
    start
    ;;
  status)
    # status $PROG
    length=${#SERVER[@]}
    echo "length:$length"

    curValue=0
    while(( $curValue<$length ))
    do
        echo "curValue:$curValue"
        echo "status:${SERVER[$curValue]} ${SERVER_ARGS[$curValue]}"
        # status ${SERVER[$curValue]}
        /usr/bin/ps -ef --no-headers | grep -v grep | grep ${SERVER[$curValue]} | grep ${SERVER_ARGS[$curValue]} && echo_active || echo_inactive

        let "curValue++"
    done
    echo ""
    ;;
  *)
    echo $"Usage: $0 {start|stop|restart|status}"
    exit 2
esac

exit $RETVAL


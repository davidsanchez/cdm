#!/bin/sh

#Source global definitions
if [ -f /etc/bashrc ]; then
  . /etc/bashrc
fi

echo "Starting ssh server ..."
/usr/sbin/sshd -D &

# ADD MOS setup
source /home/cdmmgr/MOS.conf
crond
#crontab OPCUACrontab

tail -f /dev/null


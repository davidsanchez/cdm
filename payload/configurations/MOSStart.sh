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
crontab -l | { cat; echo "* * * * * /home/cdmmgr/Run_cron_CDM.sh > /home/cdmmgr/cdm/CDM_log.txt 2>&1"; } | crontab -
#crontab OPCUACrontab

tail -f /dev/null


#!/bin/sh

#Source global definitions
if [ -f /etc/bashrc ]; then
  . /etc/bashrc
fi

echo "Starting ssh server ..."
/usr/sbin/sshd -D &

# ADD MOS setup
source /home/mosmgr/MOS.conf
rm /var/run/crond.pid
crond
#crontab -l |

{ cat; echo "* * * * * /home/cdmmgr/Run_cron_CDM.sh > /home/cdmmgr/cdm/CDM_log.txt 2>&1"; } | crontab -

tail -f /dev/null


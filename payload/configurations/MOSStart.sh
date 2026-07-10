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

export -p > /home/cdmmgr/env.bash

sudo /usr/sbin/crond -n &
CROND_PID=$!  # Store the PID of the cron daemon

# Wait briefly for cron to start
sleep 2

#crond
#crontab -l |

echo "* * * * * /home/cdmmgr/Run_cron_CDM.sh > /home/cdmmgr/cdm/CDM_log.txt 2>&1" | crontab -

#{ cat; echo "* * * * * /home/cdmmgr/Run_cron_CDM.sh > /home/cdmmgr/cdm/CDM_log.txt 2>&1"; } | crontab -

tail -f /dev/null
#!/bin/bash
PROCESS="MOS-CDM-COMPOSE"

count=`ps aux | grep -v grep | grep "$PROCESS" | awk {'print $5'} | wc -l`
if [ "$count" = 0 ]; then
        echo "No docker image"
        # cd /home/cdmmgr/cdm/
        # fuser -k -n tcp 48011
        _cdm_start
fi
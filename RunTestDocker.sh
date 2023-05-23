#!/bin/bash
PROCESS="MOS-CDM-COMPOSE"

count=`docker ps | grep -v grep | grep "$PROCESS" | awk {'print $5'} | wc -l`
if [ "$count" = 0 ]; then
        echo "No docker image"
        # cd /home/cdmmgr/cdm/
        # fuser -k -n tcp 48011
        docker-compose -f /home/shifter/Docker/cdm/containers/compose/Test-CDM-config.yaml up -d

        sleep 2
        docker exec -dt MOS-CDM-COMPOSE  rm /var/run/crond.pid
        sleep 2
        docker exec -dt MOS-CDM-COMPOSE crond 
fi

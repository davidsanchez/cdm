#!/bin/bash
PROCESS="MOS-CDM-COMPOSE"

count=`docker ps | grep -v grep | grep "$PROCESS" | awk {'print $5'} | wc -l`
if [ "$count" = 0 ]; then
        echo "No docker image"
        # cd /home/cdmmgr/cdm/
        # fuser -k -n tcp 48011
        docker-compose -f /home/shifter/david_tmp/cdm/containers/compose/Prod-CDM-config.yaml up -d
        sleep 20
        docker exec -ti MOS-CDM-COMPOSE bash& > /dev/null 
fi
#!/bin/bash
#source /home/mosmgr/MOS.conf
PROCESS="MOS_Device -d Unit_CDM_Model.xml -p 48011 -i"

count=`ps aux | grep -v grep | grep "$PROCESS" | awk {'print $5'} | wc -l`
if [ "$count" = 0 ]; then
        echo "No Serveur_OPCUA instance, starting..."
        cd /home/cdmmgr/cdmdevice/
        fuser -k -n tcp 48011
        ./RunCDM.sh
fi
#!/bin/bash
#source /home/mosmgr/MOS.conf
PROCESS="MOS_Device -d Unit_SG_Model.xml -p 48012 -i"

count=`ps aux | grep -v grep | grep "$PROCESS" | awk {'print $5'} | wc -l`
if [ "$count" = 0 ]; then
        echo "No Serveur_OPCUA instance, starting..."
        cd /home/cdmmgr/
        fuser -k -n tcp 48012
        ./RunSG.sh
fi

#!/bin/bash
#
source /home/cdmmgr/env.bash



declare -x GENICAM_GENTL64_PATH="/opt/ids-peak_2.19.0.0-260_amd64/lib/x86_64-linux-gnu/ids-peak/cti"

declare -x IDSPEAK_BASE_PATH="/opt/ids-peak_2.19.0.0-260_amd64"

declare -x LD_LIBRARY_PATH="/home/mosmgr/MOS/MOS_X86_13.1.4a9/lib::/opt/ids-peak_2.19.0.0-260_amd64/lib/x86_64-linux-gnu:/home/cdmmgr/YoctoMeteo_cpp/Binaries/linux/64bits"

declare -x MOS_LIB_PATH="/home/mosmgr/MOS/MOS_X86_13.1.4a9/lib"
declare -x MOS_PATH="/home/mosmgr/MOS/MOS_X86_13.1.4a9/bin"

declare -x PATH="/home/mosmgr/MOS/MOS_X86_13.1.4a9/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/ids-peak_2.19.0.0-260_amd64/bin"


declare -x XDG_DATA_DIRS="/root/.local/share/flatpak/exports/share:/var/lib/flatpak/exports/share:/usr/local/share:/usr/share"
declare -x YOCTO_PATH="/home/cdmmgr/YoctoMeteo_cpp"

PROCESS="MOS_Device -d Unit_CDM_Model.xml -p 48011 -i"

count=`ps aux | grep -v grep | grep "$PROCESS" | awk {'print $5'} | wc -l`
if [ "$count" = 0 ]; then
        echo "No Serveur_OPCUA instance, starting..."
        cd /home/cdmmgr/
        fuser -k -n tcp 48011
        ./RunCDM.sh
fi
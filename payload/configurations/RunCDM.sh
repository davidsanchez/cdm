ulimit -n 1024
source /home/mosmgr/MOS.conf
export CDMPATH=/home/cdmmgr
MOS_Device -d Unit_CDM_Model.xml -p 48011 -i

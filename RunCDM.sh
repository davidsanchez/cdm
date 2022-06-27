# cp /home/cdmmgr/cdm/payload/configurations/id_rsa /root/.ssh/
# chmod 600  /root/.ssh/id_rsa

source /home/cdmmgr/MOS.conf
MOS_Device -d Unit_CDM_Model.xml -p 48011 -i

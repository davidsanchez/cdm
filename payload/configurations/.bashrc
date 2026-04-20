# .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
  . /etc/bashrc
fi

# User specific aliases and functions

# export JAVA_HOME=/etc/alternatives/java_sdk_1.8.0
# if [[ $- =~ "i" ]]; then
#   echo Setting JAVA_HOME to $JAVA_HOME
# fi

# add IDS setup
export IDSPEAK_BASE_PATH=/opt/ids-peak_2.19.0.0-260_amd64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$IDSPEAK_BASE_PATH/lib/x86_64-linux-gnu
export PATH=$PATH:$IDSPEAK_BASE_PATH/bin
export GENICAM_GENTL64_PATH=$IDSPEAK_BASE_PATH/lib/x86_64-linux-gnu/ids-peak/cti

# ADD MOS setup
source /home/mosmgr/MOS.conf
export CDMPATH='/home/cdmmgr'
alias ti='ls -lrth'

# Yocto setup
export YOCTO_PATH=/home/cdmmgr/YoctoMeteo_cpp

#crond
#crontab CDMCrontab

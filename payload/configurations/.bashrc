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

# ADD MOS setup
source /home/mosmgr/MOS.conf
export CDMPATH='/home/cdmmgr'
alias ti='ls -lrth'

#crond
#crontab CDMCrontab

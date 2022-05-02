#FROM centos:7

FROM common-mos:v9

# Creating a cdmmgr user
USER root
RUN groupadd -g 1003 cdmmgr && \
    useradd -g 1002 -u 1003 -d /home/cdmmgr -m -s /bin/bash cdmmgr
 
RUN yum clean all && yum update -y 
RUN yum install -y cronie && yum install -y sudo

RUN yum install -y git
# Installing the development software
RUN yum install -y \
     openssh-server \
     xauth \
    gcc-c++ \
    gdb \
    git \
    subversion \
    autoconf \
    epel-release \
    make \
    cmake  \ 
    patch \
    wget \
    curl \
    mc \
    curl-devel \
    CCfits-devel \
    libxml2-devel \
    gdb-gdbserver \
    boost-devel \
    boost169-devel \
    gedit \
     opencv  \
     opencv-devel  \
     opencv-python

USER cdmmgr
WORKDIR /home/cdmmgr


RUN mkdir -m 700 -p .ssh/

COPY --chown=cdmmgr:cdmmgr payload/configurations/.ssh/ .ssh/
RUN mkdir -m 700 -p .ssh/
RUN chmod 640 .ssh/authorized_keys

USER root
RUN ssh-keygen -b 4096 -f /etc/ssh/ssh_host_ed25519_key -t ed25519 -N ""
RUN ssh-keygen -b 521 -f  /etc/ssh/ssh_host_ecdsa_key -t ecdsa -N ""
RUN ssh-keygen -b 4096 -f /etc/ssh/ssh_host_rsa_key -t rsa -N ""


USER root

COPY payload/configurations/MOSStart.sh ./
COPY payload/configurations/.bashrc .bashrc
#USER cdmmgr

ENTRYPOINT ["/home/cdmmgr/MOSStart.sh"]

#RUN (cd /lib/systemd/system/sysinit.target.wants/; for i in *; do [ $i == \
#systemd-tmpfiles-setup.service ] || rm -f $i; done); \
#rm -f /lib/systemd/system/multi-user.target.wants/*;\
#rm -f /etc/systemd/system/*.wants/*;\
#rm -f /lib/systemd/system/local-fs.target.wants/*; \
#rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
#rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
#rm -f /lib/systemd/system/basic.target.wants/*;\
#rm -f /lib/systemd/system/anaconda.target.wants/*;
#VOLUME [ "/sys/fs/cgroup" ]
#CMD ["/usr/sbin/init"]
RUN git clone https://gitlab-ci-token:58TWWXkdzsdrEhQFm-pG@gitlab.cta-observatory.org//cta-array-elements/lst/subsystems/aux/cdm.git /home/cdmmgr/cdm/
RUN git clone https://github.com/yoctopuce/yoctolib_cpp.git /home/cdmmgr/YoctoMeteo_cpp


#USER cdmmgr
RUN mkdir ./cdm/build
COPY  payload/CMakeLists.txt ./cdm/build

#USER root


COPY payload/ids-software-suite-linux-64-4.95.2-archive ./
RUN yes | ./ueye_4.95.2.1886_amd64.run 

WORKDIR /home/cdmmgr/cdm/build 
RUN cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/home/mosmgr/MOS/MOS_X86_9.3.0/
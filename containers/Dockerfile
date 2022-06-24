#FROM centos:7

FROM lst/common-mos:v9

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
    libusbx-devel \
    curl-devel \
    CCfits-devel \
    libxml2-devel \
    gdb-gdbserver \
    #boost-devel \
    #boost169-devel \
    gedit \
    libatomic \
    vim \
    screen \
    python-devel \
     opencv  \
     opencv-devel  \
     opencv-python

USER cdmmgr
WORKDIR /home/cdmmgr


RUN mkdir -m 700 -p .ssh/

#COPY --chown=cdmmgr:cdmmgr payload/configurations/.ssh/ .ssh/
#RUN mkdir -m 700 -p .ssh/
#RUN chmod 640 .ssh/authorized_keys


USER root

COPY --chown=cdmmgr:cdmmgr payload/configurations/id_rsa $HOME/.ssh/
RUN chmod 600 $HOME/.ssh/id_rsa



RUN ssh-keygen -b 4096 -f /etc/ssh/ssh_host_ed25519_key -t ed25519 -N ""
RUN ssh-keygen -b 521 -f  /etc/ssh/ssh_host_ecdsa_key -t ecdsa -N ""
RUN ssh-keygen -b 4096 -f /etc/ssh/ssh_host_rsa_key -t rsa -N ""

#USER root

COPY payload/configurations/MOSStart.sh /home/cdmmgr/MOSStart.sh
COPY payload/configurations/MOS.conf /home/cdmmgr/MOS.conf
#RUN ["source /home/cdmmgr/MOS.conf"]
COPY payload/configurations/.bashrc /home/cdmmgr/.bashrc
#TO BE FIXED

ENTRYPOINT ["/home/cdmmgr/MOSStart.sh"]

RUN git clone https://gitlab-ci-token:58TWWXkdzsdrEhQFm-pG@gitlab.cta-observatory.org//cta-array-elements/lst/subsystems/aux/cdm.git /home/cdmmgr/cdm/
RUN git clone https://github.com/yoctopuce/yoctolib_cpp.git /home/cdmmgr/YoctoMeteo_cpp
WORKDIR /home/cdmmgr/YoctoMeteo_cpp/Binaries/
RUN make
WORKDIR /home/cdmmgr/YoctoMeteo_cpp/Binaries/linux/64bits
RUN ln -s libyocto.so.1.0.1 libyocto.so


WORKDIR /home/cdmmgr/
RUN mkdir ./output
RUN mkdir ./output/images
RUN mkdir ./output/fits
RUN mkdir ./cdm/build

COPY  payload/CMakeLists.txt ./cdm/build

#USER root


COPY payload/uEye-Linux-4.90.06-64 ./
RUN yes | ./ueyesdk-setup-4.90.06-usb-amd64.gz.run

#COPY payload/uEye-Linux-4.81-64-bit ./
#RUN yes | ./ueyesdk-setup-4.81-usb-amd64.gz.run


COPY payload/ueye.h /usr/include/ueye.h
RUN mkdir ./boost
COPY payload/boost_1_79_0 ./boost

WORKDIR /home/cdmmgr/boost
RUN ./bootstrap.sh --with-python=python2.7
RUN  ./b2 install  --with=all

WORKDIR /home/cdmmgr/cdm/build
RUN cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/home/mosmgr/MOS/MOS_X86_9.3.0/
RUN make

WORKDIR /home/cdmmgr

CMD ["source /home/cdmmgr/MOS.conf"]

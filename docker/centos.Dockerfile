FROM centos:7

RUN yum update -y
RUN yum groupinstall -y "Development Tools"
RUN yum install -y centos-release-scl  epel-release cmake3 boost-static git libXtst-devel qt5-qtbase-devel qt5-qtdeclarative-devel libcurl-devel openssl-devel
RUN yum update -y
RUN yum install -y devtoolset-6
ENTRYPOINT source /opt/rh/devtoolset-6/enable && /bin/bash

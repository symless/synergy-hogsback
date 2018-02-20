FROM centos:7

<<<<<<< Updated upstream
RUN yum update -y
RUN yum groupinstall -y "Development Tools"
RUN yum install -y centos-release-scl  epel-release cmake3 boost-static git libXtst-devel qt5-qtbase-devel qt5-qtdeclarative-devel libcurl-devel openssl-devel
RUN yum update -y
RUN yum install -y devtoolset-6
=======
RUN yum -y update
RUN yum -y groupinstall "Development Tools"
RUN yum -y install centos-release-scl epel-release  boost-static git libXtst-devel qt5-qtbase-devel qt5-qtdeclarative-devel libcurl-devel openssl-devel
RUN yum -y update
RUN yum -y install devtoolset-6 cmake3
>>>>>>> Stashed changes
ENTRYPOINT source /opt/rh/devtoolset-6/enable && /bin/bash

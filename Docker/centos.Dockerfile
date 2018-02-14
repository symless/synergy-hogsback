FROM centos:7

RUN yum update -y
RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-6

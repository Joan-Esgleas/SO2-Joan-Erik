FROM ubuntu:22.04

ARG UID
ARG GID

RUN apt-get update \
    # Needed tools to compile bochs...
    && apt-get -y install build-essential \
    && apt-get -y install libx11-dev \
    && apt-get -y install libgtk2.0-dev \
    && apt-get -y install libreadline-dev \
    # Needed tools for Zeos
    && apt-get -y install vim \
    && apt-get -y install bin86 \
    && apt-get -y install gdb \
    && apt-get -y install git \
    && apt-get -y install wget \
    # Forward X11
    && apt-get install -y openssh-server xauth \
    && mkdir -p /var/run/sshd \
    && mkdir -p /root/.ssh \
    && chmod 700 /root/.ssh \
    # Download BOCHS
    && mkdir -p tmp \
    && cd tmp \
    && VERS=2.6.7 \
    && BOCHS=bochs-${VERS} \
    && wget http://sourceforge.net/projects/bochs/files/bochs/${VERS}/${BOCHS}.tar.gz/download -O ${BOCHS}.tar.gz \
    && tar zxf ${BOCHS}.tar.gz \
    # Compile BOCHS...
    # ... without GDB
    && mkdir -p build \
    && cd build \
    && LDFLAGS=-pthread ../${BOCHS}/configure --enable-debugger --enable-disasm --enable-x86-debugger --enable-readline --with-x --prefix=/opt/bochs \
    && make \
    && make install \
    && cd .. \
    # ... with GDB
    && mkdir -p build-gdb \
    && cd build-gdb \
    && ../${BOCHS}/configure --enable-gdb-stub --with-x --prefix=/opt/bochs_gdb \
    && make \
    && make install \
    && cd .. \
    # Create links to bochs binaries
    && ln -sf /opt/bochs/bin/bochs /usr/local/bin/bochs_nogdb \
    && ln -sf /opt/bochs_gdb/bin/bochs /usr/local/bin/bochs \
    && cd .. \
    # Clean environment
    && rm -rf tmp \
    && groupadd -g ${GID} ubuntu && useradd -m -u ${UID} -g ${GID} ubuntu

# Set the non-root user as the default user
USER ubuntu

# Set the working directory
WORKDIR /home/ubuntu/

FROM debian:stretch

ENV DEPOT_TOOLS_PATH $HOME/depot_tools
ENV BUILDROOT_PATH $HOME/buildroot
ENV ENGINE_PATH $HOME/engine
ENV PATH $PATH:$DEPOT_TOOLS_PATH

# Notes:
# - libx11-dev is used by Flutter for desktop linux (see also install-build-deps-linux-desktop.sh)
# - chrome is used by Flutter for web.

RUN apt-get update && \
    apt-get install -y git wget curl unzip python lsb-release sudo apt-transport-https && \
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git $DEPOT_TOOLS_PATH && \
    mkdir --parents $ENGINE_PATH && \
    git clone https://github.com/flutter/buildroot.git $BUILDROOT_PATH && \
    cd $BUILDROOT_PATH && \
    apt-get install -y autoconf automake bison flex gettext gperf groff libtool pkg-config acpica-tools build-essential dejagnu \
      dosfstools ifupdown libedit-dev libglib2.0-dev liblz4-tool libncurses5-dev libssl-dev lsof mtools nasm net-tools python2.7-dev \
      python-m2crypto tcpdump texinfo uuid-dev vim xz-utils zlib1g-dev firefox-esr g++-multilib libblkid-dev libc6-dev-i386 \
      libfreetype6-dev libegl1-mesa libgles2-mesa-dev libglu1-mesa-dev libgtk-3-dev libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev \
      libxxf86vm-dev mesa-common-dev openjdk-8-jdk zip xvfb ca-certificates gnupg

# Add repo for chrome stable
RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN echo 'deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main' | tee /etc/apt/sources.list.d/google-chrome.list

# Add repo for gcloud sdk and install it
RUN echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] https://packages.cloud.google.com/apt cloud-sdk main" | \
    tee -a /etc/apt/sources.list.d/google-cloud-sdk.list

RUN curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | \
    apt-key --keyring /usr/share/keyrings/cloud.google.gpg add -

RUN apt-get update && apt-get install -y google-cloud-sdk && \
    gcloud config set core/disable_usage_reporting true && \
    gcloud config set component_manager/disable_update_check true

# This is tested on a "Windows Server version 1803 Datacenter Core for Containers" VM on GCE.
# It's known that other versions such as 1809 and Windows Server 2019 won't work with this.
#
# Note that "\" is the escape character. Use "\\"" for windows path separator. In many cases,
# "/" can also be used as the separator in windows.

FROM microsoft/windowsservercore:1803

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

# Required for gclient sync on non-corp windows (otherwise it'll ask for Chromium account)
ENV DEPOT_TOOLS_WIN_TOOLCHAIN 0

# The following environment variable is required to get the vcvarsall.bat in our build/toolchain/win/setup_toolchain.py
ENV GYP_MSVS_OVERRIDE_PATH "c:/Program Files (x86)/Microsoft Visual Studio/2017/Community"

# Install git, vs2017, debugger tools, depot tools, and set environment variables.
# Do everything in a big RUN to save layers and Docker image size.
ADD https://github.com/git-for-windows/git/releases/download/v2.21.0.windows.1/MinGit-2.21.0-64-bit.zip MinGit.zip
ADD https://aka.ms/vs/15/release/vs_community.exe vs_community.exe
ADD https://download.microsoft.com/download/5/C/3/5C3770A3-12B4-4DB4-BAE7-99C624EB32AD/windowssdk/winsdksetup.exe winsdksetup.exe
ADD https://storage.googleapis.com/chrome-infra/depot_tools.zip depot_tools.zip
RUN powershell Expand-Archive -LiteralPath MinGit.zip MinGit & \
    powershell Expand-Archive -LiteralPath c:/depot_tools.zip -DestinationPath c:/depot_tools & \
    vs_community.exe --passive --wait --add Microsoft.VisualStudio.Workload.NativeCrossPlat \
        --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended & \
    winsdksetup.exe /features OptionId.WindowsDesktopDebuggers /q & \
    setx PATH "c:/MinGit/cmd/;c:/depot_tools/;%PATH%"

RUN mkdir flutter\\engine
WORKDIR c:/flutter/engine
ADD https://raw.githubusercontent.com/flutter/engine/master/ci/docker/build/engine_gclient .gclient
RUN gclient sync

WORKDIR c:/flutter/engine/src

# Currently there's problem with gn --unoptimized so we omit --unoptimized for now
RUN python flutter/tools/gn
RUN ninja -C out/host_debug

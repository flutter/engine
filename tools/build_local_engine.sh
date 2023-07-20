#!/bin/bash

##############################################
# build.sh
#
# Bash script to compile the engine optionally using goma (from macOS)
#
# See https://github.com/flutter/flutter/wiki/Compiling-the-engine
##############################################
# Set DRY_RUN to 0 to see what commands will be run without actually executing them, 1 otherwise
DRY_RUN=1
PATH_TO_ENGINE=${FLUTTER_ENGINE}
goma=""
jflag=""

bold=$(tput bold)
normal=$(tput sgr0)
under=$(tput smul)
bg=$(tput setab 4)
fg=$(tput setaf 7)

opts=("1" "2" "3" "4" "5" "6" "7")
builds=("android_debug_unopt" "android_debug_unopt_arm64" "android_debug_unopt_x86" "android_debug_unopt_x64" "ios_debug_unopt" "ios_debug_sim_unopt" "ios_debug_sim_unopt_arm64")
desc=("for device-side Android executables" "for newer 64-bit Android devices" "for x86 Android emulators" "for x64 Android emulators" "for device-side iOS executables" "for iOS Simulators" "for iOS simulators on arm64 Mac")
selections=(" " " " " " " " " " " " " ")
gnargs=(" --android" " --android --android-cpu arm64" " --android --android-cpu x86" " --android --android-cpu x64" " --ios" " --ios --simulator" " --ios --simulator --simulator-cpu=arm64")

function cursor_pos() {
  exec < /dev/tty
  oldstty=$(stty -g)
  stty raw -echo min 0
  echo -en "\033[6n" > /dev/tty
  IFS=';' read -r -d R -a pos
  stty $oldstty
  row=$((${pos[0]:2} - 1))
  col=$((${pos[1]} - 1))
}

function show_menu() {
  echo "Select local engine build(s):"
  for i in ${!opts[@]}; do
    echo "[${selections[$i]}] ${bold}${bg}${fg} ${opts[$i]} ${normal} ${desc[$i]} (${under}${builds[$i]}${normal})"
  done
  echo "${bold}q${normal} to quit"
}

function toggle() {
  if [[ "${selections[$1]}" == "X" ]]; then
    selections[$1]=" "
  else
    selections[$1]="X"
  fi
  local up=$((${#opts[@]}-$1+1))
  cursor_pos
  new_row=$(($row-$up))
  tput cup $(($new_row)) 1
  echo "${selections[$1]}"
  tput rc
}

function prompt() {
  while :
  do
    tput sc
    read -n1 -p "Check an option (again to uncheck, ENTER when done): " choice
    if [[ ${choice} = "" ]]; then
       break
    elif [[ ${choice} = "q" ]]; then
       echo -e "\nDone"
       exit 0
    elif [[ " ${opts[*]} " = *" $choice "* ]]; then
       toggle $((choice - 1))
    else
       echo -e "\nInvalid option"
       exit 1
    fi
  done
}

function update() {

  # Update the Flutter Engine repo
  cd "${PATH_TO_ENGINE}/flutter"
  git pull upstream main
}

build() {

  # Exit if there are no selections
  selected=0

  for i in ${!selections[@]}; do
    if [[ "${selections[$i]}" == "X" ]]; then
       selected=1 ;
    fi
  done

  if [[ ${selected} -eq 0 ]]; then
    echo "Nothing checked. Exiting."
    exit 0
  fi

  # Switch to the engine directory
  echo "cd ${PATH_TO_ENGINE}/.."
  cd "${PATH_TO_ENGINE}/.."

  # Update dependencies
  echo "gclient sync"
  if [[ DRY_RUN -ne 0 ]]; then
    gclient sync
  fi

  # Switch to the engine source directory
  echo "cd src"
  if [[ DRY_RUN -ne 0 ]]; then
    cd src
  fi

  # Prepare and build selected executable(s)
  for i in ${!selections[@]}; do
    if [[ "${selections[$i]}" == "X" ]]; then
      echo "./flutter/tools/gn --unoptimized${gnargs[$i]}${goma}"
      if [[ DRY_RUN -ne 0 ]]; then
        ./flutter/tools/gn --unoptimized${gnargs[$i]}${goma}
      fi
      echo "ninja ${jflag}-C out/${builds[$i]}"
      if [[ DRY_RUN -ne 0 ]]; then
        ninja ${jflag}-C out/${builds[$i]}
      fi
    fi
  done

  # Prepare and build host executable
  echo "./flutter/tools/gn --unoptimized${goma}"
  if [[ DRY_RUN -ne 0 ]]; then
    ./flutter/tools/gn --unoptimized${goma}
  fi
  echo "ninja ${jflag}-C out/host_debug_unopt"
  if [[ DRY_RUN -ne 0 ]]; then
    ninja ${jflag}-C out/host_debug_unopt
  fi
}

while getopts 'up:g:h' arg; do
  case "$arg" in
    u)
      update
      ;;
    p)
      PATH_TO_ENGINE="${OPTARG}"
      ;;
    g)
      goma=" --xcode-symlinks --goma"
      jflag="-j ${OPTARG} "
      ;;
    h)
      echo "usage: $(basename $0) [-u -p (optional path to engine, defaults to \$FLUTTER_ENGINE) -g (optional number of parallel goma jobs)]"
      exit 1
      ;;
  esac
done
shift "$(($OPTIND -1))"

if [ -z "${PATH_TO_ENGINE}" ]; then
  echo "You must either set the \$FLUTTER_ENGINE environment variable to the path of your engine or run again with -p"
  exit 1
else
  echo "Building engine at ${PATH_TO_ENGINE}"
fi

show_menu
prompt
build

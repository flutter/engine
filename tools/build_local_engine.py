# Copyright 2023 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Script to compile the engine, optionally using goma (from macOS)
#
# See https://github.com/flutter/flutter/wiki/Compiling-the-engine
import os
import importlib.machinery, importlib.util
import curses
from signal import signal, SIGWINCH
import sys
import argparse
from os.path import dirname as up
import subprocess
import shlex

TOOLS_DIR = up(__file__)
loader = importlib.machinery.SourceFileLoader('gn', TOOLS_DIR + '/gn')
spec = importlib.util.spec_from_loader(loader.name, loader)
gn = importlib.util.module_from_spec(spec)
loader.exec_module(gn)

# Set DRY_RUN to 'True' to see what commands will be run without
# actually executing them, 'False' otherwise
DRY_RUN = False

path = os.environ.get("FLUTTER_ENGINE")
jflag = ""
stdscr = None
resize = True

OPTION_1 = "0"
OPTION_2 = "1"
OPTION_3 = "2"
OPTION_4 = "3"
OPTION_5 = "4"
OPTION_6 = "5"
OPTION_7 = "6"

opts = [OPTION_1, OPTION_2, OPTION_3, OPTION_4, OPTION_5, OPTION_6, OPTION_7]
builds = [
    "android_debug_unopt", "android_debug_unopt_arm64",
    "android_debug_unopt_x86", "android_debug_unopt_x64", "ios_debug_unopt",
    "ios_debug_sim_unopt", "ios_debug_sim_unopt_arm64"
]
desc = [
    "for device-side Android executables", "for newer 64-bit Android devices",
    "for x86 Android emulators", "for x64 Android emulators",
    "for device-side iOS executables", "for iOS Simulators",
    "for iOS simulators on arm64 Mac"
]
selections = [" ", " ", " ", " ", " ", " ", " "]

row = 0
col = 0
HEADER = "=" * 72


# When the terminal window is resized, screen.getkey sometimes throws
# an error. The error should be handled safely instead of terminating.
def safe_getkey(stdscr):
  try:
    c = stdscr.getch()
    if c == -1:
      return ''
    return curses.keyname(c).decode('UTF-8')
  except:
    return ''


# When the terminal window is too small to advance the cursor,
# an error is thrown. The error should be handled safely instead of
# terminating.
def safe_addstr(stdscr, row, col, text, attr=None):
  try:
    if attr == None:
      stdscr.addstr(row, col, text)
    else:
      stdscr.addstr(row, col, text, attr)
  except curses.error:
    pass


def show_menu():
  global stdscr, row, col

  curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_BLUE)
  stdscr.clear()

  # center the menu
  row = curses.LINES // 2 - 5
  col = curses.COLS // 2
  col = col - len(HEADER) // 2

  safe_addstr(stdscr, row, col, HEADER)
  row += 2
  col += 1
  safe_addstr(stdscr, row, col, "Select local engine build(s):")
  row += 2
  newrow = row
  for i in range(len(opts)):
    brackets = " [" + selections[i] + "] "
    safe_addstr(stdscr, newrow, col, brackets)
    newcol = col + len(brackets)
    option = " " + opts[i] + " "
    safe_addstr(stdscr, newrow, newcol, option, curses.color_pair(1))
    newcol += len(option)
    description = " " + desc[i]
    safe_addstr(stdscr, newrow, newcol, description)
    newcol += len(description)
    start = " ("
    safe_addstr(stdscr, newrow, newcol, start)
    newcol += len(start)
    build = builds[i]
    safe_addstr(stdscr, newrow, newcol, build, curses.A_UNDERLINE)
    newcol += len(build)
    safe_addstr(stdscr, newrow, newcol, ")")
    newrow += 1
  stdscr.refresh()


def toggle_selection(i):
  if selections[i] == "X":
    selections[i] = " "
  else:
    selections[i] = "X"


def toggle(i, stdscr):
  toggle_selection(i)
  safe_addstr(stdscr, row + i, col + 2, selections[i])
  stdscr.refresh()


def show_prompt():
  global stdscr, row, HEADER, resized

  newrow = row + len(opts) + 1
  safe_addstr(
      stdscr, newrow, col,
      "Type a number to check an option (again to uncheck, ENTER when done)"
  )
  stdscr.refresh()
  newrow += 2
  safe_addstr(stdscr, newrow, col - 1, HEADER)


def init_screen():
  global stdscr
  stdscr = curses.initscr()
  curses.start_color()
  curses.cbreak()
  curses.noecho()
  stdscr.keypad(True)
  curses.curs_set(False)


def resize(signum, frame):
  global stdscr
  if not resize:
    return
  curses.endwin()
  stdscr = curses.initscr()
  show_menu()
  show_prompt()


def restore_terminal():
  global stdscr
  curses.nocbreak()
  curses.echo()
  stdscr.keypad(False)
  curses.curs_set(True)
  curses.endwin()
  resize = False


def call(command):
  try:
    return subprocess.call(shlex.split(command))
  except subprocess.CalledProcessError as error:
    print('Failed to execute command: ', error.returncode, error.output)
    return error.returncode


def build():
  if "X" not in selections:
    print("Nothing selected. Exiting.")
    return 0

  # Switch to the engine directory
  print("cd " + path)
  if not DRY_RUN:
    try:
      os.chdir(path)
    except:
      print("Unable to change directory - ", sys.exc_info())
      return 1

  # Update dependences
  print("gclient sync")
  if not DRY_RUN:
    return_code = call("gclient sync")
    if return_code != 0:
      print("An unexpected error occurred when executing gclient. Terminating.")
      return return_code

  # Switch to the engine source directory
  print("cd src")
  if not DRY_RUN:
    os.chdir("src")

  # Prepare and build selected executable(s)
  for i, selection in enumerate(selections):
    if selection == 'X':
      gn_args = build_gn_args(i)
      if not DRY_RUN:
        return_code = gn.run(gn_args)
        if return_code != 0:
          print(
              "An unexpected error occurred when executing gn for selected build. Terminating."
          )
          return return_code

      command = "ninja " + jflag + "-C out/" + builds[i]
      print(command)
      if not DRY_RUN:
        return_code = call(command)
        if return_code != 0:
          print(
              "An unexpected error occurred when executing ninja for selected build. Terminating."
          )
          return return_code

  # Prepare and build host executable
  gn_args = build_gn_args()
  if not DRY_RUN:
    return_code = gn.run(gn_args)
    if return_code != 0:
      print(
          "An unexpected error occured when executing gn for the host build. Terminating."
      )
      return return_code

  command = "ninja " + jflag + "-C out/host_debug_unopt"
  print(command)
  if not DRY_RUN:
    return_code = call(command)
    if return_code != 0:
      print(
          "An unexpected error occurred when executing ninja for the host build. Terminating."
      )
      return return_code

  return 0


def handle_selection():
  choice = ""
  enter = '^J'
  while choice != enter:
    choice = safe_getkey(stdscr)
    if choice in opts:
      toggle(opts.index(choice), stdscr)

  restore_terminal()
  return build()


def build_gn_args(i=None):
  gn_parser = gn.init_parser()
  args, unknown = gn_parser.parse_known_args()
  args.unoptimized = True

  command = path + '/src/flutter/tools/gn --unoptimized'
  args.unoptimized = True

  if jflag != "":
    args.goma = True
    args.xcode_symlinks = True
    command += " --goma"

  if i in range(len(opts)):

    option = opts[i]

    if option == OPTION_1:
      command += ' --android'
      args.target_os = 'android'
    elif option == OPTION_2:
      command += ' --android --android-cpu arm64'
      args.target_os = 'android'
      args.android_cpu = 'arm64'
    elif option == OPTION_3:
      command += ' --android --android-cpu x86'
      args.target_os = 'android'
      args.android_cpu = 'x86'
    elif option == OPTION_4:
      command += ' --android --android-cpu x64'
      args.target_os = 'android'
      args.android_cpu = 'x64'
    elif option == OPTION_5:
      command += ' --ios'
      args.target_os = 'ios'
    elif option == OPTION_6:
      command += ' --ios --simulator'
      args.target_os = 'ios'
      args.simulator = True
    elif option == OPTION_7:
      command += ' --ios --simulator --simulator-cpu=arm64'
      args.target_os = 'ios'
      args.simulator = True
      args.simulator_cpu = 'arm64'

  print(command)

  return args


def valid_requirements(argv):
  global path, jflag

  valid = True

  from shutil import which

  gclient = which("gclient")
  if gclient is None:
    print("gclient is not installed on the path.")
    valid = False

  ninja = which("ninja")
  if ninja is None:
    print("ninja is not installed on the path.")
    valid = False

  if not valid:
    return valid

  if (path) is None:
    path = up(up(up(TOOLS_DIR)))
    print(
        "The FLUTTER_ENGINE environment variable is not set, defaulting to " +
        path
    )
  else:
    path = up(path)

  build_parser = argparse.ArgumentParser(
      description='A script to build the Flutter engine.'
  )
  build_parser.add_argument(
      '--jobs',
      dest='jobs',
      type=str,
      default=None,
      help='optional number of parallel goma jobs'
  )
  build_parser.add_argument(
      '-j',
      dest='jobs',
      type=str,
      default=None,
      help='optional number of parallel goma jobs'
  )
  args = build_parser.parse_args(argv[1:])
  if args.jobs is not None:
    jflag = "-j " + args.jobs + " "

  return valid


def main(argv):

  if not valid_requirements(argv):
    print("One or more requirements not met. Exiting.")
    return 1

  try:
    init_screen()
    # Handle resize events
    signal(SIGWINCH, resize)
    show_menu()
    show_prompt()
    return handle_selection()
  except:
    restore_terminal()


if __name__ == '__main__':
  sys.exit(main(sys.argv))

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
from curses import initscr, endwin
from signal import signal, SIGWINCH
import sys
import threading

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
loader = importlib.machinery.SourceFileLoader('gn', TOOLS_DIR + '/gn')
spec = importlib.util.spec_from_loader(loader.name, loader)
gn = importlib.util.module_from_spec(spec)
loader.exec_module(gn)

# Set DRY_RUN to 'True' to see what commands will be run without
# actually executing them, 'False' otherwise
DRY_RUN = False
PATH_TO_ENGINE = os.environ.get("FLUTTER_ENGINE")

goma = False
jflag = ""
stdscr = None

opts = ["1", "2", "3", "4", "5", "6", "7"]
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

t_pick = None


def resize(signum, frame):
  global stdscr
  curses.endwin()
  stdscr = curses.initscr()
  show_menu()
  show_prompt()


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


def restore_terminal():
  global stdscr
  curses.nocbreak()
  curses.echo()
  stdscr.keypad(False)
  curses.curs_set(True)
  curses.endwin()


def build():
  selected = False

  for i, selection in enumerate(selections):
    if selection == 'X':
      selected = True
      gn_args = build_gn_args(i)
      if not DRY_RUN:
        gn.run(gn_args)

  if not selected:
    print('Nothing selected. Exiting.')


def pick():
  choice = ""
  enter = '^J'
  while choice != enter:
    choice = safe_getkey(stdscr)
    if choice in opts:
      toggle(opts.index(choice), stdscr)

  if choice == enter:
    restore_terminal()
    build()


def build_gn_args(i):
  parser = gn.init_parser()
  args = parser.parse_args()
  args.unoptimized = True

  if goma:
    args.goma = True
    args.xcode_symlinks = True

  command = './flutter/tools/gn --unoptimized'
  args.unoptimized = True

  if goma:
    command += ' --goma'

  match i:
    case 0:
      print(command + ' --android')
      args.target_os = 'android'
    case 1:
      print(command + ' --android --android-cpu arm64')
      args.target_os = 'android'
      args.android_cpu = 'arm64'
    case 2:
      print(command + ' --android --android-cpu x86')
      args.target_os = 'android'
      args.android_cpu = 'x86'
    case 3:
      print(command + ' --android --android-cpu x64')
      args.target_os = 'android'
      args.android_cpu = 'x64'
    case 4:
      print(command + ' --ios\n')
      args.target_os = 'ios'
    case 5:
      print(command + ' --ios --simulator')
      args.target_os = 'ios'
      args.simulator = True
    case 6:
      print(command + ' --ios --simulator --simulator-cpu=arm64')
      args.target_os = 'ios'
      args.simulator = True
      args.simulator_cpu = 'arm64'

  return args


def main(argv):
  try:
    init_screen()
    # Handle resize events
    signal(SIGWINCH, resize)
    # TODO: check for goma from flags
    show_menu()
    show_prompt()
    t_pick = threading.Thread(target=pick)
    t_pick.start()
    t_pick.join()
  except:
    restore_terminal()


if __name__ == '__main__':
  sys.exit(main(sys.argv))

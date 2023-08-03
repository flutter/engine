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
from curses import wrapper, initscr, endwin
from signal import signal, SIGWINCH

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
loader = importlib.machinery.SourceFileLoader('gn', TOOLS_DIR + '/gn')
spec = importlib.util.spec_from_loader(loader.name, loader)
gn = importlib.util.module_from_spec(spec)
loader.exec_module(gn)

# Set DRY_RUN to 0 to see what commands will be run without actually executing them, 1 otherwise
DRY_RUN = 1
PATH_TO_ENGINE = os.environ.get("FLUTTER_ENGINE")
goma = ""
jflag = ""

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
gnargs = [
    " --android", " --android --android-cpu arm64",
    " --android --android-cpu x86", " --android --android-cpu x64", " --ios",
    " --ios --simulator", " --ios --simulator --simulator-cpu=arm64"
]

row = 0
col = 0
header = "========================================================================="

stdscr = None


def resize(signum, frame):
  global stdscr
  stdscr.erase()
  endwin()
  stdscr = initscr()
  show_menu(stdscr)
  prompt(stdscr)


# When the terminal window is resized, screen.getkey sometimes throws
# an error. The error should be handled safely instead of terminating.
def safe_getkey(stdscr):
  c = stdscr.getch()
  if c == -1:
    return ''
  return curses.keyname(c).decode('UTF-8')


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


def show_menu(stdscr):
  global row, col

  curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_BLUE)
  stdscr.clear()
  row = curses.LINES // 2 - 5
  col = curses.COLS // 2
  col = col - len(header) // 2
  safe_addstr(stdscr, row, col, header)
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


def prompt(stdscr):
  global row, header

  newrow = row + len(opts) + 1
  safe_addstr(
      stdscr, newrow, col,
      "Type a number to check an option (again to uncheck, ENTER when done)"
  )
  stdscr.refresh()
  newrow += 2
  safe_addstr(stdscr, newrow, col - 1, header)
  choice = ""
  enter = '^J'
  while choice != enter:
    choice = safe_getkey(stdscr)
    if choice in opts:
      toggle(opts.index(choice), stdscr)


def main(screen):
  global stdscr

  stdscr = screen
  curses.curs_set(False)
  # Handle resize events
  signal(SIGWINCH, resize)
  show_menu(stdscr)
  prompt(stdscr)


wrapper(main)
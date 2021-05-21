#!/bin/python

import argparse
import subprocess
import sys

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument(
      'input',
      help='path to input SPIR-V assembly file')
  parser.add_argument(
      'output',
      help='path to output SPIR-V binary file')
  args = parser.parse_args()
  return subprocess.call([
    'spirv-as',
    '-o',
    args.output,
    args.input,
  ])

if __name__ == '__main__':
  sys.exit(main())


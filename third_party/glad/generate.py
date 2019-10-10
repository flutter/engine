#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import sys
import argparse

def generate(output):
    dirname, _ = os.path.split(os.path.abspath(__file__))
    command = 'cd ' + dirname + '/scripts;'
    command += 'python -m glad --profile="compatibility" --api="gl=3.3" --generator="c" --spec="gl" --extensions="" --reproducible --quiet --out-path='
    command += output
    os.system(command)


def main():
    parser = argparse.ArgumentParser();
    parser.add_argument('-o', '--output', dest='output',
      required=True, help='The output directory for coverage results.')
    args = parser.parse_args()
    output = os.path.abspath(args.output)

    generate(output)

if __name__ == '__main__':
    main()

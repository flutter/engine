#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import sys

def main(argv):
    dirname, _ = os.path.split(os.path.abspath(__file__))
    output_path = argv[1]
    command = 'cd ' + dirname + '/scripts;'
    command += 'python -m glad --profile="compatibility" --api="gl=3.3" --generator="c" --spec="gl" --extensions="" --reproducible --out-path='
    command += output_path
    os.system(command)

if __name__ == '__main__':
    main(sys.argv)

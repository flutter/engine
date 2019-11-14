#!/usr/bin/python
# -*- coding: UTF-8 -*-

# GN can't run python scripts in `python -m scripts/glad` mode,
# so the generate.py file seems to be necessary.

import os
import sys
sys.path.append(os.path.split(os.path.abspath(__file__))[0] + '/scripts')
from glad import __main__

if __name__ == '__main__':
    __main__.main()
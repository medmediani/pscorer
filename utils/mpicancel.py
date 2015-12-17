#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, signal
#from itertools import izip
for jid in sys.argv[1:]:
    try:
        pid=int(file(jid.strip()).read().strip())
        os.kill(pid,  signal.SIGKILL)
    except IndexError:
        print "Give slurm job ID please"
    except IOError:
        print "No file named '%s'\nExecute this script in the directory where you started the job or provide full path" %(jid)

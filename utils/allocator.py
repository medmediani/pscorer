#!/usr/bin/env python
# -*- coding: utf-8 -*-
import codecs
import sys, os, socket
#from itertools import izip
import subprocess

#send job info to parent through PIPE, we get the info from Slurm ENV VARs
slurm_job_id=os.environ.get("SLURM_JOB_ID","")
slurm_n_nodes=os.environ.get("SLURM_NNODES","")
slurm_node_id=int(os.environ.get("SLURM_NODEID","0"))
slurm_cpus_per_node=os.environ.get("SLURM_CPUS_ON_NODE","")

if not slurm_job_id or not slurm_n_nodes:
    print >>sys.stderr, "Are you sure this program is submitted to slurm queue via 'srun'?"
    sys.exit(1)
    
if slurm_node_id==0:
    print "JOB ID:", slurm_job_id
    print "Number of nodes:", slurm_n_nodes
    print "Number of cores:", slurm_cpus_per_node
    
print socket.gethostname()
#print slurm_node_list
#make sure you send info
sys.stdout.flush()
sys.stderr.flush()
#save the OMPI output to a file
if slurm_node_id==0:
    output=file("mpi_out."+slurm_job_id, "w")
for line in sys.stdin:
    if slurm_node_id==0:
        print >>output, line,
        output.flush()
        
if slurm_node_id==0:
    output.close()
##prog_lines=sys.stdin.readlines()
##print "I got:", prog_lines
##if slurm_node_id==0:
##    print >>file("mpi_out."+slurm_job_id), prog_lines

#print "Removing:", "nodes."+slurm_job_id
#remove the machine file
try:
    pass
    os.remove("nodes."+slurm_job_id)
    os.remove(slurm_job_id)
except OSError:
    pass

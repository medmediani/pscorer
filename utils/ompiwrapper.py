#!/usr/bin/env python
# Written by M. Mediani
#8.3.2011

import sys, os 
import subprocess

#this will be the code to submit to the queue
allocator="/project/mt/user/mmediani/ompiwrap/allocator"
#mpirun binary
#executer="/home/mmediani/ddt/bin/ddt"
executer="/project/mt/user/mmediani/tools/mpi/openmpi/bin/mpirun"
wait=False
try:
    sys.argv.remove("-w")
    wait=True
except ValueError:
    try:
        sys.argv.remove("--wait-until-done")
        wait=True
    except ValueError:
        pass
try:
    srun_args=sys.argv[1]
except IndexError:
    srun_args=""
   
try:
    mpi_args=sys.argv[2]
except IndexError:
    mpi_args=""
#    
#print "wait=", wait
#
#print "args=", sys.argv
#exit()
cmd="srun "+srun_args+" "+allocator

print >>sys.stderr,"Executing:",cmd
#Requesting a job with the specified ressources on the queue
alloc=subprocess.Popen(cmd, shell=True,stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

node_list=[]
job_id=None
n_nodes=None
n_cores=None
#We exit if the job was not submitted correctly
if alloc.returncode:
    print >>sys.stderr,"Allocator unable to start, returned error:", alloc.communicate()[1]
    sys.exit(1)
#else:
#    print alloc.stderr.readlines()
print >>sys.stderr,"Waiting for resource allocation"
while True: #alloc.poll() is None:
    #this will block until the job is started
    line = alloc.stdout.readline()
    if not line:
        continue
    #get the job information
    if line.startswith("JOB ID:"):
        job_id=line.rsplit(None, 1)[-1].strip() 
#        print >> sys.stderr, "Job ID:", job_id
    elif  line.startswith("Number of nodes:"):
        n_nodes=int(line.rsplit(None, 1)[-1])
#        print >> sys.stderr, "Number of nodes:", n_nodes
    elif  line.startswith("Number of cores:"):
        n_cores=line.rsplit(None, 1)[-1].strip()
#        print >> sys.stderr, "Number of cores:", n_cores
    else:
        node_list.append(line)
    if job_id and n_nodes and n_cores:
        break
print >>sys.stderr,"Resources have been allocated"
#print "Nodes so far:", node_list
print >>sys.stderr,"The allocated job:",job_id

machine_file="nodes."+job_id
print >>sys.stderr,"The number of nodes:", n_nodes
print >>sys.stderr,"Number of cores per node:", n_cores
#save node to machine file
print >>sys.stderr,"Saving the machine file to:",machine_file
print >>file(machine_file,"w"),"".join(node_list+[alloc.stdout.readline() for _ in range(n_nodes-len(node_list))])

alloc.stdout=sys.stdout
alloc.stderr=sys.stdout
#inherit the current environment
env=os.environ.copy()
#add the OMP Env Var
if n_cores:
    env["OMP_NUM_THREADS"]=n_cores

#cmd= "python simulator.py"
#use the same ressources, make sure you inherit the OMP ENV VAR in all spawned processes
cmd=executer+(" -np %d "% n_nodes)+ ("-machinefile %s "%machine_file)+" -x OMP_NUM_THREADS -x LD_LIBRARY_PATH "+mpi_args
#cmd=executer+" "+mpi_args

print >>sys.stderr,"Executing:",cmd
#start the OMPI application, redirecting any output to the allocated job, in order to notice completion
prog=subprocess.Popen(cmd,  env=env,shell=True, stdout=alloc.stdin, stderr=alloc.stdin)
#prog.stdout.close()
#prog.stderr.close()
#the current way to end the process
print >>file(job_id, "w"),prog.pid
print >>sys.stderr,"Wrapper done"
if wait:
    print >>sys.stderr,"Waiting for submitted job to be done"
    prog.wait()
    
#alloc.wait()
#print alloc.communicate()

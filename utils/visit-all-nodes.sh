#! /bin/bash

for i in {1..36}; do echo "Visiting i13hpc$i"; ssh -oStrictHostKeyChecking=no  i13hpc$i -f 'ls' > /dev/null;done

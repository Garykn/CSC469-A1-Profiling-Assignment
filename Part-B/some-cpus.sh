#!/bin/bash
echo "" > output.txt

for n in {0..6..2}; do
  echo "Node :" $n >> output.txt
  echo "" >> output.txt
  for i in {0..47..6}; do
    echo "CPU :" $i " on node " $n >> output.txt
    numactl --membind $n --physcpubind $i\
    /u/csc469h/fall/pub/assgn1/bin/mccalpin-stream | tail -n 8\
    | head -n 6 >> output.txt
  done
  echo "" >> output.txt 
done
exit 0
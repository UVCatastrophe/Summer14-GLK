#!/bin/bash
set -o errexit
set -o nounset

JUNK=""
function junk {
  JUNK="$JUNK $@"
}
function cleanup {
  rm -rf $JUNK
}
trap cleanup err exit int term

SZ=100

echo "-1 1" |
unu pad -min 0 0 -max 7 M -b wrap |
unu reshape -s 2 2 2 |
unu resample -s $SZ $SZ $SZ -k tent -c node -o xx
junk xx

unu permute -i xx -p 1 0 2 -o yy; junk yy
unu permute -i xx -p 1 2 0 -o zz; junk zz

unu join -i xx yy zz -a 0 -incr |
unu project -a 0 -m l2 |
unu 2op - 1 - |
unu axinfo -a 0 1 2 -mm -1 1 |
unu dnorm -i - -o sphere.nrrd

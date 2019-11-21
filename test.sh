NUM=$1
VAR=$2
SEED=$3
OUT=/tmp
DIR=~/GitHub/mphf

$DIR/cnf/encode $NUM $VAR $SEED > $OUT/mphf-$NUM-$VAR-$SEED.cnf
#cadical $OUT/mphf-$NUM-$VAR-$SEED.cnf | \
~/CnC/cube-glucose.sh $OUT/mphf-$NUM-$VAR-$SEED.cnf -d 10 | \
   grep "^v "  | tr "\n" " " | sed 's|v||g' | sed 's|-[0-9]*|-|g' | sed 's|[1-9][0-9]* |+|g' | \
   sed 's| ||g' | sed 's|0||' | sed 's|-|0|g' | sed 's|+|1|g' | $DIR/decode/decode $NUM $VAR $SEED

rm $OUT/mphf-$NUM-$VAR-$SEED.cnf

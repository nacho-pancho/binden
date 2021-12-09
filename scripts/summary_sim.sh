#!/bin/bash
#
# mostly clean images, very similar among them
# image 0008: nice, straight, with stuff beyond simple text
# image 0019: nice, tilted, just text
#
indir="data/luisa/clean"
#
# selection of a few different examples
#
# image r0566_0698: nice, straight, clean, small letters, very uniform
# image r0566_0826: nice, straight, clean, small letters, a little more varied
#
#indir="data/luisa_sel"

for image in r0566_0008 # r0566_0698
do
  for pzero in 0.025 0.05 0.1
  do
    for pone  in 0.025 0.05 0.1
    do
      for template in n8 n8star n8star3
      do
        clean="${indir}/${image}.pbm"
        suffix="p${pzero}_q${pone}_${template}"
        outdir="results/${image}_${suffix}"
        noisy="${outdir}/${image}_${suffix}.pbm"
        echo "clean ${clean} noisy ${noisy}"
        if [[ ! -f ${clean} ]]
        then
          continue
        fi
	for method in median dude quorum binnlm
	do
          denoised=${outdir}/${image}_${suffix}_${method}.pbm
          dif=${outdir}/${image}_${suffix}_${method}_dif.pbm
          dt=${outdir}/${image}_${suffix}_${method}_time.txt
          err=${outdir}/${image}_${suffix}_${method}_err.txt
	  echo -n "${method} :"
	  grep diff ${err} | tr '\n' ' '
	  grep user ${dt}  | sed 's/user/time/g'
	done
     done # for each context
    done   # for each P(1->0)
  done     # for each P(0->1)
done       # for each image


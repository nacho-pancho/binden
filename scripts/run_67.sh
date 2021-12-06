#!/bin/bash
indir=data/luisa/noisy
image="r0108_0067"
input=${indir}/${image}.pgm
template="n8star"
stats="results/luisa.stats"
perr="0.1"
#
# vanilla DUDE
#
echo "VANILLA DUDE"
output=results/${image}_dude_${template}_p${perr}.pnm
build/apps/bin_dude --template=tpl/${template}.tpl --stats=${stats} --output=${output} --perr=0.1 ${input}
echo "QUORUM DUDE"
build/apps/quorum_den ${input}
mv quorum.pnm results/${image}_quorum_${template}_p${perr}.pnm
echo "VANILLA NLM"
build/apps/original_nlm ${input}
mv nlm.pnm results/${image}_nlm_${template}_p${perr}.pnm

echo "SEMI-BIN NLM"
mv seminlm.pnm results/${image}_seminlm_${template}_p${perr}.pnm

echo "BINARY NLM"
output=results/${image}_binnlm_${template}_p${perr}.pnm

build/apps/bin_nlm --output=${output}  ${input}


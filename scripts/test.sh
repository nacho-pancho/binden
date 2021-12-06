#!/bin/bash
indir=data/test
image="noisy3"
input=${indir}/${image}.pbm
#template="n8star"
template="n8"
stats="results/luisa.stats"
perr="0.1"
#
# vanilla DUDE
#
echo "MEDIAN"
output=results/${image}_median_${template}_p${perr}.pnm
build/apps/median --template=tpl/${template}.tpl --stats=${stats} --output=${output} ${input}

echo "VANILLA DUDE"
output=results/${image}_dude_${template}_p${perr}.pnm
build/apps/bin_dude --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}

echo "QUORUM DUDE"
output=results/${image}_quorum_${template}_p${perr}.pnm
build/apps/quorum_den --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}

echo "BINARY NLM"
output=results/${image}_binnlm_${template}_p${perr}.pnm
build/apps/bin_nlm --template=tpl/${template}.tpl --output=${output}  ${input}

echo "SEMI-BIN NLM"
output=results/${image}_semibin_${template}_p${perr}.pnm
build/apps/semibin_nlm --output=${output} --template=tpl/${template}.tpl  ${input}

echo "VANILLA NLM"
build/apps/original_nlm --template=tpl/${template}.tpl --output=results/${image}_nlm_${template}_p${perr}.pnm ${input}


#!/bin/bash
indir="data/luisa/clean"
image="r0566_0019"
perr="0.05"
template="n8star"

clean=${indir}/${image}.pgm
outdir=results/${image}_p${perr}_${template}
mkdir -p ${outdir}
input=${outdir}/${image}_p${perr}.pgm
#stats="results/luisa.stats"
#
# vanilla DUDE
#
echo -e "\n\n=========================================================================\n"
echo "ADD NOISE"
cp ${clean} results/
build/tools/add_noise --perr=${perr} --seed=42 --output=${input} ${clean}

echo -e "\n\n=========================================================================\n"
echo "MEDIAN"
output=${outdir}/${image}_p${perr}_median_${template}.pnm
dif=${outdir}/${image}_median_${template}_p${perr}_dif.pnm
time build/apps/median --template=tpl/${template}.tpl --stats=${stats} --output=${output} ${input}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "VANILLA DUDE"
output=${outdir}/${image}_p${perr}_dude_${template}.pnm
dif=${outdir}/${image}_dude_${template}_p${perr}_dif.pnm
time build/apps/bin_dude --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "QUORUM DUDE"
output=${outdir}/${image}_p${perr}_quorum_${template}.pnm
dif=${outdir}/${image}_quorum_${template}_p${perr}_dif.pnm
time build/apps/quorum_den --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "BINARY NLM"
output=${outdir}/${image}_p${perr}_binnlm_${template}.pnm
dif=${outdir}/${image}_binnlm_${template}_p${perr}_dif.pnm
time build/apps/bin_nlm --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "BINARY NLM (TREE)"
output=${outdir}/${image}_p${perr}_treenlm_${template}.pnm
dif=${outdir}/${image}_treenlm_${template}_p${perr}_dif.pnm
time build/apps/bin_nlm_tree --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}
echo -e "\n\n=========================================================================\n"

#echo "SEMI-BIN NLM"
#output=results/${image}_semibin_${template}_p${perr}.pnm
#build/apps/semibin_nlm --output=${output} --template=tpl/${template}.tpl  ${input}
#build/tools/compare --output=results/${image}_seminlm_${template}_p${perr}_dif.pnm ${clean} ${output}

#echo "VANILLA NLM"
#build/apps/original_nlm --template=tpl/${template}.tpl --output=results/${image}_nlm_${template}_p${perr}.pnm ${input}


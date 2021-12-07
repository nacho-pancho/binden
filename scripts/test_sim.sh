#!/bin/bash
indir="data/luisa/clean"
image="r0566_0019"
perr="0.05"
clean=${indir}/${image}.pgm
input=results/${image}_p${perr}.pgm
template="n8star"
#template="n8"
stats="results/luisa.stats"
#
# vanilla DUDE
#
echo -e "\n\n=======================================\n"
echo "ADD NOISE"
cp ${clean} results/
build/tools/add_noise --perr=${perr} --seed=42 --output=${input} ${clean}

echo -e "\n\n=======================================\n"
echo "MEDIAN"
output=results/${image}_median_${template}_p${perr}.pnm
time build/apps/median --template=tpl/${template}.tpl --stats=${stats} --output=${output} ${input}
build/tools/compare --output=results/${image}_median_${template}_p${perr}_dif.pnm ${clean} ${output}

echo -e "\n\n=======================================\n"
echo "VANILLA DUDE"
output=results/${image}_dude_${template}_p${perr}.pnm
time build/apps/bin_dude --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
build/tools/compare --output=results/${image}_dude_${template}_p${perr}_dif.pnm ${clean} ${output}

echo -e "\n\n=======================================\n"
echo "QUORUM DUDE"
output=results/${image}_quorum_${template}_p${perr}.pnm
time build/apps/quorum_den --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
build/tools/compare --output=results/${image}_quorum_${template}_p${perr}_dif.pnm ${clean} ${output}

echo -e "\n\n=======================================\n"
echo "BINARY NLM"
output=results/${image}_binnlm_${template}_p${perr}.pnm
time build/apps/bin_nlm --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
build/tools/compare --output=results/${image}_binnlm_${template}_p${perr}_dif.pnm ${clean} ${output}

echo -e "\n\n=======================================\n"
echo "BINARY NLM (TREE)"
output=results/${image}_treenlm_${template}_p${perr}.pnm
time build/apps/bin_nlm_tree --template=tpl/${template}.tpl --output=${output} --perr=0.1 ${input}
build/tools/compare --output=results/${image}_treenlm_${template}_p${perr}_dif.pnm ${clean} ${output}
echo -e "\n\n=======================================\n"

#echo "SEMI-BIN NLM"
#output=results/${image}_semibin_${template}_p${perr}.pnm
#build/apps/semibin_nlm --output=${output} --template=tpl/${template}.tpl  ${input}
#build/tools/compare --output=results/${image}_seminlm_${template}_p${perr}_dif.pnm ${clean} ${output}

#echo "VANILLA NLM"
#build/apps/original_nlm --template=tpl/${template}.tpl --output=results/${image}_nlm_${template}_p${perr}.pnm ${input}


#!/bin/bash
indir="data/luisa/clean"
image="r0566_0019"
template="n8star4"
perrsim=0.09
# there is already some noise present and this is disastrous for DUDE-like rules if not
# taken into account
perr=0.1
clean=${indir}/${image}.pbm
outdir=results/${image}_p${perr}_${template}
mkdir -p ${outdir}
noisy=${outdir}/${image}_p${perr}.pbm
#stats="results/luisa.stats"
#
# vanilla DUDE
#
echo -e "\n\n=========================================================================\n"
echo "ADD NOISE"
cp ${clean} ${outdir}/
build/tools/add_noise --perr=${perrsim} --seed=42 --output=${noisy} ${clean}
dif=${outdir}/${image}_p${perr}_dif.pnm
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${noisy}

echo -e "\n\n=========================================================================\n"
echo "MORPHOLOGICAL"
output=${outdir}/${image}_p${perr}_morph_${template}.pnm
dif=${outdir}/${image}_p${perr}_morph_${template}_dif.pnm
time code/morph.py --output=/tmp/morph.pgm --input=${noisy}
convert /tmp/morph.pgm -depth 1 ${output}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}


echo -e "\n\n=========================================================================\n"
echo "MEDIAN"
output=${outdir}/${image}_p${perr}_median_${template}.pnm
dif=${outdir}/${image}_p${perr}_median_${template}_dif.pnm
time build/apps/median --template=tpl/${template}.tpl --stats=${stats} --output=${output} ${noisy}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "VANILLA DUDE"
output=${outdir}/${image}_p${perr}_dude_${template}.pnm
dif=${outdir}/${image}_p${perr}_dude_${template}_dif.pnm
time build/apps/bin_dude --template=tpl/${template}.tpl --output=${output} --perr=${perr} ${opt} ${noisy}
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "VANILLA DUDE x 3"
output=${outdir}/${image}_p${perr}_dudex2_${template}.pnm
dif=${outdir}/${image}_p${perr}_dudex2_${template}_dif.pnm
time build/apps/bin_dude --template=tpl/${template}.tpl --output=${output} --perr=${perr} --iterations=3 ${noisy}
build/tools/compare --output=${dif} ${clean} ${output}


echo -e "\n\n=========================================================================\n"
echo "QUORUM DUDE"
output=${outdir}/${image}_p${perr}_quorum_${template}.pnm
dif=${outdir}/${image}_p${perr}_quorum_${template}_dif.pnm
time build/apps/quorum_den --template=tpl/${template}.tpl --output=${output} --perr=${perr} ${opt} ${noisy}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "QUORUM DUDE / BALL TEMPLATE"
output=${outdir}/${image}_p${perr}_quorum_ball2.pnm
dif=${outdir}/${image}_p${perr}_quorum_ball2_dif.pnm
time build/apps/quorum_den --template=tpl/ball2.tpl --output=${output} --perr=${perr} --iterations=2 ${noisy}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

echo -e "\n\n=========================================================================\n"
echo "BINARY NLM"
output=${outdir}/${image}_p${perr}_binnlm_${template}.pnm
dif=${outdir}/${image}_p${perr}_binnlm_${template}_dif.pnm
time build/apps/bin_nlm --template=tpl/${template}.tpl --output=${output} --search=11 --perr=${perr} ${noisy}
echo -e "\n-----------------------------------------\n"
build/tools/compare --output=${dif} ${clean} ${output}

#echo -e "\n\n=========================================================================\n"
#echo "BINARY NLM (TREE)"
#output=${outdir}/${image}_p${perr}_treenlm_${template}.pnm
#dif=${outdir}/${image}_treenlm_${template}_p${perr}_dif.pnm
#time build/apps/bin_nlm_tree --template=tpl/${template}.tpl --output=${output} --perr=${perr} ${noisy}
#echo -e "\n-----------------------------------------\n"
#build/tools/compare --output=${dif} ${clean} ${output}
#echo -e "\n\n=========================================================================\n"

#echo "SEMI-BIN NLM"
#output=results/${image}_semibin_${template}_p${perr}.pnm
#build/apps/semibin_nlm --output=${output} --template=tpl/${template}.tpl  ${noisy}
#build/tools/compare --output=results/${image}_seminlm_${template}_p${perr}_dif.pnm ${clean} ${output}

echo "VANILLA NLM"
#output=${outdir}/${image}_p${perr}_nlm_${template}.pnm
#dif=${outdir}/${image}_nlm_${template}_p${perr}_dif.pnm
#time build/apps/original_nlm --template=tpl/${template}.tpl --output=${output} --search=11 --perr=${perr} ${noisy}
#echo -e "\n-----------------------------------------\n"
#build/tools/compare --output=${dif} ${clean} ${output}

#echo "VANILLA NLM"
#build/apps/original_nlm --template=tpl/${template}.tpl --output=results/${image}_nlm_${template}_p${perr}.pnm ${noisy}


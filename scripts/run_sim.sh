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
indir="data/luisa_sel"

for image in r0566_0517 r0566_0121 r0566_0175 r0566_0826 # r0566_0698
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
        mkdir -p ${outdir}
        #
        # vanilla DUDE
        #
        echo -e "\n\n=========================================================================\n"
        echo "ADD NOISE"
        cp ${clean} results/
        build/tools/add_noise -0 ${pzero} -1 ${pone} --seed=42 --output=${noisy} ${clean}

        echo -e "\n\n=========================================================================\n"
        echo "MEDIAN"
       	method="median"
        denoised=${outdir}/${image}_${suffix}_${method}.pbm
        dif=${outdir}/${image}_${suffix}_${method}_dif.pbm
        dt=${outdir}/${image}_${suffix}_${method}_time.txt
	err=${outdir}/${image}_${suffix}_${method}_err.txt
        /usr/bin/time -p -o ${dt} build/apps/median --template=tpl/${template}.tpl --output=${denoised} ${noisy}
        echo -e "\n-----------------------------------------\n"
        build/tools/compare --output=${dif} ${clean} ${denoised} | tee ${err}

        echo -e "\n\n=========================================================================\n"
        echo "VANILLA DUDE"
       	method="dude"
        denoised=${outdir}/${image}_${suffix}_${method}.pbm
        dif=${outdir}/${image}_${suffix}_${method}_dif.pbm
        dt=${outdir}/${image}_${suffix}_${method}_time.txt
	err=${outdir}/${image}_${suffix}_${method}_err.txt
        /usr/bin/time -p -o ${dt} build/apps/bin_dude --template=tpl/${template}.tpl --output=${denoised} -0 ${pzero} -1 ${pone} ${noisy}
        build/tools/compare --output=${dif} ${clean} ${denoised} | tee ${err}

        echo -e "\n\n=========================================================================\n"
        echo "QUORUM DUDE"
	method="quorum"
        denoised=${outdir}/${image}_${suffix}_${method}.pbm
        dif=${outdir}/${image}_${suffix}_${method}_dif.pbm
        dt=${outdir}/${image}_${suffix}_${method}_time.txt
	err=${outdir}/${image}_${suffix}_${method}_err.txt
        /usr/bin/time -p -o ${dt} build/apps/quorum_den --template=tpl/${template}.tpl --output=${denoised} -0 ${pzero} -1 ${pone} ${noisy}
        echo -e "\n-----------------------------------------\n"
        build/tools/compare --output=${dif} ${clean} ${denoised} | tee ${err}

        echo -e "\n\n=========================================================================\n"
        echo "BINARY NLM"
	method="binnlm"
        denoised=${outdir}/${image}_${suffix}_${method}.pbm
        dif=${outdir}/${image}_${suffix}_${method}_dif.pbm
        dt=${outdir}/${image}_${suffix}_${method}_time.txt
	err=${outdir}/${image}_${suffix}_${method}_err.txt
        /usr/bin/time -p -o ${dt} build/apps/bin_nlm --template=tpl/${template}.tpl  --output=${denoised} -0 ${pzero} -1 ${pone} ${noisy}
        echo -e "\n-----------------------------------------\n"
        build/tools/compare --output=${dif} ${clean} ${denoised} | tee ${err}

        #echo -e "\n\n=========================================================================\n"
        #echo "BINARY NLM (TREE)"
        #output=${outdir}/${image}_p${perr}_treenlm_${template}.pnm
        #dif=${outdir}/${image}_p${perr}_treenlm_${template}_dif.pnm
        #time build/apps/bin_nlm_tree --template=tpl/${template}.tpl --output=${output} --perr=${perr} ${input}
        ##echo -e "\n-----------------------------------------\n"
        #build/tools/compare --output=${dif} ${clean} ${output}
        echo -e "\n\n=========================================================================\n"
      done # for each context
    done   # for each P(1->0)
  done     # for each P(0->1)
done       # for each image

#echo "SEMI-BIN NLM"
#output=results/${image}_semibin_${template}_p${perr}.pnm
#build/apps/semibin_nlm --output=${output} --template=tpl/${template}.tpl  ${input}
#build/tools/compare --output=results/${image}_seminlm_${template}_p${perr}_dif.pnm ${clean} ${output}

#echo "VANILLA NLM"
#build/apps/original_nlm --template=tpl/${template}.tpl --output=results/${image}_nlm_${template}_p${perr}.pnm ${input}


#!/bin/bash
./dude_bsc -p0.02 -ttemplate_einstein.asc bscdata/einstein_bin_pe02.pbm e1.pbm
./dude_bsc -p0.02 -ttemplate_multi_1.asc bscdata/einstein_bin_pe02.pbm e2.pbm
./dude_bsc -p0.02 -ttemplate_einstein.asc bscdata/einstein_bin_pe02.pbm e3.pbm e1.pbm
#./dude_bsc -p0.02 -t3333 -k20 -r8 -n10 -d3 einstein_bin_pe02.pgm e4.pgm 
#./dude_bsc -p0.02 -t3333 -k20 -r8 -n100 -d3 einstein_bin_pe02.pgm e5.pgm 
#./dude_bsc -p0.02 -t3333 -k20 -r8 -n1000 -d3 einstein_bin_pe02.pgm e6.pgm 
compare -metric ae bscdata/einstein_bin.pbm e1.pbm d1.ppm
compare -metric ae bscdata/einstein_bin.pbm e2.pbm d2.ppm
compare -metric ae bscdata/einstein_bin.pbm e3.pbm d3.ppm
#compare -metric ae einstein_bin.pgm e4.pgm d4.ppm

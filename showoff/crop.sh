#!/bin/bash
#img="r0566_0008"
geom="169x245+1250+798"
convert -crop ${geom} ${1}.pbm ${1}_cropped.pbm

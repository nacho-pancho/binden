#!/bin/bash
wget -c https://www.ipol.im/pub/art/2012/l-bm3d/bm3d_src.tar.gz
wget -c https://www.ipol.im/pub/art/2013/16/nl-bayes_20130617.tar.gz
for i in *.gz
do
	tar xzf $i
done

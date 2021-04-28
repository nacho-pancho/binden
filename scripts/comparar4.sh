#!/bin/bash
convert -crop 300x1000+0+0 $1 a.pgm
convert -crop 300x1000+0+0 $2 b.pgm
convert -crop 300x1000+0+0 $3 c.pgm
convert -crop 300x1000+0+0 $4 d.pgm
convert +append a.pgm b.pgm c.pgm d.pgm comp.pgm 


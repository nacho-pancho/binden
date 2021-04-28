#!/bin/bash
convert -crop 600x1000+0+0 $1 a.pgm
convert -crop 600x1000+0+0 $2 b.pgm
convert -crop 600x1000+0+0 $3 c.pgm
convert +append a.pgm b.pgm c.pgm comp.pgm


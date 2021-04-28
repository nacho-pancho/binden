#!/bin/bash
cp data/camera.pgm .
cp data/peppers.ppm .
build/tests/test_pnm camera.pgm 
build/tests/test_pnm peppers.ppm 
build/tests/test_image camera.pgm 

#!/usr/bin/env python3

import sys
import time
import argparse

import numpy as np
import skimage.morphology as morph  # not really necessary; only to build a binary disk mask
import skimage.io as imgio

from PIL import Image, ImageMorph, ImageOps


def morph_denoise(i,args):
    #i2 = morph.remove_small_holes(i,20,in_place=False)
    #i2 = morph.remove_small_objects(i2,8,in_place=False)
    i2 = morph.dilation(i,morph.disk(args["dilate_radius"]))
    i2 = morph.erosion(i2,morph.disk(args["erode_radius"]))
    return i2

if __name__ == '__main__':
    #
    # command line arguments
    #
    ap = argparse.ArgumentParser()
    ap.add_argument("--dilate-radius", type=int, default=2,
                    help="radius of ball used for dilation")
    ap.add_argument("--erode-radius", type=int, default=2,
                    help="radius of ball used for erosion")
    ap.add_argument("--input", type=str, required=True,
                    help="path to input image")
    ap.add_argument("--output", type=str, required=True,
		    help="path to output image")
    #
    # INICIALIZACION
    #
    args = vars(ap.parse_args())
    input_file  = args["input"]
    output_file = args["output"]
    i = imgio.imread(input_file)
    o = morph_denoise(i, args)
    o = ((o>=0.5)*255).astype(np.uint8)
    imgio.imsave(output_file, o)

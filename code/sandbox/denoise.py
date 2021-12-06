#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Sep 12 17:22:41 FIRST_CROP18

@author: nacho
"""
import sys
import os
import time
import argparse

import numpy as np
from PIL import Image, ImageMorph, ImageOps
import PIL
from skimage import morphology as morph

from luisa import denoising as pre
from luisa import image
#
# ---------------------------------------------------------------------------------------
#
if __name__ == '__main__':
    #
    # command line arguments
    #
    ap = argparse.ArgumentParser()
    ap.add_argument("--indir", type=str, default="/luisa/alineadas",
                    help="path indir  where to find prealigned files")
    ap.add_argument("--outdir", type=str, default="preproc",
		    help="if specified, save data to this output directory as images and text files")
    ap.add_argument("--out-file", type=str, default="",
                    help="if specified, save data to HDF5 with this file name.")
    ap.add_argument("--list", type=str, default="",
                    help="text file where input pre-aligned files are specified")
    ap.add_argument("--more-debug", action="store_true",
                    help="Save a LOT of debugging and diagnostic info (much slower!).")
    ap.add_argument("--debug-dir", type=str, default="",
                    help="Save debugging and diagnostic info to specified directory.")
    ap.add_argument("--force", action="store_true",
                    help="Forces the output to be overwritten even if it exists.")
    ap.add_argument("--margin", type=int, default=0,
                    help="Cut this number of pixels from each side of image before analysis.")
    ap.add_argument("--recreate", type=bool, default=True,
                    help="Re-create source images tree.")
    #
    # INICIALIZACION
    #
    args = vars(ap.parse_args())
    indir    = args["indir"]
    list_file = args["list"]
    output    = args["out_file"]
    outdir    = args["outdir"]
    debugdir  = args["debug_dir"]
    recreate  = args["recreate"]
    force     = args["force"]

    if not len(list_file):
        print('Debe especificar una lista de archivos.')
        exit(1)

    if not os.path.exists(list_file):
        print(f'Lista {list_file} no encontrada.')
        exit(1)

    if not os.path.exists(indir):
        print(f'Directorio de origen {indir} no encontrado.')
        exit(1)
    #
    #
    #

    #
    # procesar archivos
    #
    with open(list_file) as fl:
        #
        # inicializacion de memoria
        #
        nimage = 0
        t0 = time.time()
        nerr = 0
        for relfname in fl:
            #
            # proxima imagen
            #
            nimage = nimage + 1
            #
            # ENTRADA Y SALIDA
            #
            # nombres de archivos de entrada y salida
            #
            relfname = relfname.rstrip('\n')
            reldir, fname = os.path.split(relfname)
            fbase, fext = os.path.splitext(fname)
            input_fname = os.path.join(indir, relfname)
            foutdir = os.path.join(outdir,reldir)

            if not os.path.exists(foutdir):
                os.makedirs(foutdir,exist_ok=True)

            output_fname = os.path.join(foutdir,fname)
            print(f'#{nimage} input={input_fname} output={output_fname}')
            if os.path.exists(output_fname) and not force:
                continue
            
            if not os.path.exists(input_fname):
                print(f'ERROR: image not found.')
                continue
            try:
                I = image.load(input_fname)
                i  = (1-np.array(I)).astype(bool)
     
                t0 = time.time()
                # parametros anteriores (buenos)
                #i2, _ = denoising.counting_filter(i, radius=4, p=perr, interleave=2, decay=False)
                # parametros con decay
                #i2, _ = denoising.counting_filter(i, radius=4, p=0.025, interleave=2, decay=True)
                # sin interleave
                i2, _ = pre.counting_filter(i, radius=8, p=0.025, interleave=1, decay=True)

                t0 = time.time()
                i3 = pre.remove_large_areas(i2,7000)
                I3 = Image.fromarray((1-i3).astype(bool))

                image.save(output_fname, I3)
            except PIL.UnidentifiedImageError as e:
                print(f'ERROR: invalid image.')
                continue
            
        #
        # fin main
        #
# ---------------------------------------------------------------------------------------

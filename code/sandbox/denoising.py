#!/usr/bin/env python3

import sys
import time

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import skimage.morphology as morph  # not really necessary; only to build a binary disk mask
from scipy.stats import binom
import scipy.signal as dsp
from PIL import Image, ImageMorph, ImageOps

import den

#
# ---------------------------------------------------------------------------------------
#
def pad(Y, radius):
    h, w = Y.shape
    hx, wx = h + 2 * radius, w + 2 * radius
    X = np.empty((hx, wx))
    X[radius:-radius, radius:-radius] = Y
    X[-radius:, radius:-radius] = np.flip(Y[-radius:, :], axis=0)
    X[:radius, radius:-radius] = np.flip(Y[: radius, :], axis=0)
    X[radius:-radius, -radius:] = np.flip(Y[:, -radius:], axis=1)
    X[radius:-radius, :radius] = np.flip(Y[:, :radius], axis=1)
    # pending: corners
    return X
#
# ---------------------------------------------------------------------------------------
#
def counting_filter_pass_1(Y, radius, Yctx=None, decay=False):
    '''
    El cálculo de contextos se puede hacer mucho mas rapido como 
    una convolucion con un kernel de 1's
    Los counts tambiénse pueden acelerar con un comprehension
    '''
    t0 = time.time()
    if Yctx is None:
        Yctx = Y
        aux = False
    else:
        aux = True
    h, w = Y.shape
    hw = h * w
    t = np.concatenate( (np.arange(-radius, 0), np.arange(0, radius + 1)) )
    kw = len(t)
    o = np.ones(kw)
    tv = np.outer(t**2,o)
    kernel = np.sqrt(tv + tv.T)
    if not decay:
        kernel = 1*(kernel <= radius)
    else:
        kernel[radius,radius] = 1
        kernel = 1 / kernel

    kernel[radius,radius] = 0
    n = int(np.ceil(np.sum(kernel)))+1
    kernel[radius, radius] = 0  # do not count center!
    context_map = np.floor(dsp.convolve(Yctx, kernel, mode="same")).astype(np.uint16)
    #print('convolution',time.time()-t0)
    t0 = time.time()
    #
    # count where S=s
    center_counts = np.zeros(n,dtype=np.uint32)
    context_counts = np.zeros(n,dtype=np.uint32)
    # written in C now
    # context_map_1 = np.copy(context_map)
    # context_map_1[Y == False] = -1  # discard places where Y = 0
    
    den.countctx(context_map,Y.astype(np.uint8),context_counts,center_counts);

    #for i in range(n):
    #    context_counts[i] = len(np.flatnonzero(context_map == i))
    #    center_counts[i] = len(np.flatnonzero(context_map_1 == i))
    #print('counts',time.time()-t0)
    #t0 = time.time()
    #if aux:
    #    plt.imsave('context_map_2.png', context_map.astype(np.uint8))
    #else:
    #    plt.imsave('context_map.png', context_map.astype(np.uint8))
    #print('save aux',time.time()-t0)
    return context_map, context_counts, center_counts

#
# ---------------------------------------------------------------------------------------

def counting_filter_pass_2(Y, context_map, context_counts, center_counts, p=0.1):
    # create pseudo image with counts in place of contexts
    t0 = time.time()
    counts = (center_counts + 0.5) / (context_counts + 1)  # Kritchevskii-Trofimov smoothing
    counts_map = np.array([counts[c] for c in context_map]).reshape(context_map.shape)
    #print('normalization',time.time()-t0)
    t0 = time.time()
    X = np.copy(Y)
    #plt.imsave('counts_map.png', (255 * counts_map).astype(np.uint8))
    X[(Y == True) & (counts_map < p)] = False
    X[(Y == False) & (counts_map > (1 - p))] = True
    #print('assignment',time.time()-t0)
    return X

#
# ---------------------------------------------------------------------------------------
#
def estimate_noise(Y, radius, interleave=1):
    h, w = Y.shape
    hw = h * w
    kernel = morph.square(radius)
    n = np.sum(kernel)
    S = dsp.convolve(Y.astype(np.int), kernel, mode="same")
    return np.percentile(S,1)/n,(1-np.percentile(S,99)/n)
#
# ---------------------------------------------------------------------------------------
#
def counting_filter(Y, radius, p, interleave=1, decay=False):
    if interleave > 1:
        # print('first pass (fast)')
        S = list()
        N = list()
        C = list()
        for i in range(interleave):
            for j in range(interleave):
                Sij, Nij, Cij = counting_filter_pass_1(Y[i::interleave, j::interleave], radius, Yctx=None, decay=decay)
                S.append(Sij)
                N.append(Nij)
                C.append(Cij)
        # print('second pass (fast)')
        X = np.empty(Y.shape, dtype=np.uint8)
        k = 0
        for i in range(interleave):
            for j in range(interleave):
                X[i::interleave, j::interleave] = counting_filter_pass_2(Y[i::interleave, j::interleave], S[k],
                                                                              N[k], C[k], p)
                k += 1
        # print('first pass (with denoised contexts)')
        # create contexts from DENOISED image
        S = list()
        N = list()
        C = list()
        for i in range(interleave):
            for j in range(interleave):
                Sij, Nij, Cij = counting_filter_pass_1(Y[i::interleave, j::interleave], radius,
                                                            Yctx=X[i::interleave, j::interleave], decay=decay)
                S.append(Sij)
                N.append(Nij)
                C.append(Cij)
        # print('second pass (with denoised contexts)')
        X0 = X
        X = np.empty(Y.shape, dtype=np.uint8)
        k = 0
        for i in range(interleave):
            for j in range(interleave):
                X[i::interleave, j::interleave] = counting_filter_pass_2(Y[i::interleave, j::interleave], S[k],
                                                                              N[k], C[k], p)
                k += 1

        return X, X0
    else:
        # print('first pass (fast)')
        S, Nc, Sc = counting_filter_pass_1(Y, radius)
        # print('second pass (fast)')
        X0 = counting_filter_pass_2(Y, S, Nc, Sc, p)
        # print('second pass (with denoised contexts)')
        S, Nc, Sc = counting_filter_pass_1(Y, radius, Yctx=X0)
        # print('second pass (with denoised contexts)')
        X = counting_filter_pass_2(Y, S, Nc, Sc, p)
        return X, X0
#
# ---------------------------------------------------------------------------------------
#
def simple_denoise_inner(i,radius):
    width = 2*radius+1
    size = width**2
    thres = radius
    kernel = np.ones((width,width))
    i2 = dsp.convolve2d(i,kernel,mode="same",boundary="symm")
    i3 = np.copy(i)
    i3[i2 < thres] = 0
    i3[i2 > (size-thres)] = 1
    return i3
#
# ---------------------------------------------------------------------------------------
#
def simple_denoise(i):
    '''
    input is assumed to be binary, with 0 corresponding to the background
    value a d 1 to the foreground (inverse interpretation compared to 8-bit grayscale!)
    '''
    i2 = simple_denoise_inner(i,1)
    i2 = simple_denoise_inner(i2,2)
    i2 = simple_denoise_inner(i2,3)
    return i2
#
# ---------------------------------------------------------------------------------------
#
def morph_denoise(i):
    i2 = morph.remove_small_holes(i,20,in_place=False)
    i2 = morph.remove_small_objects(i2,8,in_place=False)
    i2 = morph.dilation(i2,morph.disk(4))
    #
    # horizontal profile
    #
    hp = np.mean(i2[100:-100,:],axis=0)
    #
    # threshold: 20% of average profile value at center of doc
    #
    n = len(hp)
    av = np.mean(hp[n//3:(n*2)//3])
    thres = 0.1*av
    n2 = n//2
    # last index such that profile is below threshold
    nzi = np.flatnonzero(hp[:n2] < thres)
    if len(nzi):
        leftmargin = nzi[-1]
    else:
        leftmargin = 0
    # first index
    nzi = np.flatnonzero(hp[n2:] < thres)
    if len(nzi):
        rightmargin = nzi[0] + n2
    else:
        rightmargin = n
    print(leftmargin,rightmargin)
    i2 = i2.astype(np.float)
    i2[:,:leftmargin] /= 4
    i2[:,rightmargin:] /= 4
    return i2
#
# ---------------------------------------------------------------------------------------
#
def binary_nlm(Y,radius,search,p):
    h,w = Y.shape
    X = np.zeros(Y.shape)
    n = (2*radius+1)**2
    for i in range(radius,h-radius):
        for j in range(radius,w-radius):
            print(i,j)
            i0 = i-radius
            j0 = j-radius
            i1 = i+radius+1
            j1 = j+radius+1
            Yij = Y[i0:i1,j0:j1]
            imin = max(radius,i-search)
            imax = min(h-radius,i+search)
            jmin = max(radius,j-search)
            jmax = min(w-radius,j+search)
            S = 0
            Xij = 0
            for ii in range(imin,imax):
                for jj in range(jmin,jmax):
                    ii0 = ii - radius
                    jj0 = jj - radius
                    ii1 = ii + radius + 1
                    jj1 = jj + radius + 1
                    Yiijj = Y[ii0:ii1,jj0:jj1]
                    k = np.sum(Yij ^ Yiijj)
                    #
                    # weight is given by binomial tail: probability of observing
                    # k or more ones in a Binomial distribution with parameters n,p
                    W = binom.sf(k,n,p)
                    S += W
                    Xij += W*Y[ii,jj]
            X[i,j] = (2*Xij >= S)


#
# ---------------------------------------------------------------------------------------
#
def remove_large_areas(i,maxsize):
    '''
    Esto anda muy bien
    '''
    labels = morph.label(i,connectivity=2)
    labels = labels.ravel()
    counts = np.bincount(labels)
    #
    # replace labels by the size of the areas
    #
    size_map = np.array([counts[j] for j in labels],dtype=int).reshape(i.shape)
    i2 = np.copy(i)
    i2[size_map > maxsize] = 0
    return i2

#
# ---------------------------------------------------------------------------------------
#
def convexify(I):
    I = I.convert('L')
    I2 = ImageOps.invert(I)
    # mild convexity
    convex_pat = [
            '4:(.1. 1.. .1.)->1',
    ]
    lut = ImageMorph.LutBuilder(patterns=convex_pat).build_lut()
    mop = ImageMorph.MorphOp(lut)
    changed,I2 = mop.apply(I2)
    while changed > 000:
        print('changed',changed)
        changed,I2 = mop.apply(I2)

    return ImageOps.invert(I2)
#
# ---------------------------------------------------------------------------------------
#
def denoise_counting(I):
    i = np.array(I)
    #i2,_ = counting_filter.counting_filter(i,radius=4,p=0.1,interleave=2)
    i2,_ = denoising.counting_filter(i,radius=4,p=0.05,interleave=2)
    return Image.fromarray(i2.astype(bool))



#
# ---------------------------------------------------------------------------------------
#
if __name__ == "__main__":
    Y = plt.load(sys.argv[1]).astype(np.bool)
    X = np.copy(Y)
    if len(sys.argv) > 2:
        radius = int(sys.argv[2])
    else:
        radius = 3
    if len(sys.argv) > 3:
        p = float(sys.argv[3])
    else:
        p = 0.1
    if len(sys.argv) > 4:
        intl = int(sys.argv[4])
    else:
        intl = 1  # no interleaving
    print('radius', radius, 'p', p, 'interleave', intl)
    X, X0 = counting_filter(Y, radius, p, interleave=intl)
    plt.imsave("counting_filter_iter0.png", X0, cmap=cm.get_cmap('gray'))
    plt.imsave("counting_filter_iter1.png", X, cmap=cm.get_cmap('gray'))
#

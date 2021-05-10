/**
 * different binary denoising rules 
 */
#ifndef DENOISER_H
#define DENOISER_H

/**
 * prototype for denoiser rule
 * \param z observed noisy sample
 * \param k frequency of ones in the (sub)sequence
 * \param m size of the (sub)sequence
 * \param d crossover probability 
 */
typedef int (*denoiser_f)(int z, int k, int m, double d);

/**
 * dumbest rule: ignores z and d; returns 1 if 2k > m and 0 otherwise
 */
int majority(int z, int k, int m, double d);

/**
 * Bayes risk minimization (using Hamming loss)
 */
int bayes(int z, int k, int m, double d);

/**
 * Discrete Universal Denoiser, BSC channel
 */
int dude(int z, int k, int m, double d);



#endif
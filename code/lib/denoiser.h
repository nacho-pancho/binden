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
typedef double ( * denoiser_f )( double z, double k, double m, double d );

/**
 * dumbest rule: ignores z and d; returns 1 if 2k > m and 0 otherwise
 */
double majority ( double z, double k, double m, double d );

/**
 * Bayes risk minimization (using Hamming loss)
 */
double bayes ( double z, double k, double m, double d );

/**
 * Discrete Universal Denoiser, BSC channel
 */
double dude ( double z, double k, double m, double d );



#endif
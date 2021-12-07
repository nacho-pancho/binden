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
 * \param p01 P(0->1)
 * \param p10 P(1->0)
 */
typedef double ( * denoiser_f )( double z, double k, double m, double p01, double p10 );

/**
 * dumbest rule: ignores z and d; returns 1 if 2k > m and 0 otherwise
 */
double majority ( double z, double k, double m, double p01, double p10 );

/**
 * Bayes risk minimization (using Hamming loss)
 */
double bayes ( double z, double k, double m, double p01, double p10 );

/**
 * Discrete Universal Denoiser, BSC channel
 */
double dude ( double z, double k, double m, double p01, double p10 );



#endif
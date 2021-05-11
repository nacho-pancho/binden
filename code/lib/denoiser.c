/**
 * dumbest rule: ignores z and d; returns 1 if 2k > m and 0 otherwise
 */
double majority ( double z, double k, double m, double d ) {
    return 2.0*k > m ? 1.0 : 0.0;
}

/**
 * Bayes risk minimization (using Hamming loss)
 */
double bayes ( double z, double k, double m, double d ) {
    double p = z > 0 ? k / m : 1.0 -  k / m;
    return ( p >= d ) ? z : 1.0-z;
}

/**
 * Discrete Universal Denoiser, BSC channel
 */
double dude ( double z, double k, double m, double d ) {
    double p = z > 0 ? k / m : 1.0 - k / m;
    double t = 2.0 * d * ( 1.0 - d );
    return ( p >= t ) ? z : 1.0 - z;
}


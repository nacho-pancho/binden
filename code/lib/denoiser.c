/**
 * dumbest rule: ignores z and d; returns 1 if 2k > m and 0 otherwise
 */
double majority ( double z, double k, double m, double p01, double p10 ) {
    return 2.0*k > m ? 1.0 : 0.0;
}

/**
 * Bayes risk minimization (using Hamming loss)
 */
double bayes ( double z, double k, double m, double p01, double p10 ) {
    // TODO: fix for non-symmetric
    const double e = p01 + p10;
    double p = z > 0 ? k / m : 1.0 -  k / m;
    return ( p >= e ) ? z : 1.0-z;
}

/**
 * Discrete Universal Denoiser, BSC channel
 */
double dude ( double z, double k, double m, double p01, double p10 ) {
    const double e = p01 + p10;
    double p = z > 0 ? k / m : 1.0 - k / m;
    double t = 2.0 * e * ( 1.0 - e );
    return ( p >= t ) ? z : 1.0 - z;
}


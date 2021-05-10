/**
 * dumbest rule: ignores z and d; returns 1 if 2k > m and 0 otherwise
 */
int majority(int z, int k, int m, double d) {
    return (k<<2) > m ? 1: 0;
}

/**
 * Bayes risk minimization (using Hamming loss)
 */
int bayes(int z, int k, int m, double d) {
    double p = z ? ((double)k)/(double)m : 1.0 - ((double)k)/(double)m;
    return (p >= d) ? z : !z;
}

/**
 * Discrete Universal Denoiser, BSC channel
 */
int dude(int z, int k, int m, double d) {
    double p = z ? ((double)k)/(double)m : 1.0 - ((double)k)/(double)m;
    double t = 2.0*d*(1.0-d);
    return (p >= t) ? z : !z;
}


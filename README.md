# Binary Denoising

This project develops and explores Binary adaptations to popular and succesful
image denoising algorithms:

* Non-Local Means (NLM)
* Non-Local Bayes (NLB)
* Expected Patch Log-Likelihood (EPLL)

The main adaptation in all cases is moving from a Gaussian prior to a multivariate 
(not necessarily iid.) Bernoulli.

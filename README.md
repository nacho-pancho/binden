# Binary Denoising

This project implements and evaluates fast denoising methods for binary images capable of processing millions of images of very large size (20-25MPix). The methods are:

* A method partially based on the Non-Local Means (B-NLM) method, with some unique strategies for binary neighborhoods, and a custom decision (Bernoulli) prior
* The Discrete Universal DEnoiser (DUDE), with a specific choice of contexts and other tweaks
* A simplified version of the DUDE, with a much simpler and faster context model which also makes it more resilient to violations in the core DUDE hypotheses. This method is comparable in running time to a plain median filter while attaining the performance of DUDE (second best), and surpassing it (qualitatively) on real noisy images.


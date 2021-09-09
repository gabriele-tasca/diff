# diff

Diffusion equation method for drawing freeform gradients.

Algorithm adapted from: [A GPU Laplacian Solver for Diffusion Curves and Poisson Image Editing](http://peterwonka.net/Publications/pdfs/2009.SGA.Jeschke.LaplacianSolver.final.pdf)

Coming soon: GPU implementation.

Example:

1) Draw a cloud:

![Outline](https://github.com/gabriele-tasca/diff/blob/master/examples/cloud/example1.png)


2) Use some flat-colored lines to define the color at the edges:

![Outline](https://github.com/gabriele-tasca/diff/blob/master/examples/cloud/example2.png)


3) The algorithm fills in the body of the cloud:

![Outline](https://github.com/gabriele-tasca/diff/blob/master/examples/cloud/second-cloud.png)

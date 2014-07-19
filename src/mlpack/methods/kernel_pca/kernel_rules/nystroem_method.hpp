/**
 * @file nystroem_method.hpp
 * @author Marcus Edel
 *
 * Use the Nystroem method for approximating a kernel matrix.
 */

#ifndef __MLPACK_METHODS_KERNEL_PCA_NYSTROEM_METHOD_HPP
#define __MLPACK_METHODS_KERNEL_PCA_NYSTROEM_METHOD_HPP

#include <mlpack/core.hpp>
#include <mlpack/methods/nystroem_method/kmeans_selection.hpp>
#include <mlpack/methods/nystroem_method/nystroem_method.hpp>

namespace mlpack {
namespace kpca {

template<
  typename KernelType,
  typename PointSelectionPolicy = kernel::KMeansSelection<> 
>
class NystroemKernelRule
{
  public:
    /**
     * Construct the kernel matrix approximation using the nystroem method.
     *
     * @param data Input data points.
     * @param transformedData Matrix to output results into.
     * @param eigval KPCA eigenvalues will be written to this vector.
     * @param eigvec KPCA eigenvectors will be written to this matrix.
     * @param rank Rank to be used for matrix approximation.
     * @param kernel Kernel to be used for computation.
     */
    static void ApplyKernelMatrix(const arma::mat& data,
                                  arma::mat& transformedData,
                                  arma::vec& eigval,
                                  arma::mat& eigvec,
                                  const size_t rank,
                                  KernelType kernel = KernelType())
    {
      arma::mat G, v;
      kernel::NystroemMethod<KernelType, PointSelectionPolicy> nm(data, kernel,
                                                        rank);
      nm.Apply(G);
      transformedData = G.t() * G;

      // For PCA the data has to be centered, even if the data is centered. But
      // it is not guaranteed that the data, when mapped to the kernel space, is
      // also centered. Since we actually never work in the feature space we 
      // cannot center the data. So, we perform a "psuedo-centering" using the
      // kernel matrix.
      arma::rowvec rowMean = arma::sum(transformedData, 0) / 
          transformedData.n_cols;
      transformedData.each_col() -= arma::sum(transformedData, 1) /
          transformedData.n_cols;
      transformedData.each_row() -= rowMean;
      transformedData += arma::sum(rowMean) / transformedData.n_cols;

      // Eigendecompose the centered kernel matrix.
      arma::svd(eigvec, eigval, v, transformedData);
      eigval %= eigval / (data.n_cols - 1);

      transformedData = eigvec.t() * G.t();
    }
};

}; // namespace kpca
}; // namespace mlpack

#endif

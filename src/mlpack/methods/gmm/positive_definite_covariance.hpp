/**
 * @file positive_definite_covariance.hpp
 * @author Ryan Curtin
 *
 * Restricts a covariance matrix to being positive definite.
 */
#ifndef __MLPACK_METHODS_GMM_POSITIVE_DEFINITE_COVARIANCE_HPP
#define __MLPACK_METHODS_GMM_POSITIVE_DEFINITE_COVARIANCE_HPP

namespace mlpack {
namespace gmm {

/**
 * Given a covariance matrix, force the matrix to be positive definite.
 */
class PositiveDefiniteCovariance
{
 public:
  /**
   * Apply the positive definiteness constraint to the given covariance matrix.
   *
   * @param covariance Covariance matrix.
   */
  static void ApplyConstraint(arma::mat& covariance)
  {
    // TODO: make this more efficient.
    if (det(covariance) <= 1e-50)
    {
      Log::Debug << "Covariance matrix is not positive definite.  Adding "
          << "perturbation." << std::endl;

      double perturbation = 1e-30;
      while (det(covariance) <= 1e-50)
      {
        covariance.diag() += perturbation;
        perturbation *= 10;
      }
    }
  }
};

}; // namespace gmm
}; // namespace mlpack

#endif

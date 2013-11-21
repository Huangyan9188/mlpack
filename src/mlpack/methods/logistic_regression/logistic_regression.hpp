/**
 * @file logistic_regression.hpp
 * @author Sumedh Ghaisas
 *
 * The LogisticRegression class, which implements logistic regression.  This
 * implements supports L2-regularization.
 */
#ifndef __MLPACK_METHODS_LOGISTIC_REGRESSION_LOGISTIC_REGRESSION_HPP
#define __MLPACK_METHODS_LOGISTIC_REGRESSION_LOGISTIC_REGRESSION_HPP

#include <mlpack/core.hpp>
#include <mlpack/core/optimizers/lbfgs/lbfgs.hpp>

#include "logistic_regression_function.hpp"

namespace mlpack {
namespace regression {

template<
  template<typename> class OptimizerType = mlpack::optimization::L_BFGS
>
class LogisticRegression
{
 public:
  /**
   * Construct the LogisticRegression class with the given labeled training
   * data.  This will train the model.  Optionally, specify lambda, which is the
   * penalty parameter for L2-regularization.  If not specified, it is set to 0,
   * which results in standard (unregularized) logistic regression.
   *
   * @param predictors Input training variables.
   * @param responses Outputs resulting from input training variables.
   * @param lambda L2-regularization parameter.
   */
  LogisticRegression(const arma::mat& predictors,
                     const arma::vec& responses,
                     const double lambda = 0);

  /**
   * Construct the LogisticRegression class with the given labeled training
   * data.  This will train the model.  Optionally, specify lambda, which is the
   * penalty parameter for L2-regularization.  If not specified, it is set to 0,
   * which results in standard (unregularized) logistic regression.
   *
   * @param predictors Input training variables.
   * @param responses Outputs results from input training variables.
   * @param initialPoint Initial model to train with.
   * @param lambda L2-regularization parameter.
   */
  LogisticRegression(const arma::mat& predictors,
                     const arma::vec& responses,
                     const arma::mat& initialPoint,
                     const double lambda = 0);

  //! Return the parameters (the b vector).
  const arma::vec& Parameters() const { return parameters; }
  //! Modify the parameters (the b vector).
  arma::vec& Parameters() { return parameters; }

  //! Return the lambda value for L2-regularization.
  const double& Lambda() const { return errorFunction.Lambda(); }
  //! Modify the lambda value for L2-regularization.
  double& Lambda() { return errorFunction().Lambda(); }

  /**
   * Predict the responses to a given set of predictors.  The responses will be
   * either 0 or 1.  Optionally, specify the decision boundary; logistic
   * regression returns a value between 0 and 1.  If the value is greater than
   * the decision boundary, the response is taken to be 1; otherwise, it is 0.
   * By default the decision boundary is 0.5.
   *
   * @param predictors Input predictors.
   * @param responses Vector to put output predictions of responses into.
   * @param decisionBoundary Decision boundary (default 0.5).
   */
  void Predict(const arma::mat& predictors,
               arma::vec& responses,
               const double decisionBoundary = 0.5) const;

  /**
   * Compute the accuracy of the model on the given predictors and responses,
   * optionally using the given decision boundary.  The responses should be
   * either 0 or 1.  Logistic regression returns a value between 0 and 1.  If
   * the value is greater than the decision boundary, the response is taken to
   * be 1; otherwise, it is 0.  By default, the decision boundary is 0.5.
   *
   * The accuracy is returned as a percentage, between 0 and 100.
   *
   * @param predictors Input predictors.
   * @param responses Vector of responses.
   * @param decisionBoundary Decision boundary (default 0.5).
   * @return Percentage of responses that are predicted correctly.
   */
  double ComputeAccuracy(const arma::mat& predictors,
                         const arma::vec& responses,
                         const double decisionBoundary = 0.5) const;

  /**
   * Compute the error of the model.  This returns the negative objective
   * function of the logistic regression log-likelihood function.  For the model
   * to be optimal, the negative log-likelihood function should be minimized.
   *
   * @param predictors Input predictors.
   * @param responses Vector of responses.
   */
  double ComputeError(const arma::mat& predictors,
                      const arma::vec& responses) const;

 private:
  //! Matrix of predictor points (X).
  const arma::mat& predictors;
  //! Vector of responses (y).
  const arma::vec& responses;
  //! Vector of trained parameters.
  arma::vec parameters;

  //! Instantiated error function that will be optimized.
  LogisticRegressionFunction errorFunction;
  //! Instantiated optimizer.
  OptimizerType<LogisticRegressionFunction> optimizer;

  /**
   * Learn the model by optimizing the logistic regression objective function.
   * Returns the objective function evaluated when the parameters are optimized.
   */
  double LearnModel();
};

}; // namespace regression
}; // namespace mlpack

// Include implementation.
#include "logistic_regression_impl.hpp"

#endif // __MLPACK_METHODS_LOGISTIC_REGRESSION_LOGISTIC_REGRESSION_HPP

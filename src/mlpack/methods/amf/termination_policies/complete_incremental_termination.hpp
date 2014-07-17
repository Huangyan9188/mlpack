#ifndef COMPLETE_INCREMENTAL_TERMINATION_HPP_INCLUDED
#define COMPLETE_INCREMENTAL_TERMINATION_HPP_INCLUDED

#include "amf/termination_policies/simple_tolerance_termination.hpp"

namespace mlpack
{
namespace amf
{

template <class TerminationPolicy>
class CompleteIncrementalTermination
{
 public:
  CompleteIncrementalTermination(TerminationPolicy t_policy = TerminationPolicy())
            : t_policy(t_policy) {}

  template <class MatType>
  void Initialize(const MatType& V)
  {
    t_policy.Initialize(V);

    incrementalIndex = accu(V != 0);
    iteration = 0;
  }

  void Initialize(const arma::sp_mat& V)
  {
    t_policy.Initialize(V);

    incrementalIndex = V.n_nonzero;
    iteration = 0;
  }

  bool IsConverged(arma::mat& W, arma::mat& H)
  {
    iteration++;
    if(iteration % incrementalIndex == 0)
      return t_policy.IsConverged(W, H);
    else return false;
  }

  const double& Index()
  {
    return t_policy.Index();
  }
  const size_t& Iteration()
  {
    return iteration;
  }

 private:
  TerminationPolicy t_policy;

  size_t incrementalIndex;
  size_t iteration;
};

} // namespace amf
} // namespace mlpack


#endif // COMPLETE_INCREMENTAL_TERMINATION_HPP_INCLUDED


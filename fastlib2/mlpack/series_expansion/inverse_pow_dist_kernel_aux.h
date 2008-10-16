#ifndef INVERSE_POW_DIST_KERNEL_AUX_H
#define INVERSE_POW_DIST_KERNEL_AUX_H

#include "inverse_pow_dist_kernel.h"

/** @brief The auxilary class for $r / ||r||^{\lambda}$ kernels using
 *         $O(D^p)$ expansion.
 */
class InversePowDistGradientKernelAux {

 private:

  void SubFrom_(index_t dimension, int decrement,
		const ArrayList<int> &subtract_from, 
		ArrayList<int> &result) const {
    
    for(index_t d = 0; d < subtract_from.size(); d++) {
      if(d == dimension) {
	result[d] = subtract_from[d] - decrement;
      }
      else {
	result[d] = subtract_from[d];
      }
    }
  }

 public:
  
  typedef InversePowDistGradientKernel TKernel;
  
  typedef SeriesExpansionAux TSeriesExpansionAux;
  
  typedef FarFieldExpansion<InversePowDistGradientKernelAux> TFarFieldExpansion;

  typedef LocalExpansion<InversePowDistGradientKernelAux> TLocalExpansion;

  /** @brief The actual kernel object.
   */
  TKernel kernel_;
  
  /** @brief The actual series expansion auxilary object.
   */
  TSeriesExpansionAux sea_;

  OT_DEF_BASIC(InversePowDistGradientKernelAux) {
    OT_MY_OBJECT(kernel_);
    OT_MY_OBJECT(sea_);
  }

 public:

  void Init(double bandwidth, int max_order, int dim) {
    kernel_.Init(bandwidth, dim);
    sea_.Init(max_order, dim);
  }

  void AllocateDerivativeMap(int dim, int order, 
			     Matrix *derivative_map) const {
    derivative_map->Init(sea_.get_total_num_coeffs(order), 1);
  }

  void ComputeDirectionalDerivatives(const Vector &x, 
				     Matrix *derivative_map, int order) const {

    derivative_map->SetZero();

    // Squared L2 norm of the vector.
    double squared_l2_norm = la::Dot(x, x);

    // Temporary variable to look for arithmetic operations on
    // multiindex.
    ArrayList<int> tmp_multiindex;
    tmp_multiindex.Init(sea_.get_dimension());

    for(index_t i = 0; i < derivative_map->n_rows(); i++) {
    
      // Contribution to the current multiindex position.
      double contribution = 0;

      // Retrieve the multiindex mapping.
      const ArrayList<int> &multiindex = sea_.get_multiindex(i);
      
      // $D_{x}^{0} \phi_{\nu, d}(x)$ should be computed normally.
      if(i == 0) {
	derivative_map->set(0, 0, kernel_.EvalUnnorm(x.ptr()));
	continue;
      }

      // Compute the contribution of $D_{x}^{n - e_d} \phi_{\nu,
      // d}(x)$ component for each $d$.
      for(index_t d = 0; d < x.length(); d++) {
	
	// Subtract 1 from the given dimension.
	SubFrom_(d, 1, multiindex, tmp_multiindex);
	index_t n_minus_e_d_position = 
	  sea_.ComputeMultiindexPosition(tmp_multiindex);
	if(n_minus_e_d_position >= 0) {
	  double factor = 2 * multiindex[d] * x[d];
	  if((kernel_.dimension_ == 0 && d == 1) ||
	     (kernel_.dimension_ > 0 && d == 0)) {
	    factor += (kernel_.lambda_ - 2);
	  }
	  contribution += factor * 
	    derivative_map->get(n_minus_e_d_position, 0);
	}
	
	// Subtract 2 from the given dimension.
	SubFrom_(d, 2, multiindex, tmp_multiindex);
	index_t n_minus_two_e_d_position =
	  sea_.ComputeMultiindexPosition(tmp_multiindex);
	if(n_minus_two_e_d_position >= 0) {
	  double factor = multiindex[d] * (multiindex[d] - 1);

	  if((kernel_.dimension_ == 0 && d == 1) ||
	     (kernel_.dimension_ > 0 && d == 0)) {
	    factor += (kernel_.lambda_ - 2) * (multiindex[d] - 1);
	  }

	  contribution += factor *
	    derivative_map->get(n_minus_two_e_d_position, 0);
	}
	
      } // end of iterating over each dimension.
      
      // Set the final contribution for this multiindex.
      derivative_map->set(i, 0, -contribution / squared_l2_norm);
      
    } // end of iterating over all required multiindex positions...
  }
  
  double ComputePartialDerivative(const Matrix &derivative_map,
				  const ArrayList<int> &mapping) const {
    
    return derivative_map.get(sea_.ComputeMultiindexPosition(mapping), 0);
  }

};

/** @brief The auxilary class for $1 / r^{\lambda}$ kernels using
 *         $O(D^p)$ expansion.
 */
class InversePowDistKernelAux {

 private:
  void SubFrom_(index_t dimension, int decrement,
		const ArrayList<int> &subtract_from, 
		ArrayList<int> &result) const {
    
    for(index_t d = 0; d < subtract_from.size(); d++) {
      if(d == dimension) {
	result[d] = subtract_from[d] - decrement;
      }
      else {
	result[d] = subtract_from[d];
      }
    }
  }

 public:
  
  typedef InversePowDistKernel TKernel;
  
  typedef SeriesExpansionAux TSeriesExpansionAux;
  
  typedef FarFieldExpansion<InversePowDistKernelAux> TFarFieldExpansion;

  typedef LocalExpansion<InversePowDistKernelAux> TLocalExpansion;

  /** @brief The actual kernel object.
   */
  TKernel kernel_;
  
  /** @brief The series expansion object.
   */
  TSeriesExpansionAux sea_;

  ////////// Public Functions //////////

  void Init(double power, int max_order, int dim) {
    kernel_.Init(power, dim);
    sea_.Init(max_order, dim);
  }

  void AllocateDerivativeMap(int dim, int order, 
			     Matrix *derivative_map) const {
    derivative_map->Init(sea_.get_total_num_coeffs(order), 1);
  }

  void ComputeDirectionalDerivatives(const Vector &x, 
				     Matrix *derivative_map, int order) const {

    derivative_map->SetZero();

    // Squared L2 norm of the vector.
    double squared_l2_norm = la::Dot(x, x);

    // Temporary variable to look for arithmetic operations on
    // multiindex.
    ArrayList<int> tmp_multiindex;
    tmp_multiindex.Init(sea_.get_dimension());

    // Get the inverse multiindex factorial factors.
    const Vector &inv_multiindex_factorials = 
      sea_.get_inv_multiindex_factorials();

    for(index_t i = 0; i < derivative_map->n_rows(); i++) {
    
      // Contribution to the current multiindex position.
      double contribution = 0;

      // Retrieve the multiindex mapping.
      const ArrayList<int> &multiindex = sea_.get_multiindex(i);
      
      // $D_{x}^{0} \phi_{\nu, d}(x)$ should be computed normally.
      if(i == 0) {
	derivative_map->set(0, 0, kernel_.EvalUnnorm(x.ptr()));
	continue;
      }

      // The sum of the indices.
      index_t sum_of_indices = 0;
      for(index_t d = 0; d < x.length(); d++) {
	sum_of_indices += multiindex[d];
      }

      // The first factor multiplied.
      double first_factor = 2 * sum_of_indices + kernel_.lambda_ - 2;

      // The second factor multilied.
      double second_factor = sum_of_indices + kernel_.lambda_ - 2;
      
      // Compute the contribution of $D_{x}^{n - e_d} \phi_{\nu,
      // d}(x)$ component for each $d$.
      for(index_t d = 0; d < x.length(); d++) {
	
	// Subtract 1 from the given dimension.
	SubFrom_(d, 1, multiindex, tmp_multiindex);
	index_t n_minus_e_d_position = 
	  sea_.ComputeMultiindexPosition(tmp_multiindex);
	if(n_minus_e_d_position >= 0) {
	  contribution += first_factor * x[d] *
	    derivative_map->get(n_minus_e_d_position, 0) *
	    inv_multiindex_factorials[n_minus_e_d_position];
	}
	
	// Subtract 2 from the given dimension.
	SubFrom_(d, 2, multiindex, tmp_multiindex);
	index_t n_minus_two_e_d_position =
	  sea_.ComputeMultiindexPosition(tmp_multiindex);
	if(n_minus_two_e_d_position >= 0) {
	  contribution += second_factor *
	    derivative_map->get(n_minus_two_e_d_position, 0) *
	    inv_multiindex_factorials[n_minus_two_e_d_position];
	}
	
      } // end of iterating over each dimension.
      
      // Set the final contribution for this multiindex.
      derivative_map->set(i, 0, -contribution / squared_l2_norm /
			  sum_of_indices / inv_multiindex_factorials[i]);
      
    } // end of iterating over all required multiindex positions...
  }
  
  double ComputePartialDerivative(const Matrix &derivative_map,
				  const ArrayList<int> &mapping) const {
    
    return derivative_map.get(sea_.ComputeMultiindexPosition(mapping), 0);
  }
};

#endif

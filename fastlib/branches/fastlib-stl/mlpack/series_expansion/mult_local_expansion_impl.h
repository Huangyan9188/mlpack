#ifndef INSIDE_MULT_LOCAL_EXPANSION_H
#error "This is not a public header file!"
#endif

#ifndef MULT_LOCAL_EXPANSION_IMPL_H
#define MULT_LOCAL_EXPANSION_IMPL_H

template<typename TKernelAux>
void MultLocalExpansion<TKernelAux>::AccumulateCoeffs(const arma::mat& data, 
						      const arma::vec& weights, 
						      size_t begin, size_t end, 
						      size_t order) {

  if(order > order_) {
    order_ = order;
  }

  size_t dim = sea_->get_dimension();
  size_t total_num_coeffs = sea_->get_total_num_coeffs(order);
  
  // get inverse factorials (precomputed)
  const arma::vec& neg_inv_multiindex_factorials = sea_->get_neg_inv_multiindex_factorials();

  // declare deritave mapping
  arma::mat derivative_map;
  ka_->AllocateDerivativeMap(dim, order, derivative_map);
  
  // some temporary variables
  arma::vec x_r_minus_x_Q(dim);
  
  // sqrt two times bandwidth
  double bandwidth_factor = ka_->BandwidthFactor(kernel_->bandwidth_sq());
  
  // get the order of traversal for the given order of approximation
  const std::vector<size_t> &traversal_order = 
    sea_->traversal_mapping_[order];

  // for each data point,
  for(size_t r = begin; r < end; r++) {
    
    // calculate x_r - x_Q
    for(size_t d = 0; d < dim; d++) {
      x_r_minus_x_Q[d] = (center_[d] - data(d, r)) / 
	bandwidth_factor;
    }
    
    // precompute necessary partial derivatives based on coordinate difference
    ka_->ComputeDirectionalDerivatives(x_r_minus_x_Q, derivative_map, order);
    
    // compute h_{beta}((x_r - x_Q) / sqrt(2h^2))
    for(size_t j = 0; j < total_num_coeffs; j++) {
      size_t index = traversal_order[j];
      const std::vector<size_t>& mapping = sea_->get_multiindex(index);
      double partial_derivative = 
	ka_->ComputePartialDerivative(derivative_map, mapping);
      coeffs_[index] += neg_inv_multiindex_factorials[index] * weights[r] * 
	partial_derivative;
    }
  } // End of looping through each reference point.
}

template<typename TKernelAux>
void MultLocalExpansion<TKernelAux>::PrintDebug(const char *name, 
						FILE *stream) const {
  
    
  size_t dim = sea_->get_dimension();
  size_t total_num_coeffs = sea_->get_total_num_coeffs(order_);

  fprintf(stream, "----- SERIESEXPANSION %s ------\n", name);
  fprintf(stream, "Local expansion\n");
  fprintf(stream, "Center: ");
  
  for (size_t i = 0; i < center_.n_elem; i++) {
    fprintf(stream, "%g ", center_[i]);
  }
  fprintf(stream, "\n");
  
  fprintf(stream, "f(");
  for(size_t d = 0; d < dim; d++) {
    fprintf(stream, "x_q%d", d);
    if(d < dim - 1)
      fprintf(stream, ",");
  }
  fprintf(stream, ") = \\sum\\limits_{x_r \\in R} K(||x_q - x_r||) = ");
  
  for (size_t i = 0; i < total_num_coeffs; i++) {
    std::vector<size_t> mapping = sea_->get_multiindex(i);
    fprintf(stream, "%g", coeffs_[i]);
    
    for(size_t d = 0; d < dim; d++) {
      fprintf(stream, "(x_q%d - (%g))^%d ", d, center_[d], mapping[d]);
    }

    if(i < total_num_coeffs - 1) {
      fprintf(stream, " + ");
    }
  }
  fprintf(stream, "\n");
}

template<typename TKernelAux>
double MultLocalExpansion<TKernelAux>::EvaluateField(const arma::mat& data,
						     size_t row_num) const {
    
  // if there are no local coefficients, then return 0
  if(order_ < 0) {
    return 0;
  }

  // total number of coefficient
  size_t total_num_coeffs = sea_->get_total_num_coeffs(order_);

  // number of dimensions
  size_t dim = sea_->get_dimension();

  // evaluated sum to be returned
  double sum = 0;
  
  // sqrt two bandwidth
  double bandwidth_factor = ka_->BandwidthFactor(kernel_->bandwidth_sq());

  // temporary variable
  arma::vec x_Q_to_x_q(dim);
  arma::vec tmp(sea_->get_max_total_num_coeffs());
  std::vector<size_t> heads(dim + 1);
  
  // compute (x_q - x_Q) / (sqrt(2h^2))
  for(size_t i = 0; i < dim; i++) {
    x_Q_to_x_q[i] = (data(i, row_num) - center_[i]) / bandwidth_factor;
  }
  
  for(size_t i = 0; i < dim; i++)
    heads[i] = 0;
  heads[dim] = SHRT_MAX;

  tmp[0] = 1.0;

  // get the order of traversal for the given order of approximation
  const std::vector<size_t>& traversal_order = 
    sea_->traversal_mapping_[order_];

  for(size_t i = 1; i < total_num_coeffs; i++) {
    
    size_t index = traversal_order[i];
    const std::vector<size_t> &lower_mappings = 
      sea_->lower_mapping_index_[index];
    
    // from the direct descendant, recursively compute the multipole moments
    size_t direct_ancestor_mapping_pos = 
      lower_mappings[lower_mappings.size() - 2];
    size_t position = 0;
    const std::vector<size_t>& mapping = sea_->multiindex_mapping_[index];
    const std::vector<size_t>& direct_ancestor_mapping = 
      sea_->multiindex_mapping_[direct_ancestor_mapping_pos];
    for(size_t i = 0; i < dim; i++) {
      if(mapping[i] != direct_ancestor_mapping[i]) {
	position = i;
	break;
      }
    }
    tmp[index] = tmp[direct_ancestor_mapping_pos] * x_Q_to_x_q[position];
  }

  for(size_t i = 0; i < total_num_coeffs; i++) {
    size_t index = traversal_order[i];
    sum += coeffs_[index] * tmp[index];
  }

  return sum;
}

template<typename TKernelAux>
double MultLocalExpansion<TKernelAux>::EvaluateField(const arma::vec& x_q) const {
    
  // if there are no local coefficients, then return 0
  if(order_ < 0) {
    return 0;
  }

  // total number of coefficient
  size_t total_num_coeffs = sea_->get_total_num_coeffs(order_);

  // number of dimensions
  size_t dim = sea_->get_dimension();

  // evaluated sum to be returned
  double sum = 0;
  
  // sqrt two bandwidth
  double bandwidth_factor = ka_->BandwidthFactor(kernel_.bandwidth_sq());

  // temporary variable
  arma::vec x_Q_to_x_q(dim);
  arma::vec tmp(sea_->get_max_total_num_coeffs());
  std::vector<size_t> heads(dim + 1);
  
  // compute (x_q - x_Q) / (sqrt(2h^2))
  for(size_t i = 0; i < dim; i++) {
    x_Q_to_x_q[i] = (x_q[i] - center_[i]) / bandwidth_factor;
  }
  
  for(size_t i = 0; i < dim; i++)
    heads[i] = 0;
  heads[dim] = UINT_MAX;

  tmp[0] = 1.0;

  // get the order of traversal for the given order of approximation
  std::vector<size_t>& traversal_order = sea_->traversal_mapping_[order_];

  for(size_t i = 1; i < total_num_coeffs; i++) {
    
    size_t index = traversal_order[i];
    std::vector<size_t> &lower_mappings = sea_->lower_mapping_index_[index];
    
    // from the direct descendant, recursively compute the multipole moments
    size_t direct_ancestor_mapping_pos = 
      lower_mappings[lower_mappings.size() - 2];
    size_t position = 0;
    const std::vector<size_t> &mapping = sea_->multiindex_mapping_[index];
    const std::vector<size_t> &direct_ancestor_mapping = 
      sea_->multiindex_mapping_[direct_ancestor_mapping_pos];
    for(size_t i = 0; i < dim; i++) {
      if(mapping[i] != direct_ancestor_mapping[i]) {
	position = i;
	break;
      }
    }
    tmp[index] = tmp[direct_ancestor_mapping_pos] * x_Q_to_x_q[position];
  }

  for(size_t i = 0; i < total_num_coeffs; i++) {
    size_t index = traversal_order[i];
    sum += coeffs_[index] * tmp[index];
  }

  return sum;
}

template<typename TKernelAux>
void MultLocalExpansion<TKernelAux>::Init(const arma::vec& center, 
					  const TKernelAux &ka) {
  
  // copy kernel type, center, and bandwidth squared
  kernel_ = &(ka.kernel_);
  center_ = center;
  order_ = -1;
  sea_ = &(ka.sea_);
  ka_ = &ka;

  // initialize coefficient array
  coeffs_.zeros(sea_->get_max_total_num_coeffs());
}

template<typename TKernelAux>
void MultLocalExpansion<TKernelAux>::Init(const TKernelAux &ka) {
  
  // copy kernel type, center, and bandwidth squared
  kernel_ = &(ka.kernel_);
  sea_ = &(ka.sea_);
  // initialize coefficient array
  center_.zeros(sea_->get_dimension());
  order_ = -1;
  ka_ = &ka;
}

template<typename TKernelAux>
template<typename TBound>
size_t MultLocalExpansion<TKernelAux>::OrderForEvaluating
(const TBound &far_field_region, 
 const TBound &local_field_region, double min_dist_sqd_regions,
 double max_dist_sqd_regions, double max_error, double *actual_error) const {
  
  return ka_->OrderForEvaluatingLocal(far_field_region, local_field_region, 
				     min_dist_sqd_regions,
				     max_dist_sqd_regions, max_error, 
				     actual_error);
}

template<typename TKernelAux>
void MultLocalExpansion<TKernelAux>::TranslateFromFarField
(const MultFarFieldExpansion<TKernelAux> &se) {

  arma::vec pos_arrtmp, neg_arrtmp;
  arma::mat derivative_map;
  arma::vec& far_center;
  arma::vec cent_diff;
  arma::vec& far_coeffs;
  size_t dimension = sea_->get_dimension();
  size_t far_order = se.get_order();
  size_t total_num_coeffs = sea_->get_total_num_coeffs(far_order);
  size_t limit;
  double bandwidth_factor = ka_->BandwidthFactor(se.bandwidth_sq());

  ka_->AllocateDerivativeMap(dimension, 2 * order_, &derivative_map);

  // get center and coefficients for far field expansion
  far_center = se.get_center();
  far_coeffs = se.get_coeffs();
  cent_diff.set_size(dimension);

  // if the order of the far field expansion is greater than the
  // local one we are adding onto, then increase the order.
  if(far_order > order_) {
    order_ = far_order;
  }

  // compute Gaussian derivative
  pos_arrtmp.set_size(total_num_coeffs);
  neg_arrtmp.set_size(total_num_coeffs);

  // compute center difference divided by bw_times_sqrt_two;
  for(size_t j = 0; j < dimension; j++) {
    cent_diff[j] = (center_[j] - far_center[j]) / bandwidth_factor;
  }

  // compute required partial derivatives
  ka_->ComputeDirectionalDerivatives(cent_diff, &derivative_map, 2 * order_);
  std::vector<size_t> beta_plus_alpha;
  beta_plus_alpha.reserve(dimension);

  // get the order of traversal for the given order of approximation
  std::vector<size_t> &traversal_order = sea_->traversal_mapping_[far_order];

  for(size_t j = 0; j < total_num_coeffs; j++) {

    size_t index_j = traversal_order[j];
    std::vector<size_t> beta_mapping = sea_->get_multiindex(index_j);
    pos_arrtmp[index_j] = neg_arrtmp[index_j] = 0;

    for(size_t k = 0; k < total_num_coeffs; k++) {

      size_t index_k = traversal_order[k];
      std::vector<size_t> alpha_mapping = sea_->get_multiindex(index_k);
      for(size_t d = 0; d < dimension; d++) {
	beta_plus_alpha[d] = beta_mapping[d] + alpha_mapping[d];
      }
      double derivative_factor =
	ka_->ComputePartialDerivative(derivative_map, beta_plus_alpha);
      
      double prod = far_coeffs[index_k] * derivative_factor;

      if(prod > 0) {
	pos_arrtmp[index_j] += prod;
      }
      else {
	neg_arrtmp[index_j] += prod;
      }
    } // end of k-loop
  } // end of j-loop

  arma::vec C_k_neg = sea_->get_neg_inv_multiindex_factorials();
  for(size_t j = 0; j < total_num_coeffs; j++) {
    size_t index_j = traversal_order[j];
    coeffs_[index_j] += (pos_arrtmp[index_j] + neg_arrtmp[index_j]) * 
      C_k_neg[index_j];
  }
}
  
template<typename TKernelAux>
void MultLocalExpansion<TKernelAux>::TranslateToLocal(MultLocalExpansion &se) {

  // if no local coefficients have formed, then nothing to translate
  if(order_ < 0) {
    return;
  }

  // get the center and the order and the total number of coefficients of 
  // the expansion we are translating from. Also get coefficients we
  // are translating
  arma::vec& new_center = se.get_center();
  size_t prev_order = se.get_order();
  size_t total_num_coeffs = sea_->get_total_num_coeffs(order_);
  const std::vector<std::vector<size_t> >& upper_mapping_index = 
    sea_->get_upper_mapping_index();
  arma::vec& new_coeffs = se.get_coeffs();

  // dimension
  size_t dim = sea_->get_dimension();

  // temporary variable
  std::vector<size_t> tmp_storage;
  tmp_storage.reserve(dim);

  // sqrt two times bandwidth
  double bandwidth_factor = ka_->BandwidthFactor(kernel_->bandwidth_sq());

  // center difference between the old center and the new one
  arma::vec center_diff(dim);
  for(size_t d = 0; d < dim; d++) {
    center_diff[d] = (new_center[d] - center_[d]) / bandwidth_factor;
  }

  // set to the new order if the order of the expansion we are translating
  // from is higher
  if(prev_order < order_) {
    se.set_order(order_);
  }

  // inverse multiindex factorials
  //const arma::vec& C_k = sea_->get_inv_multiindex_factorials();

  // get the order of traversal for the given order of approximation
  const std::vector<size_t> &traversal_order =
    sea_->traversal_mapping_[order_];

  // do the actual translation
  for(size_t j = 0; j < total_num_coeffs; j++) {

    size_t index_j = traversal_order[j];
    const std::vector<size_t> &alpha_mapping = sea_->get_multiindex(index_j);
    const std::vector<size_t> &upper_mappings_for_alpha = 
      upper_mapping_index[index_j];
    double pos_coeffs = 0;
    double neg_coeffs = 0;

    for(size_t k = 0; k < upper_mappings_for_alpha.size(); k++) {

      if(upper_mappings_for_alpha[k] >= total_num_coeffs) {
	break;
      }

      const std::vector<size_t> &beta_mapping = 
	sea_->get_multiindex(upper_mappings_for_alpha[k]);
      size_t flag = 0;
      double diff1 = 1.0;

      for(size_t l = 0; l < dim; l++) {
	tmp_storage[l] = beta_mapping[l] - alpha_mapping[l];

	if(tmp_storage[l] < 0) {
	  flag = 1;
	  break;
	}
      } // end of looping over dimension
      
      if(flag)
	continue;

      for(size_t l = 0; l < dim; l++) {
	diff1 *= pow(center_diff[l], tmp_storage[l]);
      }

      double prod = coeffs_[upper_mappings_for_alpha[k]] * diff1 *
	sea_->get_n_multichoose_k_by_pos
	(upper_mappings_for_alpha[k], index_j);
      
      if(prod > 0) {
	pos_coeffs += prod;
      }
      else {
	neg_coeffs += prod;
      }

    } // end of k loop

    new_coeffs[index_j] += pos_coeffs + neg_coeffs;
  } // end of j loop
}

#endif

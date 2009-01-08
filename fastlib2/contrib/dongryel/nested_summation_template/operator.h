#ifndef OPERATOR_H
#define OPERATOR_H

#include "fastlib/fastlib.h"

class Operator {

 private:

  /** @brief The nested operators under this operator.
   */
  ArrayList<Operator *> operators_;

  /** @brief The dataset index that must be set for this operator.
   */
  index_t dataset_index_;

  /** @brief The list of restrictions for each dataset index. This is
   *         a global pointer
   */
  std::map<index_t, std::vector<index_t> > *restrictions_;

  /** @brief The ordered list of datasets. This is a global list of
   *         datasets.
   */
  ArrayList<Matrix *> datasets_;

  /** @brief A boolean flag that specifies whether the recursive
   *         result should be negated or not.
   */
  bool is_positive_;

  /** @brief A boolean flag that specifies whether the recursive
   *         result should be inverted or not.
   */
  bool should_be_inverted_;

  OT_DEF_BASIC(Operator) {
    OT_MY_OBJECT(operators_);
    OT_MY_OBJECT(dataset_index_);
    OT_MY_OBJECT(is_positive_);
    OT_MY_OBJECT(should_be_inverted_);
  }

  bool CheckViolation_
  (const std::map<index_t, index_t> &previous_constant_dataset_indices,
   const std::vector<index_t> &restriction_vector, index_t new_point_index) {

    for(index_t n = 0; n < restriction_vector.size(); n++) {
      
      // Look up the point index chosen for the current
      // restriction...
      index_t restriction_dataset_index = restriction_vector[n];
      
      if(previous_constant_dataset_indices.find(new_point_index) !=
	 previous_constant_dataset_indices.end()) {
	
	return true;
      }
    }
    return false;
  }

  void ChoosePointIndex_
  (std::map<index_t, index_t> &previous_constant_dataset_indices) {
    
    // Choose a new point index subject to the existing restrictions...
    index_t new_point_index;
    bool done_flag = true;

    // Get the list of restrictions associated with the current
    // dataset index.
    std::map<index_t, std::vector<int> >::iterator restriction = 
      restrictions_->find(dataset_index_);
    const std::vector<index_t> &restriction_vector = (*restriction).second;

    // The pointer to the current dataset.
    const Matrix *current_dataset = datasets_[dataset_index_];

    if(restriction != restrictions_->end()) {
      
      do {
	
	// Reset the flag.
	done_flag = true;
	
	// Randomly choose the point index and check whether it
	// satisfies all constraints...
	new_point_index = math::RandInt(0, current_dataset->n_cols());
	done_flag = CheckViolation_(previous_constant_dataset_indices,
				    restriction_vector, new_point_index);
	
	// Repeat until all restrictions are satisfied...

      } while(!done_flag);
    }
    else {
      new_point_index = math::RandInt(0, current_dataset->n_cols());
    }

    previous_constant_data_indices[dataset_index_] = new_point_index;
  }

  double PostProcess_(std::map<index_t, index_t> &constant_dataset_indices,
		      double sub_result) {

    double result = sub_result;

    if(!is_positive_) {
      result = -result;
    }
    if(should_be_inverted_) {
      result = 1.0 / result;
    }
    
    // Erase the point index associated with the current index.
    constant_dataset_indices.erase(dataset_index_);

    return result;
  }

 public:

  /** @brief Evaluate the operator exactly.
   */
  virtual double NaiveCompute
  (std::map<index_t, index_t> &constant_dataset_indices) = 0;

  /** @brief Evaluate the operator using Monte Carlo.
   */
  virtual double MonteCarloCompute
  (std::map<index_t, index_t> &constant_dataset_indices) = 0;

  const ArrayList<index_t> &dataset_indices() {
    return dataset_indices_;
  }
  
  const std::map<index_t, std::vector<index_t> > *restrictions() {
    return restrictions_;
  }

  const ArrayList<Operator *> &child_operators() {
    return operators_;
  }

};

#endif

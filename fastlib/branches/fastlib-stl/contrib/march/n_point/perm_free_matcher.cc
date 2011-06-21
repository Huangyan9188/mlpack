/*
 *  perm_free_matcher.cc
 *  
 *
 *  Created by William March on 2/7/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "perm_free_matcher.h"



bool npt::PermFreeMatcher::TestNodeTuple(NodeTuple& nodes) {
  
  for (index_t i = 0; i < upper_bounds_sqr_.size(); i++) {
    
    if (nodes.lower_bound(i) > upper_bounds_sqr_[i]) {
      return false;
    }

    
    if (nodes.upper_bound(i) < lower_bounds_sqr_[i]) {
      return false;
    }
     
  } // for i
  
/*
 // Thinking about trying something with the triangle inequality here
  for (index_t i = 0; i < tuple_size_; i++) {
    for (index_t j = i+1; j < tuple_size_; j++) {
      for (index_t k = j+1; k < tuple_size_; k++) {
        
        if () {
          
        }
        
      } // for k
    } // for j
  } // for i
  */
  return true;
  
} // TestNodeTuple




bool npt::PermFreeMatcher::TestPointPair(double dist_sq, index_t tuple_index_1, 
                                         index_t tuple_index_2, 
                                         std::vector<bool>& permutation_ok) {
  
  
  bool any_matches = false;
  
  for (index_t i = 0; i < perms_.num_permutations(); i++) {
    
    if (!(permutation_ok[i])) {
      continue;
    } // does this permutation work?
    
    index_t template_index_1 = GetPermutationIndex_(i, tuple_index_1);
    index_t template_index_2 = GetPermutationIndex_(i, tuple_index_2);
    
    
    if (dist_sq <= upper_bounds_sqr_mat_(template_index_1, template_index_2) &&
        dist_sq >= lower_bounds_sqr_mat_(template_index_1, template_index_2)) {
      any_matches = true;
    } // this placement works
    else {
      permutation_ok[i] = false;
    } // this placement doesn't work
    
  } // for i
  
  return any_matches;  
  
  
} // TestPointPair



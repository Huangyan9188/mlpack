#include "fastlib/fastlib.h"

namespace tree_cfmm_tree_private {

  void SwapValues(Vector &v, index_t first_index, index_t second_index) {
    
    double tmp_value = v[first_index];
    v[first_index] = v[second_index];
    v[second_index] = tmp_value;
  }

  index_t MatrixPartitionByTargets
  (index_t particle_set_number, ArrayList<Matrix *> &matrices,
   ArrayList<Vector *> &targets, index_t dim, double splitvalue, index_t first,
   index_t count, ArrayList< ArrayList<index_t> > *old_from_new) {
    
    index_t left = first;
    index_t right = first + count - 1;
    
    /* At any point:
     *
     *   everything < left is correct
     *   everything > right is correct
     */
    for (;;) {
      while(likely(left <= right) &&
	    (*targets[particle_set_number])[left] < splitvalue) {
        left++;
      }
      
      while(likely(left <= right) && 
	    (*targets[particle_set_number])[right] >= splitvalue) {
        right--;
      }

      if (unlikely(left > right)) {
        /* left == right + 1 */
        break;
      }

      Vector left_vector;
      Vector right_vector;

      matrices[particle_set_number]->MakeColumnVector(left, &left_vector);
      matrices[particle_set_number]->MakeColumnVector(right, &right_vector);

      // Swap the vectors and the targets.
      left_vector.SwapValues(&right_vector);
      SwapValues(*(targets[particle_set_number]), left, right);
      
      if (old_from_new) {
        index_t t = (*old_from_new)[particle_set_number][left];
        (*old_from_new)[particle_set_number][left] = 
	  (*old_from_new)[particle_set_number][right];
        (*old_from_new)[particle_set_number][right] = t;
      }
      
      DEBUG_ASSERT(left <= right);
      right--;
    } // end of the infinite loop...
    
    DEBUG_ASSERT(left == right + 1);

    return left;
  }

  index_t MatrixPartition(index_t particle_set_number, 
			  ArrayList<Matrix *> &matrices,
			  ArrayList<Vector *> &targets, index_t dim, 
			  double splitvalue, index_t first, index_t count, 
			  ArrayList< ArrayList<index_t> > *old_from_new) {
    
    index_t left = first;
    index_t right = first + count - 1;
    
    /* At any point:
     *
     *   everything < left is correct
     *   everything > right is correct
     */
    for (;;) {
      while (likely(left <= right) &&
	     matrices[particle_set_number]->get(dim, left) < splitvalue) {
        left++;
      }

      while (likely(left <= right) && 
	     matrices[particle_set_number]->get(dim, right) >= splitvalue) {
        right--;
      }

      if (unlikely(left > right)) {
        /* left == right + 1 */
        break;
      }

      Vector left_vector;
      Vector right_vector;

      matrices[particle_set_number]->MakeColumnVector(left, &left_vector);
      matrices[particle_set_number]->MakeColumnVector(right, &right_vector);

      // Swap the vectors and the targets.
      left_vector.SwapValues(&right_vector);
      SwapValues(*(targets[particle_set_number]), left, right);
      
      if (old_from_new) {
        index_t t = (*old_from_new)[particle_set_number][left];
        (*old_from_new)[particle_set_number][left] = 
	  (*old_from_new)[particle_set_number][right];
        (*old_from_new)[particle_set_number][right] = t;
      }
      
      DEBUG_ASSERT(left <= right);
      right--;
    } // end of the infinite loop...
    
    DEBUG_ASSERT(left == right + 1);

    return left;
  }

  template<typename TStatistic>
  bool RecursiveMatrixPartition
  (ArrayList<Matrix *> &matrices, CFmmTree<TStatistic> *node, index_t count,
   ArrayList<index_t> &child_begin, ArrayList<index_t> &child_count,
   ArrayList< ArrayList<CFmmTree<TStatistic> *> > *nodes_in_each_level,
   ArrayList< ArrayList<index_t> > *old_from_new, const int level, 
   int recursion_level, unsigned int code) {
    
    if(recursion_level < matrices[0]->n_rows()) {
      const DRange &range_in_this_dimension = node->bound().get
	(recursion_level);
      double split_value = 0.5 * (range_in_this_dimension.lo +
				  range_in_this_dimension.hi);
      
      // Partition based on the current dimension.
      index_t total_left_count = 0;
      index_t total_right_count = 0;

      // Temporary ArrayList for passing in the indices owned by the
      // children.
      ArrayList<index_t> left_child_begin;
      ArrayList<index_t> left_child_count;
      ArrayList<index_t> right_child_begin;
      ArrayList<index_t> right_child_count;
      left_child_begin.Init(matrices.size());
      left_child_count.Init(matrices.size());
      right_child_begin.Init(matrices.size());
      right_child_count.Init(matrices.size());

      // Divide each particle set.
      for(index_t particle_set_number = 0; 
	  particle_set_number < matrices.size(); particle_set_number++) {

	// If there is nothing to divide for the current particle set,
	// then skipt.
	if(child_count[particle_set_number] == 0) {
	  left_child_begin[particle_set_number] = -1;
	  left_child_count[particle_set_number] = 0;
	  right_child_begin[particle_set_number] = -1;
	  right_child_count[particle_set_number] = 0;
	  continue;
	}

	index_t left_count = MatrixPartition(particle_set_number, matrices, 
					     recursion_level, split_value, 
					     child_begin[particle_set_number],
					     child_count[particle_set_number],
					     old_from_new) - 
	  child_begin[particle_set_number];
	index_t right_count = child_count[particle_set_number] - left_count;

	// Divide into two sets.
	left_child_count[particle_set_number] = left_count;
	right_child_count[particle_set_number] = right_count;  
	if(left_count > 0) {
	  left_child_begin[particle_set_number] = 
	    child_begin[particle_set_number];
	}
	else {
	  left_child_begin[particle_set_number] = -1;
	}
	if(right_count > 0) {
	  right_child_begin[particle_set_number] = 
	    child_begin[particle_set_number] + left_count;
	}
	else {
	  right_child_begin[particle_set_number] = -1;
	}
	
	total_left_count += left_count;
	total_right_count += right_count;
      }

      bool left_result = false;
      bool right_result = false;

      if(total_left_count > 0) {
	left_result =
	  RecursiveMatrixPartition
	  (matrices, node, total_left_count, left_child_begin, 
	   left_child_count, nodes_in_each_level, old_from_new, level, 
	   recursion_level + 1, 2 * code);
      }
      if(total_right_count > 0) {
	right_result =
	  RecursiveMatrixPartition
	  (matrices, node, total_right_count, right_child_begin, 
	   right_child_count, nodes_in_each_level, old_from_new, level, 
	   recursion_level + 1, 2 * code + 1);
      }
      
      return left_result || right_result;
    }
    else {

      // Create the child. From the code, also set the bounding cube
      // of half the side length.
      CFmmTree<TStatistic> *new_child =
	node->AllocateNewChild(matrices.size(), matrices[0]->n_rows(),
			       (node->node_index() << 
				matrices[0]->n_rows()) + code);
      
      // Set the level one more of the parent.
      new_child->set_level(node->level() + 1);

      // Appropriately set the membership in each particle set.
      for(index_t p = 0; p < matrices.size(); p++) {
	new_child->Init(p, child_begin[p], child_count[p]);
      }

      // Push the newly created child onto the list.
      *(((*nodes_in_each_level)[level + 1]).PushBackRaw()) = new_child;
      new_child->bound().Init(matrices[0]->n_rows());
      
      Vector lower_coord, upper_coord;
      lower_coord.Init(matrices[0]->n_rows());
      upper_coord.Init(matrices[0]->n_rows());

      for(index_t d = matrices[0]->n_rows() - 1; d >= 0; d--) {
	const DRange &range_in_this_dimension = 
	  node->bound().get(matrices[0]->n_rows() - 1 - d);

	if((code & (1 << d)) > 0) {
	  lower_coord[matrices[0]->n_rows() - 1 - d] = 0.5 * 
	    (range_in_this_dimension.lo + range_in_this_dimension.hi);
	  upper_coord[matrices[0]->n_rows() - 1 - d] = 
	    range_in_this_dimension.hi;
	}
	else {
	  lower_coord[matrices[0]->n_rows() - 1 - d] = 
	    range_in_this_dimension.lo;
	  upper_coord[matrices[0]->n_rows() - 1 - d] = 
	    0.5 * (range_in_this_dimension.lo + range_in_this_dimension.hi);
	}
      }
      new_child->bound() |= lower_coord;
      new_child->bound() |= upper_coord;

      return true;
    }
  }

  template<typename TStatistic>
  void ComputeBoundingHypercube(const ArrayList<Matrix *> &matrices,
				CFmmTree<TStatistic> *node) {

    // Initialize the bound.
    node->bound().Init(matrices[0]->n_rows());

    // Iterate over each point owned by the node and compute its
    // bounding hypercube.
    for(index_t n = 0; n < matrices.size(); n++) {
      for(index_t i = node->begin(n); i < node->end(n); i++) {
	Vector point;
	matrices[n]->MakeColumnVector(i, &point);
	node->bound() |= point;
      }
    }

    // Compute the longest side and correct the maximum coordinate of
    // the bounding box accordingly.
    double max_side_length = 0;
    for(index_t d = 0; d < matrices[0]->n_rows(); d++) {
      const DRange &range_in_this_dimension = node->bound().get(d);
      double side_length = range_in_this_dimension.hi -
	range_in_this_dimension.lo;
      max_side_length = std::max(max_side_length, side_length);
    }
    Vector new_upper_coordinate;
    new_upper_coordinate.Init(matrices[0]->n_rows());
    for(index_t d = 0; d < matrices[0]->n_rows(); d++) {
      const DRange &range_in_this_dimension = node->bound().get(d);
      new_upper_coordinate[d] = range_in_this_dimension.lo + 
	max_side_length;
    } 
    node->bound() |= new_upper_coordinate;
  }

  template<typename TStatistic>
  void SplitCFmmTree
  (ArrayList<Matrix *> &matrices, CFmmTree<TStatistic> *node, 
   index_t leaf_size,
   ArrayList< ArrayList<CFmmTree<TStatistic> *> > *nodes_in_each_level,
   ArrayList< ArrayList<index_t> > *old_from_new, index_t level) {

    // If the node is just too small, then do not split.
    if(node->count() <= leaf_size) {
    }
    
    // Otherwise, attempt to split.
    else {

      // Recursively split each dimension.
      unsigned int code = 0;
      ArrayList<index_t> child_begin;
      ArrayList<index_t> child_count;
      child_begin.Init(matrices.size());
      child_count.Init(matrices.size());
      for(index_t i = 0; i < matrices.size(); i++) {
	child_begin[i] = node->begin(i);
	child_count[i] = node->count(i);
      }

      bool can_cut = (node->side_length() > DBL_EPSILON) &&
	RecursiveMatrixPartition
	(matrices, node, node->count(), child_begin, child_count,
	 nodes_in_each_level, old_from_new, level, 0, code);

      if(can_cut) {
	for(index_t i = 0; i < node->num_children(); i++) {
	  CFmmTree<TStatistic> *child_node = node->get_child(i);
	  SplitCFmmTree(matrices, child_node, leaf_size, nodes_in_each_level,
			old_from_new, level + 1);
	}
      }
    }
  }
};

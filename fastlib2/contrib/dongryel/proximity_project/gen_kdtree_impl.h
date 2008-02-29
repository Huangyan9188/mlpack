/* Implementation for the regular pointer-style spill kd-tree builder. */

#include "fastlib/fastlib_int.h"

namespace tree_gen_kdtree_private {

  template<typename TBound>
    void FindBoundFromMatrix(const Matrix& matrix,
			     index_t first, index_t count, TBound *bounds) {
    index_t end = first + count;
    for (index_t i = first; i < end; i++) {
      Vector col;
      
      matrix.MakeColumnVector(i, &col);
      *bounds |= col;
    }
  }
  
  template<typename TBound>
    index_t MatrixPartition(Matrix& matrix, index_t dim, double splitvalue,
			    index_t first, index_t count,
			    TBound* left_bound, TBound* right_bound,
			    index_t *old_from_new) {
    index_t left = first;
    index_t right = first + count - 1;
    
    /* At any point:
     *
     *   everything < left is correct
     *   everything > right is correct
     */
    for (;;) {
      while (matrix.get(dim, left) < splitvalue && likely(left <= right)) {
        Vector left_vector;
        matrix.MakeColumnVector(left, &left_vector);
        *left_bound |= left_vector;
        left++;
      }

      while (matrix.get(dim, right) >= splitvalue && likely(left <= right)) {
        Vector right_vector;
        matrix.MakeColumnVector(right, &right_vector);
        *right_bound |= right_vector;
        right--;
      }

      if (unlikely(left > right)) {
        /* left == right + 1 */
        break;
      }

      Vector left_vector;
      Vector right_vector;

      matrix.MakeColumnVector(left, &left_vector);
      matrix.MakeColumnVector(right, &right_vector);

      left_vector.SwapValues(&right_vector);

      *left_bound |= left_vector;
      *right_bound |= right_vector;
      
      if (old_from_new) {
        index_t t = old_from_new[left];
        old_from_new[left] = old_from_new[right];
        old_from_new[right] = t;
      }
      
      DEBUG_ASSERT(left <= right);
      right--;
    }
    
    DEBUG_ASSERT(left == right + 1);

    return left;
  }

  template<typename TKdTree, typename TKdTreeSplitter>
    void SplitGenKdTree(Matrix& matrix, TKdTree *node, 
			index_t leaf_size, index_t *old_from_new) {
    TKdTree *left = NULL;
    TKdTree *right = NULL;

    if (node->count() > leaf_size) {
      index_t split_dim = BIG_BAD_NUMBER;
      double max_width = -1;

      for (index_t d = 0; d < matrix.n_rows(); d++) {
        double w = node->bound().get(d).width();

        if (unlikely(w > max_width)) {
          max_width = w;
          split_dim = d;
        }
      }
      
      // choose the split value along the dimension to be splitted
      double split_val = 
	TKdTreeSplitter::ChooseKdTreeSplitValue(matrix, node, split_dim);
    
      if (max_width == 0) {
        // Okay, we can't do any splitting, because all these points are the
        // same.  We have to give up.
      } 
      else {
        left = new TKdTree();
        left->bound().Init(matrix.n_rows());
	
        right = new TKdTree();
        right->bound().Init(matrix.n_rows());

        index_t split_col = MatrixPartition(matrix, split_dim, split_val,
					    node->begin(), node->count(),
					    &left->bound(), &right->bound(),
					    old_from_new);
	
	VERBOSE_MSG(3.0,"split (%d,[%d],%d) dim %d on %f (between %f, %f)",
		    node->begin(), split_col,
		    node->begin() + node->count(), split_dim, split_val,
		    node->bound().get(split_dim).lo,
		    node->bound().get(split_dim).hi);

	left->Init(node->begin(), split_col - node->begin());
	right->Init(split_col, node->begin() + node->count() - split_col);
	
	SplitGenKdTree<TKdTree, TKdTreeSplitter>
	  (matrix, left, leaf_size, old_from_new);
	SplitGenKdTree<TKdTree, TKdTreeSplitter>
	  (matrix, right, leaf_size, old_from_new);
      }
    }

    node->set_children(matrix, left, right);
  }
};

/**
  * @file rectangle_tree_traverser.hpp
  * @author Andrew Wells
  *
  * A nested class of Rectangle Tree for traversing rectangle type trees
  * with a given set of rules which indicate the branches to prune and the
  * order in which to recurse.  This is a depth-first traverser.
  */
#ifndef __MLPACK_CORE_TREE_RECTANGLE_TREE_RECTANGLE_TREE_TRAVERSER_HPP
#define __MLPACK_CORE_TREE_RECTANGLE_TREE_RECTANGLE_TREE_TRAVERSER_HPP

#include <mlpack/core.hpp>

#include "rectangle_tree.hpp"

namespace mlpack {
namespace tree {

template<typename SplitType,
         typename DescentType,
	 typename StatisticType,
         typename MatType>
template<typename RuleType>
class RectangleTree<SplitType, DescentType, StatisticType, MatType>::
    RectangleTreeTraverser
{
 public:
  /**
    * Instantiate the traverser with the given rule set.
    */
    RectangleTreeTraverser(RuleType& rule);

  /**
    * Traverse the tree with the given point.
    *
    * @param queryIndex The index of the point in the query set which is being
    *     used as the query point.
    * @param referenceNode The tree node to be traversed.
    */
  void Traverse(const size_t queryIndex, const RectangleTree& referenceNode);

  //! Get the number of prunes.
  size_t NumPrunes() const { return numPrunes; }
  //! Modify the number of prunes.
  size_t& NumPrunes() { return numPrunes; }

 private:
  //! Reference to the rules with which the tree will be traversed.
  RuleType& rule;

  //! The number of nodes which have been prenud during traversal.
  size_t numPrunes;
};

}; // namespace tree
}; // namespace mlpack

// Include implementation.
#include "rectangle_tree_traverser_impl.hpp"

#endif

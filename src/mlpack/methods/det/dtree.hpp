/**
 * @file dtree.hpp
 * @author Parikshit Ram (pram@cc.gatech.edu)
 *
 * Density Estimation Tree class
 */

#ifndef __MLPACK_METHODS_DET_DTREE_HPP
#define __MLPACK_METHODS_DET_DTREE_HPP

#include <assert.h>

#include <mlpack/core.hpp>

using namespace mlpack;
using namespace std;


namespace mlpack {
namespace det /** Density Estimation Trees */ {

/**
 * This two types in the template are used for two purposes:
 *
 *   eT - the type to store the data in (for most practical purposes, storing
 *       the data as a float suffices).
 *   cT - the type to perform computations in (computations like computing the
 *       error, the volume of the node etc.).
 *
 * For high dimensional data, it might be possible that the computation might
 * overflow, so you should use either normalize your data in the (-1, 1)
 * hypercube or use long double or modify this code to perform computations
 * using logarithms.
 *
 * A density estimation tree is similar to both a decision tree and a space
 * partitioning tree (like a kd-tree).  Each leaf represents a constant-density
 * hyper-rectangle.  The tree is constructed in such a way as to minimize the
 * integrated square error between the probability distribution of the tree and
 * the observed probability distribution of the data.  Because the tree is
 * similar to a decision tree, the density estimation tree can provide very fast
 * density estimates for a given point.
 *
 * For more information, see the following paper:
 *
 * @code
 * @incollection{ram2011,
 *   author = {Ram, Parikshit and Gray, Alexander G.},
 *   title = {Density estimation trees},
 *   booktitle = {{Proceedings of the 17th ACM SIGKDD International Conference
 *       on Knowledge Discovery and Data Mining}},
 *   series = {KDD '11},
 *   year = {2011},
 *   pages = {627--635}
 * }
 * @endcode
 */
template<typename eT = double,
         typename cT = long double>
class DTree
{
 private:
  // The indices in the complete set of points
  // (after all forms of swapping in the original data
  // matrix to align all the points in a node
  // consecutively in the matrix. The 'old_from_new' array
  // maps the points back to their original indices.
  size_t start;
  size_t end;

  // since we are using uniform density, we need
  // the max and min of every dimension for every node
  arma::vec maxVals;
  arma::vec minVals;

  // The split dim for this node
  size_t splitDim;

  // The split val on that dim
  double splitValue;

  // L2-error of the node
  double error;

  // sum of the error of the leaves of the subtree
  double subtreeLeavesError;

  // number of leaves of the subtree
  size_t subtreeLeaves;

  // flag to indicate if this is the root node
  // used to check whether the query point is
  // within the range
  bool root;

  // ratio of number of points in the node to the
  // total number of points (|t| / N)
  double ratio;

  // the inverse of  volume of the node
  double vTInv;

  // sum of the reciprocal of the inverse v_ts
  // the leaves of this subtree
  double subtreeLeavesVTInv;

  // the tag for the leaf used for hashing points
  int bucketTag;

  // The children
  DTree<eT, cT> *left;
  DTree<eT, cT> *right;

public:

  ////////////////////// Getters and Setters //////////////////////////////////
  size_t Start() const { return start; }

  size_t End() const { return end; }

  size_t SplitDim() const { return splitDim; }

  double SplitValue() const { return splitValue; }

  double Error() const { return error; }

  double SubtreeLeavesError() const { return subtreeLeavesError; }

  size_t SubtreeLeaves() const { return subtreeLeaves; }

  double Ratio() const { return ratio; }

  double VTInv() const { return vTInv; }

  double SubtreeLeavesVTInv() const { return subtreeLeavesVTInv; }

  DTree<eT, cT>* Left() const { return left; }
  DTree<eT, cT>* Right() const { return right; }

  bool Root() const { return root; }

  ////////////////////// Private Functions ////////////////////////////////////
 private:

  inline double LogNegativeError(const size_t total_points) const;

  bool FindSplit(const arma::mat& data,
                 size_t& splitDim,
                 double& splitValue,
                 double& leftError,
                 double& rightError,
                 const size_t maxLeafSize = 10,
                 const size_t minLeafSize = 5) const;

  size_t SplitData(arma::mat& data,
                   const size_t splitDim,
                   const double splitValue,
                   arma::Col<size_t>& oldFromNew) const;

  inline bool WithinRange(const arma::vec& query) const;

  ///////////////////// Public Functions //////////////////////////////////////
 public:

  DTree();

  // Root node initializer
  // with the bounding box of the data
  // it contains instead of just the data.
  DTree(const arma::vec& maxVals,
        const arma::vec& minVals,
        const size_t totalPoints);

  // Root node initializer
  // with the data, no bounding box.
  DTree(arma::mat& data);

  // Non-root node initializers
  DTree(const arma::vec& maxVals,
        const arma::vec& minVals,
        const size_t start,
        const size_t end,
        const double error);

  DTree(const arma::vec& maxVals,
        const arma::vec& minVals,
        const size_t totalPoints,
        const size_t start,
        const size_t end);

  ~DTree();

  // Greedily expand the tree
  double Grow(arma::mat& data,
              arma::Col<size_t>& oldFromNew,
              const bool useVolReg = false,
              const size_t maxLeafSize = 10,
              const size_t minLeafSize = 5);

  // perform alpha pruning on the tree
  double PruneAndUpdate(const double old_alpha, const bool useVolReg = false);

  // compute the density at a given point
  double ComputeValue(const arma::vec& query) const;

  // print the tree (in a DFS manner)
  void WriteTree(size_t level, FILE *fp);

  // indexing the buckets for possible usage later
  int TagTree(int tag);

  // This is used to generate the class membership
  // of a learned tree.
  int FindBucket(const arma::vec& query) const;

  // This computes the variable importance list
  // for the learned tree.
  void ComputeVariableImportance(arma::vec& importances) const;

}; // Class DTree

}; // namespace det
}; // namespace mlpack

#include "dtree_impl.hpp"

#endif // __MLPACK_METHODS_DET_DTREE_HPP

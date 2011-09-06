/**
 * @file discrete.h
 *
 * Discrete math helpers.
 */

#ifndef MATH_DISCRETE_H
#define MATH_DISCRETE_H

#include "../base/base.h"
#include "fastlib/fx/io.h"

#include <vector>

#include <math.h>

namespace math {
  /**
   * Computes the factorial of an integer.
   */
  __attribute__((const)) double Factorial(int d);
  
  /**
   * Computes the binomial coefficient, n choose k for nonnegative integers
   * n and k
   *
   * @param n the first nonnegative integer argument
   * @param k the second nonnegative integer argument
   * @return the binomial coefficient n choose k
   */
  __attribute__((const)) double BinomialCoefficient(int n, int k);

  /**
   * Creates an identity permutation where the element i equals i.
   *
   * Low-level pointer version -- preferably use the @c std::vector 
   * version instead.
   *
   * For instance, result[0] == 0, result[1] == 1, result[2] == 2, etc.
   *
   * @param size the number of elements in the permutation
   * @param array a place to store the permutation
   */
  void MakeIdentityPermutation(index_t size, index_t *array);
  
  /**
   * Creates an identity permutation where the element i equals i.
   *
   * For instance, result[0] == 0, result[1] == 1, result[2] == 2, etc.
   *
   * @param size the size to initialize the result to
   * @param result will be initialized to the identity permutation
   */
  void MakeIdentityPermutation(
      index_t size, std::vector<index_t>& result);
  
  /**
   * Creates a random permutation and stores it in an existing C array
   * (power user version).
   *
   * The random permutation is over the integers 0 through size - 1.
   *
   * @param size the number of elements
   * @param array the array to store a permutation in
   */
  void MakeRandomPermutation(index_t size, index_t* array);
  
  /**
   * Creates a random permutation over integers 0 throush size - 1.
   *
   * @param size the number of elements
   * @param result will be initialized to a permutation array
   */
  inline void MakeRandomPermutation(
      index_t size, std::vector<index_t>& result) {
    result.resize(size,0);
    MakeRandomPermutation(size, &result.front());
  }

  /**
   * Inverts or transposes an existing permutation.
   */
  void MakeInversePermutation(index_t size,
      const index_t* original, index_t* reverse);

  /**
   * Inverts or transposes an existing permutation.
   */
  inline void MakeInversePermutation(
      const std::vector<index_t>& original, std::vector<index_t>& reverse) {
    reverse.resize(original.size(),0);
    MakeInversePermutation(original.size(), &original.front(), &reverse.front());
  }
  
  template<typename TAnyIntegerType>
  inline bool IsPowerTwo(TAnyIntegerType i) {
    return (i & (i - 1)) == 0;
  }
  
  /**
   * Finds the log base 2 of an integer.
   *
   * This integer must absolutely be a power of 2.
   */
  inline unsigned IntLog2(unsigned i) {
    unsigned l;
    for (l = 0; (unsigned(1) << l) != i; l++) {
      mlpack::IO::AssertMessage(l < 1024, "Taking IntLog2 of a non-power-of-2");
    }
    return l;
  }
};

#endif

/**
 * @file farfield_expansion.h
 *
 * This file contains a templatized class implementing $O(D^p)$
 * expansion for computing the coefficients for a far-field expansion
 * for an arbitrary kernel function.
 *
 * @author Dongryeol Lee (dongryel)
 * @bug No known bugs.
 */

#ifndef FARFIELD_EXPANSION
#define FARFIELD_EXPANSION

#include <fastlib/fastlib.h>

#include "kernel_aux.h"
#include "series_expansion_aux.h"

template<typename TKernelAux> 
class LocalExpansion;

/** @brief Far field expansion class in \f$O(D^p)\f$ expansion.
 *
 *  \f$O(D^p)\f$ expansion for a kernel is a traditional expansion
 *  generated by the multivariate Taylor expansion.
 *
 *  @code
 *   //Declare a far-field expansion for the Gaussian kernel.
 *   FarFieldExpansion<GaussianKernelAux> fe;
 *
 *  @endcode
 */
template<typename TKernelAux>
class FarFieldExpansion {

 private:
  
  ////////// Private Member Variables //////////

  /** @brief The center of the expansion. */
  arma::vec center_;
  
  /** @brief The coefficients. */
  arma::vec coeffs_;

  /** @brief The order of the expansion. */
  int order_;
  
  /** @brief The pointer to the auxilirary methods for the kernel
   *         (derivative, truncation error bound).
   */
  const TKernelAux *ka_;

  /** @brief The pointer to the kernel object inside kernel auxiliary
   *         object.
   */
  const typename TKernelAux::TKernel *kernel_;

  /** @brief The pointer to the precomputed constants inside kernel
   *         auxiliary object 
   */
  const typename TKernelAux::TSeriesExpansionAux *sea_;

 public:
  
  ////////// Getters/Setters //////////
  
  /** @brief Gets the squared bandwidth value that is being used by
   *         the current far-field expansion object.
   *
   *  @return The squared bandwidth value.
   */
  double bandwidth_sq() const { return kernel_->bandwidth_sq(); }

  /** @brief Gets the center of expansion.
   *
   *  @return The center of expansion for the current far-field expansion.
   */
  arma::vec& get_center() { return center_; }
  const arma::vec& get_center() const { return center_; }

  /** @brief Gets the set of far-field coefficients.
   *
   *  @return The const reference to the vector containing the
   *          far-field coefficients.
   */
  arma::vec& get_coeffs() { return coeffs_; }
  const arma::vec& get_coeffs() const { return coeffs_; }
  
  /** @brief Gets the approximation order.
   *
   *  @return The integer representing the current approximation order.
   */
  int get_order() const { return order_; }
  
  /** @brief Gets the maximum possible approximation order.
   *
   *  @return The maximum allowable approximation order.
   */
  int get_max_order() const { return sea_->get_max_order(); }

  /** @brief Gets the weight sum.
   */
  double get_weight_sum() const { return coeffs_[0]; }

  /** @brief Sets the approximation order of the far-field expansion.
   *
   *  @param new_order The desired new order of the approximation.
   */
  void set_order(int new_order) { order_ = new_order; }
  
  /** @brief Set the center of the expansion - assumes that the center
   *         has been initialized before...
   *
   *  @param center The center of expansion whose coordinate values
   *                will be copied to the center of the given far-field 
   *                expansion object.
   */
  void set_center(const arma::vec& center) {
    center_ = center;
  }

  ////////// User-level Functions //////////
  
  /** @brief Accumulates the contribution of a single reference point as a 
   *         far-field moment.
   *
   *  For the reference point $r$ and the current expansion centroid
   *  in the reference node \f$R\f$ as \f$R.c\f$, this function
   *  computes \f$\left( \frac{r - R.c}{kh} \right)^{\alpha}\f$ for
   *  \f$0 \leq \alpha \leq p\f$ where \f$\alpha\f$ is a
   *  \f$D\f$-dimensional multi-index and adds to the currently
   *  accumulated far-field moments: \f$ F(R) \leftarrow F(R) + w [
   *  \left( \frac{r - R.c}{kh} \right)^{\alpha}]_{0 \leq p \leq
   *  \alpha}\f$ where \f$F(R)\f$ is the current set of far-field
   *  moments for the reference node \f$R\f$. \f$k\f$ is an
   *  appropriate factor to multiply the bandwidth \f$h\f$ by; for the
   *  Gaussian kernel expansion, it is \f$\sqrt{2}\f$.
   *
   *  @param reference_point The coordinates of the reference point.
   *  @param weight The weight of the reference point v.
   *  @param order The order up to which the far-field moments should be 
   *               accumulated up to.
   */
  void Accumulate(const arma::vec& reference_point, double weight, int order);

  /** @brief Accumulates the far field moment represented by the given
   *         reference data into the coefficients.
   *
   *  Given the set of reference points \f$r_{j_n} \in R\f$ in the
   *  reference node \f$R\f$, this function computes
   *  \f$\sum\limits_{r_{j_n} \in R} w_{j_n} \left( \frac{r_{j_n} -
   *  R.c}{kh} \right)^{\alpha}\f$ for \f$0 \leq \alpha \leq p\f$
   *  where \f$\alpha\f$ is a \f$D\f$-dimensional multi-index and adds
   *  to the currently accumulated far-field moments: \f$ F(R)
   *  \leftarrow F(R) + \left[ \sum\limits_{r_{j_n} \in R} w_{j_n}
   *  \left( \frac{r_{j_n} - R.c}{kh} \right)^{\alpha} \right]_{0 \leq
   *  p \leq \alpha}\f$ where \f$F(R)\f$ is the current set of
   *  far-field moments for the reference node \f$R\f$. \f$k\f$ is an
   *  appropriate factor to multiply the bandwidth \f$h\f$ by; for the
   *  Gaussian kernel expansion, it is \f$\sqrt{2}\f$.
   *
   *  @param data The entire reference dataset \f$\mathcal{R}\f$.
   *  @param weights The entire reference weights \f$\mathcal{W}\f$.
   *  @param begin The beginning index of the reference points for
   *               which we would like to accumulate the moments for.
   *  @param end The upper limit on the index of the reference points for
   *             which we would like to accumulate the moments for.
   *  @param order The order up to which the far-field moments should be
   *               accumulated up to.
   */
  void AccumulateCoeffs(const arma::mat& data, const arma::vec& weights,
			int begin, int end, int order);

  /** @brief Refine the far field moment that has been computed before
   *         up to a new order.
   */
  void RefineCoeffs(const arma::mat& data, const arma::vec& weights,
		    int begin, int end, int order);
  
  /** @brief Evaluates the far-field coefficients at the given point.
   */
  double EvaluateField(const arma::mat& data, int row_num, int order) const;
  double EvaluateField(const double *x_q, int order) const;

  /** @brief Evaluates the two-way convolution mixed with exhaustive
   *         computations with two other far field expansions.
   */
  double MixField(const arma::mat& data, int node1_begin, int node1_end, 
		  int node2_begin, int node2_end, const FarFieldExpansion &fe2,
		  const FarFieldExpansion &fe3, int order2, int order3) const;

  /** @brief Evaluates the convolution with the other farfield
   *         expansion.
   */
  double ConvolveField(const FarFieldExpansion &fe, int order) const;

  /** @brief Evaluates the three-way convolution with two other far
   *         field expansions.
   */
  double ConvolveField(const FarFieldExpansion &fe2,
		       const FarFieldExpansion &fe3,
		       int order1, int order2, int order3) const;

  /** @brief Initializes the current far field expansion object with
   *         the given center.
   */
  void Init(const arma::vec& center, const TKernelAux &ka);
  void Init(const TKernelAux &ka);

  template<typename TBound>
  int OrderForConvolving(const TBound &far_field_region,
			 const arma::vec &far_field_region_centroid,
			 const TBound &local_field_region,
			 const arma::vec &local_field_region_centroid,
			 double min_dist_sqd_regions,
			 double max_dist_dsqd_regons,
			 double max_error, double *actual_error) const;

  /** @brief Computes the required order for evaluating the far field
   *         expansion for any query point within the specified region
   *         for a given bound.
   */
  template<typename TBound>
  int OrderForEvaluating(const TBound &far_field_region,
			 const TBound &local_field_region,
			 double min_dist_sqd_regions,
			 double max_dist_sqd_regions,
			 double max_error, double *actual_error) const;

  /** @brief Computes the required order for converting to the local
   *         expansion inside another region, so that the total error
   *         (truncation error of the far field expansion plus the
   *         conversion error) is bounded above by the given user
   *         bound.
   *
   *  @return the minimum approximation order required for the error,
   *          -1 if approximation up to the maximum order is not possible.
   */
  template<typename TBound>
  int OrderForConvertingToLocal(const TBound &far_field_region,
				const TBound &local_field_region, 
				double min_dist_sqd_regions, 
				double max_dist_sqd_regions,
				double required_bound, 
				double *actual_error) const;

  /** @brief Prints out the series expansion represented by this object.
   */
  void PrintDebug(const char *name="", FILE *stream=stderr) const;

  /** @brief Translate from a far field expansion to the expansion
   *         here. The translated coefficients are added up to the
   *         ones here.
   */
  void TranslateFromFarField(const FarFieldExpansion &se);
  
  /** @brief Translate to the given local expansion. The translated
   *         coefficients are added up to the passed-in local
   *         expansion coefficients.
   */
  void TranslateToLocal(LocalExpansion<TKernelAux> &se, int truncation_order);

};

#define INSIDE_FARFIELD_EXPANSION_H
#include "farfield_expansion_impl.h"
#undef INSIDE_FARFIELD_EXPANSION_H

#endif

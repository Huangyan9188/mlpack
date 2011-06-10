/** @file original_ifgt.h
 *
 *  Revision: 0.99
 *  Date	: Mon Jul  8 18:11:31 EDT 2002
 *
 *  Revision: 1.00
 *  Date : Wed Jan 29 16:05:21 EDT 2003
 *
 *  Revision: 1.01
 *  Date : Wed Apr 14 09:51:26 EDT 2004
 *
 *  This implements the KDE using the original improved fast Gauss
 *  transform algorithm. For more details, take a look at:
 *
 *  inproceedings{946593, author = {Changjiang Yang and Ramani
 *  Duraiswami and Nail A. Gumerov and Larry Davis}, title = {Improved
 *  Fast Gauss Transform and Efficient Kernel Density Estimation},
 *  booktitle = {ICCV '03: Proceedings of the Ninth IEEE International
 *  Conference on Computer Vision}, year = {2003}, isbn =
 *  {0-7695-1950-4}, pages = {464}, publisher = {IEEE Computer
 *  Society}, address = {Washington, DC, USA}, }
 *
 *  LICENSE for FIGTREE V1.0 Copyright (c) 2002-2004, University of
 *  Maryland, College Park. All rights reserved.  The University of
 *  Maryland grants Licensee permission to use, copy, modify, and
 *  distribute FIGTREE ("Software) for any non-commercial, academic
 *  purpose without fee, subject to the following conditions: 1. Any
 *  copy or modification of this Software must include the above
 *  copyright notice and this license.  2. THE SOFTWARE AND ANY
 *  DOCUMENTATION ARE PROVIDED "AS IS"; THE UNIVERSITY OF MARYLAND
 *  MAKES NO WARRANTY OR REPRESENTATION THAT THE SOFTWARE WILL BE
 *  ERROR-FREE.  THE UNIVERSITY OF MARYLAND DISCLAIMS ANY AND ALL
 *  WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 *  PARTICULAR PURPOSE, or NONINFRINGEMENT.  THIS DISCLAIMER OF
 *  WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS LICENSE.  IN NO
 *  EVENT WILL THE STATE OF MARYLAND, THE UNIVERSITY OF MARYLAND OR
 *  ANY OF THEIR RESPECTIVE OFFCERS, AGENTS, EMPLOYEES OR STUDENTS BE
 *  LIABLE FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL DAMAGES OR
 *  ANY KIND, INCLUDING, BUT NOT LIMITED TO, LOSS OF BUSINESS, WORK
 *  STOPPAGE, COMPUTER FAILURE OR MALFUNCTION, OR ANY AND ALL OTHER
 *  COMMERCIAL DAMAGES OR LOSSES, EVEN IF THE UNIVERSITY OF MARYLAND
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  3. The
 *  University of Maryland has no obligation to support, upgrade, or
 *  maintain the Software.  4. Licensee shall not use the name
 *  University of Maryland or any adaptation of the University of
 *  Maryland or any of its service or trade marks for any commercial
 *  purpose, including, but not limited to, publicity and advertising,
 *  without prior written consent of the University of Maryland.
 *  5. Licensee is responsible for complying with United States export
 *  control laws, including the Export Administration Regulations and
 *  International Traffic in Arms Regulations, and with United States
 *  boycott laws.  In the event a license is required under export
 *  control laws and regulations as a condition of exporting this
 *  Software outside the United States or to a foreign national in the
 *  United State, Licensee will be responsible for obtaining said
 *  license.  6. This License will terminate automatically upon
 *  Licensee's failure to correct its breach of any term of this
 *  License within thirty (30) days of receiving notice of its breach.
 *  7. Report bugs to: Changjiang Yang (yangcj@umiacs.umd.edu) and
 *  Ramani Duraiswami (ramani@umiacs.umd.edu), Department of Computer
 *  Science, University of Maryland, College Park.
 *
 *  Submit a request for a commercial license to: Jim Poulos, III
 *  Office of Technology Commercialization University of Maryland 6200
 *  Baltimore Avenue, Suite 300 Riverdale, Maryland 20737-1054
 *  301-403-2711 tel 301-403-2717 fax
 *
 *  @author Changjiang Yang (modified by Dongryeol Lee for FastLib usage)
 */

#ifndef ORIGINAL_IFGT_H
#define ORIGINAL_IFGT_H

#include <fastlib/fastlib.h>

class OriginalIFGT {

 private:

  ////////// Private Member Variables //////////

  /** @brief Dimensionality of the points. */
  index_t dim_;

  /** @brief The number of reference points. */
  index_t num_reference_points_;

  /** @brief The column-oriented query dataset. */
  arma::mat query_set_;

  /** @brief The column-oriented reference dataset. */
  arma::mat reference_set_;

  /** @brief The weights associated with each reference point. */
  arma::vec reference_weights_;

  /** @brief The bandwidth */
  double bandwidth_;

  /** @brief Squared bandwidth */
  double bandwidth_sq_;

  /** @brief Bandwidth multiplied by \f$\sqrt{2}\f$ for modifying the
   *         original code to do computation for the Gaussian kernel
   *         \f$e^{-\frac{x^2}{2h^2}} \f$, rather than the alternative
   *         Gaussian kernel \f$e^{-\frac{x^2}{h^2}}\f$ used by the
   *         original code.
   */
  double bandwidth_factor_;

  /** @brief The desired absolute error precision. */
  double epsilon_;

  /** @brief The truncation order. */
  index_t pterms_;

  /** @brief The total number of coefficients */
  index_t total_num_coeffs_;
  
  /** @brief The coefficients weighted by reference_weights_ */
  arma::mat weighted_coeffs_;

  /** @brief The unweighted coefficients */
  arma::mat unweighted_coeffs_;

  /** @brief The number of clusters desired for preprocessing */
  index_t num_cluster_desired_;

  /** @brief If the distance between a query point and the cluster
   *         centroid is more than this quanity times the bandwidth,
   *         then the kernel sum contribution of the cluster is
   *         presumed to be zero.
   */
  double cut_off_radius_;

  /** @brief The maximum radius of the cluster among those generated.
   */
  double max_radius_cluster_;

  /** @brief The set of cluster centers
   */
  arma::mat cluster_centers_;

  /** @brief The reference point index that is being used for the
   *         center of the clusters during K-center algorithm.
   */
  std::vector<index_t> index_during_clustering_;

  /** @brief The i-th position of this vector tells the cluster number
   *         to which the i-th reference point belongs.
   */
  std::vector<index_t> cluster_index_;

  /** @brief The i-th position of this vector tells the radius of the i-th
   *         cluster.
   */
  arma::vec cluster_radii_;

  /** @brief The number of reference points owned by each cluster.
   */
  std::vector<index_t> num_reference_points_in_cluster_;

  /** @brief This will hold the final computed densities.
   */
  arma::vec densities_;

  ////////// Private Member Functions //////////
  void TaylorExpansion() {
      
    arma::vec tmp_coeffs(total_num_coeffs_);
    
    ComputeUnweightedCoeffs_(tmp_coeffs);
    
    ComputeWeightedCoeffs_(tmp_coeffs);
    
    return;	
  }

  void ComputeUnweightedCoeffs_(arma::vec& taylor_coeffs) {
	
    std::vector<index_t> heads;
    heads.reserve(dim_ + 1);
    std::vector<index_t> cinds;
    cinds.reserve(total_num_coeffs_);
    
    for (index_t i = 0; i < dim_; i++) {
      heads[i] = 0;
    }
    heads[dim_] = INT_MAX;
    
    cinds[0] = 0;
    taylor_coeffs[0] = 1.0;
    
    for(index_t k = 1, t = 1, tail = 1; k < pterms_; k++, tail = t) {
      for(index_t i = 0; i < dim_; i++) {
	index_t head = heads[i];
	heads[i] = t;
	for(index_t j = head; j < tail; j++, t++) {
	  cinds[t] = (j < heads[i+1]) ? cinds[j] + 1 : 1;
	  taylor_coeffs[t] = 2.0 * taylor_coeffs[j];
	  taylor_coeffs[t] /= (double) cinds[t];
	}
      }
    }
    return;
  }

  void ComputeWeightedCoeffs_(arma::vec& taylor_coeffs) {
    
    arma::vec dx(dim_);
    arma::vec prods(total_num_coeffs_);
    std::vector<index_t> heads;
    heads.reserve(dim_);
    
    // initialize coefficients for all clusters to be zero.
    weighted_coeffs_.zeros();
    unweighted_coeffs_.zeros();
    
    for(index_t n = 0; n < num_reference_points_; n++) {
      
      index_t ix2c = cluster_index_[n];
      double sum = 0.0;
      
      for(index_t i = 0; i < dim_; i++) {
	dx[i] = (reference_set_(i, n) - cluster_centers_(i, ix2c)) / 
	  bandwidth_factor_;

	sum -= dx[i] * dx[i];
	heads[i] = 0;
      }
      
      prods[0] = exp(sum);
      for(index_t k = 1, t = 1, tail = 1; k < pterms_; k++, tail = t) {
	
	for (index_t i = 0; i < dim_; i++) {
	  index_t head = heads[i];
	  heads[i] = t;
	  for(index_t j = head; j < tail; j++, t++)
	    prods[t] = dx[i] * prods[j];
	} // for i
      } // for k
      
      // compute the weighted coefficients and unweighted coefficients.
      for(index_t i = 0; i < total_num_coeffs_; i++) {
	weighted_coeffs_(i, ix2c) += reference_weights_[n] * prods[i];
	unweighted_coeffs_(i, ix2c) += prods[i];
      }
      
    }// for n
    
    // normalize by the Taylor coefficients.
    for(index_t k = 0; k < num_cluster_desired_; k++) {
      for(index_t i = 0; i < total_num_coeffs_; i++) {
	weighted_coeffs_(i, k) *= taylor_coeffs[i];
	unweighted_coeffs_(i, k) *= taylor_coeffs[i];
      }
    }
    return;
  }

  /** @brief Compute the center and the radius of each cluster.
   *
   *  @return The maximum radius of the cluster among generated clusters.
   */
  double ComputeCenters() {

    // set cluster max radius to zero
    max_radius_cluster_ = 0;

    // clear all centers.
    cluster_centers_.zeros();
    
    // Compute the weighted centroid for each cluster.
    for(index_t j = 0; j < dim_; j++) {
      for(index_t i = 0; i < num_reference_points_; i++) {
	cluster_centers_(j, cluster_index_[i]) += reference_set_(j, i);
      }
    }
    
    for(index_t j = 0; j < dim_; j++) {
      for(index_t i = 0; i < num_cluster_desired_; i++) {
	cluster_centers_(j, i) /= num_reference_points_in_cluster_[i];
      }
    }
    
    // Now loop through and compute the radius of each cluster.
    cluster_radii_.zeros();
    for(index_t i = 0; i < num_reference_points_; i++) {
      arma::vec reference_pt = reference_set_.col(i);

      // the index of the cluster this reference point belongs to.
      index_t cluster_id = cluster_index_[i];
      arma::vec center = cluster_centers_.col(cluster_id);
      cluster_radii_[cluster_id] = 
	std::max(cluster_radii_[cluster_id], 
		 sqrt(la::DistanceSqEuclidean(reference_pt, center)));
      max_radius_cluster_ =
	std::max(max_radius_cluster_, cluster_radii_[cluster_id]);
    }

    return max_radius_cluster_;
  }

  /** @brief Perform the farthest point clustering algorithm on the 
   *         reference set.
   */
  double KCenterClustering() {
    
    arma::vec distances_to_center(num_reference_points_);
    
    // randomly pick one node as the first center.
    srand((unsigned) time(NULL));
    index_t ind = rand() % num_reference_points_;
    
    // add the ind-th node to the first center.
    index_during_clustering_[0] = ind;
    arma::vec first_center = reference_set_.col(ind);
    
    // compute the distances from each node to the first center and
    // initialize the index of the cluster ID to zero for all
    // reference points.
    for(index_t j = 0; j < num_reference_points_; j++) {
      arma::vec reference_point = reference_set_.col(j);
      
      distances_to_center[j] = (j == ind) ? 
	0.0 : la::DistanceSqEuclidean(reference_point, first_center);
      cluster_index_[j] = 0;
    }
    
    // repeat until the desired number of clusters is reached.
    for(index_t i = 1; i < num_cluster_desired_; i++) {
      
      // Find the reference point that is farthest away from the
      // current center.
      ind = IndexOfLargestElement(distances_to_center);
      
      // Add the ind-th node to the centroid list.
      index_during_clustering_[i] = ind;
      
      // Update the distances from each point to the current center.
      arma::vec center = reference_set_.col(ind);
      
      for (index_t j = 0; j < num_reference_points_; j++) {
	arma::vec reference_point = reference_set_.col(j);
	double d = (j == ind)? 
	  0.0 : la::DistanceSqEuclidean(reference_point, center);
	
	if (d < distances_to_center[j]) {
	  distances_to_center[j] = d;
	  cluster_index_[j] = i;
	}
      }
    }
    
    // Find the maximum radius of the k-center algorithm.
    ind = IndexOfLargestElement(distances_to_center);
    
    double radius = distances_to_center[ind];
    
    
    for(index_t i = 0; i < num_cluster_desired_; i++) {
      num_reference_points_in_cluster_[i] = 0;
    }
    // tally up the number of reference points for each cluster.
    for (index_t i = 0; i < num_reference_points_; i++) {
      num_reference_points_in_cluster_[cluster_index_[i]]++;
    }
    
    return sqrt(radius);  
  }

  /** @brief Return the index whose position in the vector contains
   *         the largest element.
   */
  index_t IndexOfLargestElement(const arma::vec& x) {
    
    index_t largest_index = 0;
    double largest_quantity = -DBL_MAX;
    
    for(index_t i = 0; i < x.n_elem; i++) {
      if(largest_quantity < x[i]) {
	largest_quantity = x[i];
	largest_index = i;
      }
    }
    return largest_index;
  }

  /** 
   * Normalize the density estimates after the unnormalized sums have
   * been computed 
   */
  void NormalizeDensities() {
    double norm_const = pow(2 * math::PI * bandwidth_sq_, 
			    query_set_.n_rows / 2.0) *
                            reference_set_.n_cols;

    for(index_t q = 0; q < query_set_.n_cols; q++) {
      densities_[q] /= norm_const;
    }
  }

  void IFGTChooseTruncationNumber_() {	

    double rx = max_radius_cluster_;
    double max_diameter_of_the_datasets = sqrt(dim_);
    
    double two_h_square = bandwidth_factor_ * bandwidth_factor_;
    
    double r = min(max_diameter_of_the_datasets, 
		   bandwidth_factor_ * sqrt(log(1 / epsilon_)));
    
    index_t p_ul = 300;
    
    double rx_square = rx * rx;
    
    double error = 1;
    double temp = 1;
    index_t p = 0;
    while((error > epsilon_) & (p <= p_ul)) {
      p++;
      double b = min(((rx + sqrt((rx_square) + (2 * p * two_h_square))) / 2),
		     rx + r);
      double c = rx - b;
      temp = temp * (((2 * rx * b) / two_h_square) / p);
      error = temp * (exp(-(c * c) / two_h_square));			
    }	
    
    // update the truncation order.
    pterms_ = p;

    // update the cut-off radius
    cut_off_radius_ = r;
    
  }
  
  void IFGTChooseParameters_(index_t max_num_clusters) {
    
    // for references and queries that fit in the unit hypercube, this
    // assumption is true, but for general case it is not.
    double max_diamater_of_the_datasets = sqrt(dim_);
    
    double two_h_square = bandwidth_factor_ * bandwidth_factor_;

    // The cut-off radius.
    double r = min(max_diamater_of_the_datasets, 
		   bandwidth_factor_ * sqrt(log(1 / epsilon_)));
    
    // Upper limit on the truncation number.
    index_t p_ul = 200; 
    
    num_cluster_desired_ = 1;
    
    double complexity_min = 1e16;
    double rx;

    for(index_t i = 0; i < max_num_clusters; i++){
     
      // Compute an estimate of the maximum cluster radius.
      rx = pow((double) i + 1, -1.0 / (double) dim_);
      double rx_square = rx * rx;

      // An estimate of the number of neighbors.
      double n = std::min(i + 1.0, pow(r / rx, (double) dim_));
      double error = 1;
      double temp = 1;
      index_t p = 0;

      // Choose the truncation order.
      while((error > epsilon_) & (p <= p_ul)) {
	p++;
	double b = 
	  std::min(((rx + sqrt((rx_square) + (2 * p * two_h_square))) / 2.0),
		   rx + r);
	double c = rx - b;
	temp = temp * (((2 * rx * b) / two_h_square) / p);
	error = temp * (exp(-(c * c) / two_h_square));
      }
      double complexity = (i + 1) + log((double) i + 1) + 
	((1 + n) * math::BinomialCoefficient(p - 1 + dim_, dim_));
	
      if(complexity < complexity_min) {
	complexity_min = complexity;
	num_cluster_desired_ = i + 1;
	pterms_ = p;
      }
    }    
  }

 public:

  ////////// Constructor/Destructor //////////

  /** @brief Constructor
   */
  OriginalIFGT() {}

  /** @brief Destructor
   */
  ~OriginalIFGT() {}

  
  ////////// Getters/Setters //////////

  /** @brief Get the density estimates.
   *
   *  @param results An uninitialized vector which will be initialized
   *                 with the computed density estimates.
   */
  void get_density_estimates(arma::vec& results) {
    results = densities_;
  }

  ////////// User-leverl Functions //////////

  void Init(arma::mat& queries, arma::mat& references) {

    // set dimensionality
    dim_ = references.n_rows;
        
    // set up query set and reference set.
    query_set_ = queries;
    reference_set_ = references;
    num_reference_points_ = reference_set_.n_cols;
    
    // By default, the we do uniform weights only
    reference_weights_.ones(reference_set_.n_cols);

    // initialize density estimate vector
    densities_.set_size(query_set_.n_cols);
    
    // A "hack" such that the code uses the proper Gaussian kernel.
    bandwidth_ = mlpack::IO::GetParam<double>("kde/bandwidth");
    bandwidth_sq_ = bandwidth_ * bandwidth_;
    bandwidth_factor_ = sqrt(2) * bandwidth_;
    
    // Read in the desired absolute error accuracy.
    epsilon_ = mlpack::IO::GetParam<double>("kde/absolute_error"); //Default value .1

    // This is the upper limit on the number of clusters.
    index_t cluster_limit = (index_t) ceilf(20.0 * sqrt(dim_) / sqrt(bandwidth_));
    
    mlpack::IO::Debug << "Automatic parameter selection phase..." << std::endl;

    mlpack::IO::Info << "Preprocessing phase for the original IFGT..." << std::endl;

    mlpack::IO::StartTimer("kde/ifgt_kde_preprocess");
    IFGTChooseParameters_(cluster_limit);
    mlpack::IO::Debug << "Chose " << num_cluster_desired_ << " clusters..." << std::endl;
    mlpack::IO::Debug << "Tentatively chose " << pterms_ << " truncation order..." << std::endl;

    // Allocate spaces for storing coefficients and clustering information.
    cluster_centers_.set_size(dim_, num_cluster_desired_);
    index_during_clustering_.reserve(num_cluster_desired_);
    cluster_index_.reserve(num_reference_points_);
    cluster_radii_.set_size(num_cluster_desired_);
    num_reference_points_in_cluster_.reserve(num_cluster_desired_);    
    
    mlpack::IO::Debug << "Now clustering..." << std::endl;

    // Divide the source space into num_cluster_desired_ parts using
    // K-center algorithm
    max_radius_cluster_ = KCenterClustering();
    
    // computer the center of the sources
    ComputeCenters();

    // Readjust the truncation order based on the actual clustering result.
    IFGTChooseTruncationNumber_();
    // pd = C_dim^(dim+pterms-1)
    total_num_coeffs_ = 
      (index_t) math::BinomialCoefficient(pterms_ + dim_ - 1, dim_);
    weighted_coeffs_.set_size(total_num_coeffs_, num_cluster_desired_);
    unweighted_coeffs_.set_size(total_num_coeffs_, num_cluster_desired_);

    mlpack::IO::Debug << "Maximum radius generated in the cluster: " 
      << max_radius_cluster_ << "..." << std::endl,
		
    mlpack::IO::Debug << "Truncation order updated to " 
      << pterms_ << " after clustering..." << std::endl;

    // Compute coefficients.    
    mlpack::IO::Debug << "Now computing Taylor coefficients..." << std::endl;
    TaylorExpansion();
    mlpack::IO::Debug << "Taylor coefficient computation finished..." << std::endl;
    mlpack::IO::StopTimer("kde/ifgt_kde_preprocess");
    mlpack::IO::Info << "Preprocessing step finished..." << std::endl;
  }

  void Compute() {
    
    mlpack::IO::Info << "Starting the original IFGT-based KDE computation..."
      << std::endl;

    mlpack::IO::StartTimer("kde/original_ifgt_kde_compute");

    arma::vec dy(dim_);
    arma::vec tempy(dim_);
    arma::vec prods(total_num_coeffs_);
    
    std::vector<index_t> heads;
    heads.reserve(dim_);
    
    // make sure the sum for each query point starts at zero.
    densities_.zeros();
    
    for(index_t m = 0; m < query_set_.n_cols; m++) {
      
      // loop over each cluster and evaluate Taylor expansions.
      for(index_t kn = 0; kn < num_cluster_desired_; kn++) {
	
	double sum2 = 0.0;
	
	// compute the ratio of the squared distance between each query
	// point and each cluster center to the bandwidth factor.
	for (index_t i = 0; i < dim_; i++) {
	  dy[i] = (query_set_(i, m) - cluster_centers_(i, kn)) / 
              bandwidth_factor_;
	  sum2 += dy[i] * dy[i];
	}
	
	// If the ratio is greater than the cut-off, this cluster's
	// contribution is ignored.
	if (sum2 > (cut_off_radius_ + cluster_radii_[kn]) /
	    (bandwidth_factor_ * bandwidth_factor_)) {
	  continue;
	}
	
	for(index_t i = 0; i < dim_; i++) {
	  heads[i] = 0;
	}
	
	prods[0] = exp(-sum2);		
	for(index_t k = 1, t = 1, tail = 1; k < pterms_; k++, tail = t) {
	  for (index_t i = 0; i < dim_; i++) {
	    index_t head = heads[i];
	    heads[i] = t;
	    for(index_t j = head; j < tail; j++, t++)
	      prods[t] = dy[i] * prods[j];
	  } // for i
	}// for k
	
	for(index_t i = 0; i < total_num_coeffs_; i++) {
	  densities_[m] += weighted_coeffs_(i, kn) * prods[i];
	}
	
      } // for each cluster
    } //for each query point

    // normalize density estimates
    NormalizeDensities();

    mlpack::IO::StopTimer("kde/original_ifgt_kde_compute");
    mlpack::IO::Info << "Computation finished..." << std::endl;
  }

  void PrintDebug() {
    
    FILE *stream = stdout;
    const char *fname = NULL;
    if(mlpack::IO::HasParam("kde/ifgt_kde_output"))
      fname = mlpack::IO::GetParam<std::string>("kde/ifgt_kde_output").c_str();

    if(fname != NULL) {
      stream = fopen(fname, "w+");
    }
    for(index_t q = 0; q < query_set_.n_cols; q++) {
      fprintf(stream, "%g\n", densities_[q]);
    }
    
    if(stream != stdout) {
      fclose(stream);
    }
  }

};

#endif

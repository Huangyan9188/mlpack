#include "fastlib/fastlib.h"
#include "mlpack/svm/svm.h"
#include "utils_lkm.h"
#define SVM_RBF_KERNEL 1
#define SVM_LINEAR_KERNEL 2

#define GAUSSIAN_SMOOTHING_KERNEL 3
#define EPAN_SMOOTHING_KERNEL 4

#ifndef LOCAL_KERNEL_MACHINES_DEF_H_
#define LOCAL_KERNEL_MACHINES_DEF_H_




template<typename TKernel>
class LocalKernelMachines{
  
  
 private:
  
  // The essentials
  Matrix train_data_;
  
  
  Matrix test_data_;

  Vector train_labels_vector_;

  index_t num_train_points_;

  index_t num_test_points_;

  index_t num_dims_;

  struct datanode *module_;

  // A dataset object. This will be useful for crossvalidation

  Dataset dset_;

  // Useful for crossvalidation
  ArrayList <index_t> random_permutation_array_list_;
 

  // The regularization constant

  double optimal_lambda_;

  // Bandwidth of the svm kernel (if used);

  double optimal_svm_kernel_bandwidth_;

  // Optimal bandwidth of the smoothing kernel used
  double optimal_smoothing_kernel_bandwidth_;
  
  // Some auxiliary variables

  // The different kernels used

  SVMRBFKernel svm_rbf_;
  SVMLinearKernel svm_linear_;
  EpanKernel epan_kernel_;

  // Indication of the kernels being used

  index_t smoothing_kernel_;
  index_t svm_kernel_;

  // Indication of what parameters to cv for

  index_t cv_lambda_flag_;
  index_t cv_svm_kernel_bandwidth_flag_;
  index_t cv_smoothing_kernel_bandwidth_flag_;


  // Number of folds of crossvalidation

  index_t k_folds_;
  
  void  SolveLocalSVMProblem_(Matrix &local_train_data, Matrix &local_test_data, 
			      Vector &local_train_data_labels, Vector local_test_data_labels);

  void GetTheFold_(Dataset &cv_train_data,Dataset &cv_test_data,
		   Vector &cv_train_labels, Vector &cv_test_labels,index_t fold_num);
  
  void CrossValidateOverAllThreeParameters_();

  void  CrossValidateOverSVMKernelBandwidthAndSmoothingKernelBandwidth_();

  void  CrossValidateOverSVMKernelBandwidthAndLambda_();
 
 
  void  GenerateSmoothingBandwidthVectorForCV_();
 
  void  GenerateLambdaVectorForCV_();
 
  void  CrossValidateOverSmoothingKernelBandwidthAndLambda_();
 
  void  CrossValidateOverSmoothingKernelBandwidth_();
 
 
  void  CrossValidateOverSVMKernelBandwidth_();
 
  void  CrossValidateOverLambda_();
 
  void PerformCrossValidation_();
 
 
  void SetCrossValidationFlags_();
 
  void PrepareForCrossValidation_();
  void CrossValidation_();
 public:
 
  void TrainLocalKernelMachines();
 
 void Init(Matrix &train_data,Matrix &test_data,Vector &train_labels_vector,
	   struct datanode *module_in);
};
#include "local_kernel_machines_impl.h"
#include "my_crossvalidation.h"
#endif


/*
 * =====================================================================================
 *
 *       Filename:  kernel_pca_test.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/09/2008 11:26:48 AM EST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 *
 * =====================================================================================
 */

#include "kernel_pca.h"
#include <vector>
#include <fastlib/fastlib.h>
#include <fastlib/base/test.h>
#include <fastlib/base/common.h>

class KernelPCATest {
 public:    
  void Init() {
    engine_ = new KernelPCA();
    engine_->Init("test_data_3_1000.csv", 5, 20);
  }
  void Destruct() {
    delete engine_;
  }
  void TestGeneralKernelPCA() {
   IO::Info << "Testing KernelPCA ..." << std::endl;
   arma::mat eigen_vectors;
   arma::vec eigen_values;
   Init();
   engine_->ComputeNeighborhoods();
   double bandwidth;
   engine_->EstimateBandwidth(&bandwidth);
   IO::Info << "Estimated Bandwidth " << bandwidth << "..." << std::endl;
   kernel_.set(bandwidth); 
   engine_->LoadAffinityMatrix();
   engine_->ComputeGeneralKernelPCA(kernel_, 15, 
                                    &eigen_vectors,
                                    &eigen_values);

   engine_->SaveToTextFile("results", eigen_vectors, eigen_values);
   Destruct();
   IO::Info << "Test ComputeGeneralKernelPCA passed...!" << std::endl;
  }
  void TestLLE() {
   IO::Info << "Testing ComputeLLE" << std::endl;
    arma::mat eigen_vectors;
    arma::vec eigen_values;
    Init();
    engine_->ComputeNeighborhoods();
    engine_->LoadAffinityMatrix();
    engine_->ComputeLLE(2,
                         &eigen_vectors,
                         &eigen_values);
    engine_->SaveToTextFile("results", eigen_vectors, eigen_values);
    Destruct();
    IO::Info << "Test ComputeLLE passed...!" << std::endl; 
  }
  void TestSpectralRegression() {
    IO::Info << "Test ComputeSpectralRegression..." << std::endl;
    Init();
    engine_->ComputeNeighborhoods();
    double bandwidth;
    engine_->EstimateBandwidth(&bandwidth);
    IO::Info << "Estimated bandwidth " << bandwidth << " ..." << std::endl;
    kernel_.set(bandwidth); 
    engine_->LoadAffinityMatrix();
    std::map<index_t, index_t> data_label;
    for(index_t i=0; i<20; i++) {
      data_label[math::RandInt(0, engine_->data_.n_cols())] = 
        math::RandInt(0 ,2);
    }
    arma::mat embedded_coordinates;
    arma::vec eigenvalues; 
    engine_->ComputeSpectralRegression(kernel_,
                                       data_label,
                                       &embedded_coordinates, 
                                       &eigenvalues);
    engine_->SaveToTextFile("results", embedded_coordinates, eigenvalues);
    Destruct();

    IO::Info << "Test ComputeSpectralRegression passed..." << std::endl;
  }
  void TestAll() {
     //TestGeneralKernelPCA();
     TestLLE();
     //TestSpectralRegression();
  }
 private:
  KernelPCA *engine_;
  KernelPCA::GaussianKernel kernel_;
};

int main() {
  KernelPCATest test;
  test.TestAll();
}

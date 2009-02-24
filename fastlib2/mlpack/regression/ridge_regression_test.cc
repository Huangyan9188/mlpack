/*
 * =====================================================================================
 *
 *       Filename:  ridge_regression_test.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/15/2009 01:34:25 PM EST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 *
 * =====================================================================================
 */
#include "fastlib/fastlib.h"
#include "fastlib/base/test.h"
#include "ridge_regression.h"
#include "ridge_regression_util.h"

class RidgeRegressionTest {
 public:
  void Init(fx_module *module) {
    module_ = module;
    data::Load("predictors.csv", &predictors_);
    data::Load("predictions.csv", &predictions_);
    data::Load("true_factors.csv", &true_factors_);
  }

  void TestRegressVersusSVDRegress() {

    engine_ = new RidgeRegression();
    engine_->Init(module_, predictors_, predictions_);
    engine_->Regress(0);
    RidgeRegression svd_engine;
    svd_engine.Init(module_, predictors_, predictions_);
    svd_engine.SVDRegress(0);
    Matrix factors, svd_factors;
    engine_->factors(&factors);
    svd_engine.factors(&svd_factors);
    
    for(index_t i=0; i<factors.n_rows(); i++) {
      TEST_DOUBLE_APPROX(factors.get(i, 0), svd_factors.get(i, 0), 1e-3);
    }
    
    Destruct();
  }

  void TestVIFBasedFeatureSelection() {
    
    // Craft a synthetic dataset in which the third dimension is
    // completely dependent on the first and the second.
    Matrix synthetic_data;
    synthetic_data.Init(4, 5);
    for(index_t i = 0; i < 5; i++) {
      synthetic_data.set(0, i, i);
      synthetic_data.set(1, i, 3 * i + 1);
      synthetic_data.set(2, i, 4);
      synthetic_data.set(3, i, 5);
    }
    GenVector<index_t> predictor_indices;
    GenVector<index_t> prune_predictor_indices;
    GenVector<index_t> output_predictor_indices;
    predictor_indices.Init(4);
    predictor_indices[0] = 0;
    predictor_indices[1] = 1;
    predictor_indices[2] = 2;
    predictor_indices[3] = 3;
    prune_predictor_indices.Copy(predictor_indices);

    RidgeRegressionUtil::FeatureSelection(module_, synthetic_data, 
					  predictor_indices,
					  prune_predictor_indices,
					  &output_predictor_indices);
    printf("Output indices: ");
    for(index_t i = 0; i < output_predictor_indices.length(); i++) {
      printf(" %d ", output_predictor_indices[i]);
    }
    printf("\n");
  }

  void TestAll() {
    TestRegressVersusSVDRegress();
    TestVIFBasedFeatureSelection();
    NOTIFY("[*] Test1 passed !!");
  }  

  void Destruct() {
    delete engine_;
  }

 private:
  fx_module *module_;
  RidgeRegression *engine_;
  Matrix predictors_;
  Matrix predictions_;
  Matrix true_factors_;
};

int main(int argc, char *argv[]) {
  fx_module *module = fx_init(argc, argv, NULL);
  fx_set_param_double(module, "lambda", 1.0);
  RidgeRegressionTest  test;
  test.Init(module);
  test.TestAll();
  fx_done(module);
}

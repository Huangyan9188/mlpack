/**
 * =====================================================================================
 *
 *       Filename:  optim_test.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/11/2008 10:52:49 PM EST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Nikolaos Vasiloglou (NV), nvasil@ieee.org
 *        Company:  Georgia Tech Fastlab-ESP Lab
 *
 * =====================================================================================
 */

#define EPSILON 1.0e-4
#include "optimizer.h"
class Rosen {
public:
  Rosen() {
    dimension_ = 2;
    initval_.Init(dimension_);
    initval_[0]=0.;
    initval_[1]=0.;
  };

  ~Rosen(){}; 

  void Init(Vector &x) {
    dimension_ = x.length();
    initval_.Copy(x);
  }

  void GiveInit(Vector *vec) {
    (*vec)[0]=initval_[0];
    (*vec)[1]=initval_[1]; 
  }

  void ComputeObjective(Vector &x, double *value) {
    double x1=x[0];
    double x2=x[1];
    double f1=(x2-x1*x1);
    double f2=1.-x1;
    *value = 100. *f1*f1+f2*f2;
  } 

  void ComputeGradient(Vector &x, Vector *gx) {
    double x1=x[0];
    double x2=x[1];
    double f1=(x2-x1*x1);
    double f2=1.-x1;
    (*gx)[0]=-400.*f1*x1-2.*f2;
    (*gx)[1]=200.*f1;
  }

  void ComputeHessian(Vector &x, Matrix *hx) {
    double x1=x[0];
    double x2=x[1];
    double f1=(x2-x1*x1);
    hx->set(0,0,-400.*f1+800.*x1*x1 + 2.);
    hx->set(0,1,-400.*x1);
    hx->set(1,0,-400.*x1);
    hx->set(1,1,200.);
  }

  void GetBoundConstraint(Vector *lb, Vector *ub) {
    (*lb)[0] = -0.5;
    (*lb)[1] = -0.5;
    (*ub)[0] = 0.5;
    (*ub)[1] = 0.5;
  }

  void GetLinearEquality(Matrix *a_mat, Vector *b_vec) {
    // assuming that the matrix and vector are initialized 
    // to fit 2 constraints
    a_mat->set(0,0,1.);
    a_mat->set(0,1,1.);
    a_mat->set(1,0,1.);
    a_mat->set(1,1,-2.);
    (*b_vec)[0] = 1.;
    (*b_vec)[1] = 0.;
  }

  void GetLinearInequality(Matrix *a_mat, Vector *lb_vec, Vector *ub_vec) {
    // assuming that the matrix and vector are initialized 
    // to fit 2 constraints
    a_mat->set(0,0,1.);
    a_mat->set(0,1,1.);
    a_mat->set(1,0,1.);
    a_mat->set(1,1,-2.);
    (*lb_vec)[0] = 0.;
    (*ub_vec)[0] = 1.;
    (*lb_vec)[1] = -0.5;
    (*ub_vec)[1] = 0.5;
  }

  index_t num_of_non_linear_equalities() {

  }

  void ComputeNonLinearEqualityConstraints(Vector &x, Vector *c) {

  }

  void ComputeNonLinearEqualityConstraintsJacobian(Vector &x, Matrix *c_jacob) {

  }

  index_t num_of_non_linear_inequalities() {

  }

  void ComputeNonLinearInequalityConstraints(Vector &x, Vector *c) {

  }

  void ComputeNonLinearInequalityConstraintsJacobian(Vector &x, Matrix *c_jacobi) {

  }

  void GetNonLinearInequalityConstraintBounds(Vector *lb, Vector *ub) {

  }

  index_t dimension() {
    return dimension_;
  } 

private:
  index_t dimension_;
  Vector initval_;
    
};


class StaticOptppOptimizerTest {
public:
  StaticOptppOptimizerTest(fx_module *module) {
    module_ = module;
    trueval_.Init(2);
    trueval_[0] = 1.0;
    trueval_[1] = 1.0;
  }

  void TestLBFGS() {
    Rosen rosen;
    optimizer_LBFGS_.Init(module_, &rosen);
    Vector result;
    optimizer_LBFGS_.Optimize(&result);  
    for (index_t i = 0; i < trueval.length(); i++) {
      DEBUG_WARNING_IF(fabs(result[i] - trueval_[i]) > EPSILON);
    }
  }

  void TestLBFGS_BC() {
    Rosen rosen;
    optimizer_LBFGS_BC_.Init(module_, &rosen);
    Vector result;
    optimizer_LBFGS_BC_.Optimize(&result);
  }

  void TestLBFGS_LE() {
    Rosen rosen;
    optimizer_LBFGS_LE_.Init(module_, &rosen);
    Vector result;
    optimizer_LBFGS_LE_.Optimize(&result);
  }

  void TestLBFGS_LI() {
    Rosen rosen;
    optimizer_LBFGS_LI_.Init(module_, &rosen);
    Vector result;
    optimizer_LBFGS_LI_.Optimize(&result);
  }

  void TestLBFGS_NLE() {}

  void TestLBFGS_NLI() {}

  void TestCG() {
    Rosen rosen;
    optimizer_CG_.Init(module_, &rosen);
    Vector result;
    optimizer_CG_.Optimize(&result);  
    for (index_t i = 0; i < trueval.length(); i++) {
      DEBUG_WARNING_IF(fabs(result[i] - trueval_[i]) > EPSILON);
    }
  }
  void TestQNewton() {
    Rosen rosen;
    optimizer_QNewton_.Init(module_, &rosen);
    Vector result;
    optimizer_QNewton_.Optimize(&result);  
    for (index_t i = 0; i < trueval.length(); i++) {
      DEBUG_WARNING_IF(fabs(result[i] - trueval_[i]) > EPSILON);
    }
  }
  void TestBFGS() {
    Rosen rosen;
    optimizer_BFGS_.Init(module_, &rosen);
    Vector result;
    optimizer_BFGS_.Optimize(&result);  
    for (index_t i = 0; i < trueval.length(); i++) {
      DEBUG_WARNING_IF(fabs(result[i] - trueval_[i]) > EPSILON);
    }
  }
  void TestFDNewton() {
    Rosen rosen;
    optimizer_FDNewton_.Init(module_, &rosen);
    Vector result;
    optimizer_FDNewton_.Optimize(&result);  
    for (index_t i = 0; i < trueval.length(); i++) {
      DEBUG_WARNING_IF(fabs(result[i] - trueval_[i]) > EPSILON);
    }
  }
  void TestNewton() {
    Rosen rosen;
    optimizer_Newton_.Init(module_, &rosen);
    Vector result;
    optimizer_Newton_.Optimize(&result);  
    for (index_t i = 0; i < trueval.length(); i++) {
      DEBUG_WARNING_IF(fabs(result[i] - trueval_[i]) > EPSILON);
    }
  }
  void TestAll() {
    TestLBFGS();
    TestLBFGS_BC();
    TestLBFGS_LE();
    TestLBFGS_LI();
    TestLBFGS_NLE();
    TestLBFGS_NLI();

    TestCG();
    TestQNewton();
    TestBFGS();
    TestFDNewton();
    TestNewton();
  }
private:
  fx_module *module_;
  optim::optpp::StaticOptppOptimizer<optim::optpp::LBFGS, Rosen> optimizer_LBFGS_;
  optim::optpp::StatisOptppOptimizer<optim::optpp::LBFGS, Rosen, 
				     optim::optpp::BoundConstraint> optimizer_LBFGS_BC_;
  optim::optpp::StatisOptppOptimizer<optim::optpp::LBFGS, Rosen, 
				     optim::optpp::LinearEquality> optimizer_LBFGS_LE_;
  optim::optpp::StatisOptppOptimizer<optim::optpp::LBFGS, Rosen, 
				     optim::optpp::LinearInequality> optimizer_LBFGS_LI_;
  optim::optpp::StaticOptppOptimizer<optim::optpp::CG, Rosen> optimizer_CG_;
  optim::optpp::StaticOptppOptimizer<optim::optpp::QNewton, Rosen> optimizer_QNewton_;
  optim::optpp::StaticOptppOptimizer<optim::optpp::BFGS, Rosen> optimizer_BFGS_;
  optim::optpp::StaticOptppOptimizer<optim::optpp::FDNewton, Rosen> optimizer_FDNewton_;
  optim::optpp::StaticOptppOptimizer<optim::optpp::Newton, Rosen> optimizer_Newton_;
  Vector trueval_;
};

int main(int argc, char *argv[]) {
  fx_module *fx_root = fx_init(argc, argv, NULL);
  StaticOptppOptimizerTest test(fx_root);
  test.TestAll();
  fx_done(fx_root);
  return 0;
}

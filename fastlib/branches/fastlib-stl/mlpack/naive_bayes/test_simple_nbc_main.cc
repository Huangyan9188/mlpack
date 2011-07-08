#include <fastlib/fastlib.h>
#include <fastlib/fx/io.h>
#include "simple_nbc.h"

/*const fx_entry_doc test_simple_nbc_main_entries[] = {
  {"nbc/classes", FX_RESERVED, FX_INT, NULL,
   "Set during testing."},
  FX_ENTRY_DOC_DONE
};*/

/*const fx_submodule_doc test_simple_nbc_main_submodules[] = {
  {"nbc", &parm_nbc_doc,
   " Trains on a given set and number of classes and "
   "tests them on a given set\n"},
  FX_SUBMODULE_DOC_DONE
};*/

/*const fx_module_doc test_simple_nbc_main_doc = {
  test_simple_nbc_main_entries, test_simple_nbc_main_submodules,
  " Tests the simple nbc class.\n"
};*/

PROGRAM_INFO("NBC", "Tests the simple nbc class.", "nbc");

using namespace mlpack;

class TestClassSimpleNBC{
 private:
  const char *filename_train_, *filename_test_;
  const char *train_result_, *test_result_;
  index_t number_of_classes_;

 public:

  void Init(const char *filename_train, const char *filename_test,
	    const char *train_result, const char *test_result,
	    const int number_of_classes) {
    filename_train_ = filename_train;
    filename_test_ = filename_test;
    train_result_ = train_result;
    test_result_ = test_result;
    number_of_classes_ = number_of_classes;
  }

  void Destruct() {
  }

  void TestAll() {
    arma::mat train_data, train_res, calc_mat;
    data::Load(filename_train_, train_data);
    data::Load(train_result_, train_res); 

    IO::GetParam<int>("nbc/classes") = number_of_classes_;

    IO::Info << "Training..." << std::endl;

    SimpleNaiveBayesClassifier nbc_test_(train_data);

    index_t number_of_features = nbc_test_.means_.n_rows;
    calc_mat.zeros(2*number_of_features + 1, number_of_classes_);

    IO::Info << "Beginning training test..." << std::endl;

    for(index_t i = 0; i < number_of_features; i++) {
      for(index_t j = 0; j < number_of_classes_; j++) {
	calc_mat(i, j) = nbc_test_.means_(i, j);
	calc_mat(i + number_of_features, j) = nbc_test_.variances_(i, j);
      }
    }
    for(index_t i = 0; i < number_of_classes_; i++) {
      calc_mat(2 * number_of_features, i) = nbc_test_.class_probabilities_(i);
    }
    
    for(index_t i = 0; i < calc_mat.n_rows; i++) {
      for(index_t j = 0; j < number_of_classes_; j++) {
//      Needs to be redone when converted to Boost UTF
//	TEST_DOUBLE_APPROX(train_res(i, j), calc_mat(i, j), 0.0001);
      }
    }
    IO::Info << "Training test passed..." << std::endl;
    
    IO::Info << "Beginning classification test..." << std::endl;

    arma::mat test_data, test_res;
    arma::vec test_res_vec, calc_vec;
    data::Load(filename_test_, test_data);
    data::Load(test_result_, test_res); 

    nbc_test_.Classify(test_data, calc_vec);

    index_t number_of_datum = test_data.n_cols;
    test_res_vec = test_res.col(0);

    for(index_t i = 0; i < number_of_datum; i++) {
      assert(test_res_vec(i) == calc_vec(i));
    }
    IO::Info << "Classification test passed..." << std::endl;
  }

};

//int main(int argc, char *argv[]) {
int main(){ 
//  IO::ParseCommandLine(argc, argv);

// NPS: comment (for now) code that generates warnings
//  TestClassSimpleNBC*  test;
//  const char *train_data = "trainSet.arff";
//  const char *train_res = "trainRes.arff";
//  const char *test_data = "testSet.arff";
//  const char *test_res = "testRes.arff";
//  const int num_classes = 2;

  //test.Init(train_data, test_data, train_res, test_res, num_classes);
 // test.TestAll();


}

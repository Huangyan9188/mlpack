/** @file mixed_logit.test.cc
 *
 *  A "stress" test driver for mixed logit.
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 */

#include <numeric>
#include <time.h>
#include "core/table/table.h"
#include "core/tree/gen_metric_tree.h"
#include "mlpack/mixed_logit_dcm/constant_distribution.h"
#include "mlpack/mixed_logit_dcm/distribution.h"
#include "mlpack/mixed_logit_dcm/gaussian_distribution.h"
#include "mlpack/mixed_logit_dcm/mixed_logit_dcm_argument_parser.h"
#include "mlpack/mixed_logit_dcm/mixed_logit_dcm_dev.h"

namespace mlpack {
namespace mixed_logit_dcm {

int num_attributes_;
int num_people_;
std::vector<int> num_discrete_choices_;

class TestMixedLogitDCM {

  private:

    template<typename TableType>
    void GenerateRandomDataset_(
      TableType *random_attribute_dataset,
      TableType *random_decisions_dataset,
      TableType *random_num_alternatives_dataset) {

      // Find the total number of discrete choices.
      int total_num_attributes =
        std::accumulate(
          mlpack::mixed_logit_dcm::num_discrete_choices_.begin(),
          mlpack::mixed_logit_dcm::num_discrete_choices_.end(), 0);

      random_attribute_dataset->Init(
        mlpack::mixed_logit_dcm::num_attributes_, total_num_attributes);

      for(int j = 0; j < total_num_attributes; j++) {
        core::table::DensePoint point;
        random_attribute_dataset->get(j, &point);
        for(int i = 0; i < mlpack::mixed_logit_dcm::num_attributes_; i++) {
          point[i] = core::math::Random(0.1, 1.0);
        }
      }

      random_num_alternatives_dataset->Init(
        1, mlpack::mixed_logit_dcm::num_people_);
      for(int j = 0; j < mlpack::mixed_logit_dcm::num_people_; j++) {
        core::table::DensePoint point;
        random_num_alternatives_dataset->get(j, &point);

        // This is the number of discrete choices for the given
        // person.
        point[0] = mlpack::mixed_logit_dcm::num_discrete_choices_[j];
      }
      random_decisions_dataset->Init(
        1, mlpack::mixed_logit_dcm::num_people_);
      for(int j = 0; j < mlpack::mixed_logit_dcm::num_people_; j++) {
        core::table::DensePoint point;
        random_decisions_dataset->get(j, &point);

        // This is the discrete choice index of the given person.
        point[0] = core::math::RandInt(
                     mlpack::mixed_logit_dcm::num_discrete_choices_[j]) + 1;
      }
    }

  public:

    int StressTestMain() {
      for(int i = 0; i < 1; i++) {
        for(int k = 0; k < 1; k++) {

          // Randomly choose the number of attributes and the number
          // of people and the number of discrete choices per each
          // person.
          mlpack::mixed_logit_dcm::num_attributes_ = core::math::RandInt(3, 5);
          mlpack::mixed_logit_dcm::num_people_ = core::math::RandInt(50, 70);
          mlpack::mixed_logit_dcm::num_discrete_choices_.resize(
            mlpack::mixed_logit_dcm::num_people_);
          for(int j = 0; j < mlpack::mixed_logit_dcm::num_people_; j++) {
            mlpack::mixed_logit_dcm::num_discrete_choices_[j] =
              core::math::RandInt(3, 7);
          }

          switch(k) {
            case 0:

              // Test the constant distribution.
              StressTest<mlpack::mixed_logit_dcm::ConstantDistribution>();
              break;
            case 1:

              // Test the Gaussian distribution.
              StressTest<mlpack::mixed_logit_dcm::GaussianDistribution>();
              break;
          }
        }
      }
      return 0;
    }

    template<typename DistributionType>
    int StressTest() {

      typedef core::table::Table <
      core::tree::GenMetricTree <
      core::tree::AbstractStatistic > > TableType;

      // The list of arguments.
      std::vector< std::string > args;

      // Push in the reference dataset name.
      std::string attributes_in("random_attributes.csv");
      args.push_back(std::string("--attributes_in=") + attributes_in);

      // Push in the decision set info name.
      std::string decisions_in(
        "random_decisions.csv");
      args.push_back(
        std::string("--decisions_in=") +
        decisions_in);

      // Push in the number of alternatives.
      std::string num_alternatives_in("random_num_alternatives.csv");
      args.push_back(
        std::string("--num_alternatives_in=") +
        num_alternatives_in);

      // Print out the header of the trial.
      std::cout << "\n==================\n";
      std::cout << "Test trial begin\n";
      std::cout << "Number of attributes: " <<
                mlpack::mixed_logit_dcm::num_attributes_ << "\n";
      std::cout << "Number of people: " <<
                mlpack::mixed_logit_dcm::num_people_ << "\n";

      // Generate the random dataset and save it.
      TableType random_attribute_table;
      TableType random_decisions_table;
      TableType random_num_alternatives_table;
      GenerateRandomDataset_(
        &random_attribute_table,
        &random_decisions_table,
        &random_num_alternatives_table);
      random_attribute_table.Save(attributes_in);
      random_decisions_table.Save(decisions_in);
      random_num_alternatives_table.Save(num_alternatives_in);

      // Parse the mixed logit DCM arguments.
      mlpack::mixed_logit_dcm::MixedLogitDCMArguments<TableType> arguments;
      boost::program_options::variables_map vm;
      mlpack::mixed_logit_dcm::MixedLogitDCMArgumentParser::
      ConstructBoostVariableMap(args, &vm);
      mlpack::mixed_logit_dcm::MixedLogitDCMArgumentParser::ParseArguments(
        vm, &arguments);

      // Call the mixed logit driver.
      mlpack::mixed_logit_dcm::MixedLogitDCM <
      TableType, DistributionType > instance;
      instance.Init(arguments);

      // Compute the result.
      mlpack::mixed_logit_dcm::MixedLogitDCMResult result;
      instance.Compute(arguments, &result);

      return 0;
    };
};
}
}

int main(int argc, char *argv[]) {

  // Call the tests.
  mlpack::mixed_logit_dcm::TestMixedLogitDCM dcm_test;
  dcm_test.StressTestMain();

  std::cout << "All tests passed!\n";
  return 0;
}

/** @file distributed_kpca.h
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 */

#ifndef MLPACK_DISTRIBUTED_KPCA_DISTRIBUTED_KPCA_H
#define MLPACK_DISTRIBUTED_KPCA_DISTRIBUTED_KPCA_H

#include <boost/math/distributions/normal.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/program_options.hpp>
#include "core/table/distributed_table.h"
#include "mlpack/distributed_kpca/distributed_kpca_arguments.h"
#include "mlpack/distributed_kpca/kpca_result.h"

namespace mlpack {
namespace distributed_kpca {

/** @brief The argument parsing class for distributed KPCA computation.
 */
class DistributedKpcaArgumentParser {
  public:
    template<typename DistributedTableType>
    static bool ParseArguments(
      boost::mpi::communicator &world,
      boost::program_options::variables_map &vm,
      mlpack::distributed_kpca::DistributedKpcaArguments <
      DistributedTableType > *arguments_out);

    static bool ConstructBoostVariableMap(
      boost::mpi::communicator &world,
      const std::vector<std::string> &args,
      boost::program_options::variables_map *vm);

    static bool ConstructBoostVariableMap(
      boost::mpi::communicator &world,
      int argc,
      char *argv[],
      boost::program_options::variables_map *vm);

    template<typename TableType>
    static void RandomGenerate(
      boost::mpi::communicator &world, const std::string &file_name,
      int num_dimensions, int num_points, const std::string &prescale_option);
};

template<typename IncomingDistributedTableType, typename IncomingKernelType>
class DistributedKpca {
  public:

    typedef IncomingDistributedTableType DistributedTableType;

    typedef IncomingKernelType KernelType;

    typedef typename DistributedTableType::TableType TableType;

    typedef mlpack::distributed_kpca::KpcaResult ResultType;

    typedef mlpack::distributed_kpca::DistributedKpcaArguments <
    TableType > ArgumentType;

  private:

    void GenerateRandomFourierFeatures_(
      const mlpack::distributed_kpca::DistributedKpcaArguments <
      DistributedTableType > &arguments_in,
      const KernelType &kernel,
      int num_random_fourier_features,
      std::vector <
      core::table::DensePoint > *random_variates,
      std::vector< arma::vec > *random_variate_aliases);

  public:

    DistributedKpca();

    /** @brief Initialize a Kpca engine with the arguments.
     */
    void Init(
      boost::mpi::communicator &world_in,
      mlpack::distributed_kpca::DistributedKpcaArguments <
      DistributedTableType > &arguments_in);

    void Compute(
      const mlpack::distributed_kpca::DistributedKpcaArguments <
      DistributedTableType > &arguments_in,
      ResultType *result_out);

  private:

    /** @brief The communicator.
     */
    boost::mpi::communicator *world_;

    /** @brief The normal distribution object.
     */
    boost::math::normal normal_dist_;

    double mult_const_;

    double effective_num_reference_points_;

    double correction_term_;
};
}
}

#endif

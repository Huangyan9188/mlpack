/** @file distributed_dualtree_dfs.h
 *
 *  The prototype header for performing a distributed pairwise GNPs.
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 */

#ifndef CORE_GNP_DISTRIBUTED_DUALTREE_DFS_H
#define CORE_GNP_DISTRIBUTED_DUALTREE_DFS_H

#include <boost/mpi/communicator.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/tuple/tuple.hpp>
#include "core/gnp/dualtree_dfs.h"
#include "core/math/range.h"
#include "core/parallel/subtable_send_request.h"
#include "core/parallel/table_exchange.h"
#include "core/table/sub_table.h"
#include "core/table/sub_table_list.h"
#include <omp.h>

namespace core {
namespace gnp {

template<typename DistributedProblemType>
class DistributedDualtreeDfs {

  public:

    /** @brief The table type.
     */
    typedef typename DistributedProblemType::TableType TableType;

    /** @brief The distributed computation problem.
     */
    typedef typename DistributedProblemType::ProblemType ProblemType;

    /** @brief The distributed table type.
     */
    typedef typename DistributedProblemType::DistributedTableType
    DistributedTableType;

    /** @brief The local tree type.
     */
    typedef typename TableType::TreeType TreeType;

    /** @brief The distributed tree type.
     */
    typedef typename DistributedTableType::TreeType DistributedTreeType;

    /** @brief The global constant type for the problem.
     */
    typedef typename DistributedProblemType::GlobalType GlobalType;

    /** @brief The type of the result being outputted.
     */
    typedef typename DistributedProblemType::ResultType ResultType;

    /** @brief The argument type of the computation.
     */
    typedef typename DistributedProblemType::ArgumentType ArgumentType;

    /** @brief The type of the object used for prioritizing the
     *         computation globally (or on a coarse scale).
     */
    typedef boost::tuple <
    TreeType *, boost::tuple<int, int, int>, double > CoarseFrontierObjectType;

    /** @brief The type of the object used for prioritizing the
     *         computation per stage on a shared memory (on a fine
     *         scale).
     */
    typedef boost::tuple <
    TreeType *,
             boost::tuple<TableType *, TreeType *, int>, double > FineFrontierObjectType;

    /** @brief The type of the subtable in use.
     */
    typedef core::table::SubTable<TableType> SubTableType;

    /** @brief The type of the list of subtables in use.
     */
    typedef core::table::SubTableList<SubTableType> SubTableListType;

  private:

    /** @brief The pointer to the boost communicator.
     */
    boost::mpi::communicator *world_;

    /** @brief The problem definition for the distributed computation.
     */
    DistributedProblemType *problem_;

    /** @brief The distributed query table.
     */
    DistributedTableType *query_table_;

    /** @brief The distributed reference table.
     */
    DistributedTableType *reference_table_;

    /** @brief The maximum number of points a leaf node of a local
     *         tree contains.
     */
    int leaf_size_;

    /** @brief The maximum size of the subtree to serialize at a time.
     */
    int max_subtree_size_;

    /** @brief The maximum number of work items to dequeue per
     *         process.
     */
    int max_num_work_to_dequeue_per_stage_;

    /** @brief Some statistics about the priority queue size during
     *         the computation.
     */
    int max_computation_frontier_size_;

    /** @brief The number of deterministic prunes.
     */
    int num_deterministic_prunes_;

    /** @brief The number of probabilistic prunes.
     */
    int num_probabilistic_prunes_;

  private:

    /** @brief The class used for prioritizing a sending operation.
     */
    class PrioritizeSendTasks_:
      public std::binary_function <
      core::parallel::SubTableSendRequest &,
      core::parallel::SubTableSendRequest &,
        bool > {
      public:
        bool operator()(
          const core::parallel::SubTableSendRequest &a,
          const core::parallel::SubTableSendRequest &b) const {
          return a.priority() < b.priority();
        }
    };

    /** @brief The class used for prioritizing a computation object
     *         (query, reference pair).
     */
    template<typename FrontierObjectType>
    class PrioritizeTasks_:
      public std::binary_function <
        FrontierObjectType &, FrontierObjectType &, bool > {
      public:
        bool operator()(
          const FrontierObjectType &a, const FrontierObjectType &b) const {
          return a.get<2>() < b.get<2>();
        }
    };

    /** @brief The type of the priority queue used for prioritizing
     *         the send operations.
     */
    typedef std::priority_queue <
    core::parallel::SubTableSendRequest,
         std::vector< core::parallel::SubTableSendRequest >,
         PrioritizeSendTasks_ >
         SendRequestPriorityQueueType;

    /** @brief The type of the priority queue that is used for
     *         prioritizing the coarse-grained computations.
     */
    typedef std::priority_queue <
    CoarseFrontierObjectType,
    std::vector<CoarseFrontierObjectType>,
    PrioritizeTasks_<CoarseFrontierObjectType> >
    CoarsePriorityQueueType;

    /** @brief The type of the priority queue that is used for
     *         prioritizing the fine-grained computations.
     */
    typedef std::priority_queue <
    FineFrontierObjectType,
    std::vector<FineFrontierObjectType>,
    PrioritizeTasks_<FineFrontierObjectType> >
    FinePriorityQueueType;

    template<typename MetricType>
    void ComputeEssentialReferenceSubtrees_(
      const MetricType &metric_in,
      int max_reference_subtree_size,
      DistributedTreeType *global_query_node,
      TreeType *local_reference_node,
      std::vector< std::vector< std::pair<int, int> > > *
      essential_reference_subtrees,
      std::vector< std::vector< core::math::Range> > *
      remote_priorities,
      std::vector<double> *extrinsic_prunes);

    template<typename MetricType>
    void GenerateTasks_(
      const MetricType &metric_in,
      core::parallel::TableExchange <
      DistributedTableType, SubTableListType > &table_exchange,
      std::vector<TreeType *> &local_query_subtrees,
      const std::vector <
      boost::tuple<int, int, int, int> > &received_subtable_ids,
      std::vector< FinePriorityQueueType > *tasks);

    template<typename MetricType>
    void InitialSetup_(
      const MetricType &metric,
      typename DistributedProblemType::ResultType *query_results,
      core::parallel::TableExchange <
      DistributedTableType, SubTableListType > &table_exchange,
      std::vector< TreeType *> *local_query_subtrees,
      std::vector< std::vector< std::pair<int, int> > > *
      essential_reference_subtrees_to_send,
      std::vector< std::vector< core::math::Range> > *send_priorities,
      std::vector< SendRequestPriorityQueueType > *prioritized_send_subtables,
      int *num_reference_subtrees_to_send,
      std::vector <
      std::vector< std::pair<int, int> > > *reference_frontier_lists,
      std::vector< std::vector< core::math::Range> > *receive_priorities,
      int *num_reference_subtrees_to_receive,
      std::vector< FinePriorityQueueType > *tasks);

    /** @brief The collaborative way of exchanging items among all MPI
     *         processes for a distributed computation. This routine
     *         utilizes asynchronous MPI calls to maximize
     *         communication and computation overlap.
     */
    template<typename MetricType>
    void AllToAllIReduce_(
      const MetricType &metric,
      typename DistributedProblemType::ResultType *query_results);

    void ResetStatisticRecursion_(
      DistributedTreeType *node, DistributedTableType * table);

    template<typename TemplateTreeType>
    void PreProcessReferenceTree_(TemplateTreeType *rnode);

    template<typename TemplateTreeType>
    void PreProcess_(TemplateTreeType *qnode);

    template<typename MetricType>
    void PostProcess_(
      const MetricType &metric,
      TreeType *qnode, ResultType *query_results);

  public:

    /** @brief Returns the number of deterministic prunes so far.
     */
    int num_deterministic_prunes() const;

    /** @brief Returns the number of probabilistic prunes so far.
     */
    int num_probabilistic_prunes() const;

    /** @brief Sets the tweak parameters for the maximum number of
     *         levels of trees to grab at a time and the maximum
     *         number of work per stage to dequeue.
     */
    void set_work_params(
      int leaf_size_in,
      int max_subtree_size_in,
      int max_num_work_to_dequeue_per_stage_in);

    /** @brief The default constructor.
     */
    DistributedDualtreeDfs();

    /** @brief Returns the associated problem.
     */
    DistributedProblemType *problem();

    /** @brief Returns the distributed query table.
     */
    DistributedTableType *query_table();

    /** @brief Returns the distributed reference table.
     */
    DistributedTableType *reference_table();

    /** @brief Resets the statistics of the query tree.
     */
    void ResetStatistic();

    /** @brief Initializes the distributed dualtree engine.
     */
    void Init(
      boost::mpi::communicator *world, DistributedProblemType &problem_in);

    /** @brief Initiates the distributed computation.
     */
    template<typename MetricType>
    void Compute(
      const MetricType &metric,
      typename DistributedProblemType::ResultType *query_results);
};
}
}

#endif

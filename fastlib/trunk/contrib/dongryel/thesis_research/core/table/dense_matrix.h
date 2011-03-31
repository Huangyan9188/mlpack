/** @file dense_matrix.h
 *
 *  A class representing a dense column-oriented matrix.
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 */

#ifndef CORE_TABLE_DENSE_MATRIX_H
#define CORE_TABLE_DENSE_MATRIX_H

#include <armadillo>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include "dense_point.h"
#include "memory_mapped_file.h"

namespace core {
namespace table {

/** @brief The trait class for determining the number of rows for a
 *         matrix-like object. Currently supports the
 *         core::table::DensePoint and arma::vec class objects.
 */
template<typename MatrixType>
class RowTrait {
  public:
    static int n_rows(const MatrixType &p);
};

template<typename MatrixType>
class ColTrait {
  public:
    static int n_cols(const MatrixType &p);
};

/** @brief The dense column-oriented matrix.
 */
class DenseMatrix {

  private:

    // For serialization.
    friend class boost::serialization::access;

    /** @brief The pointer to the matrix.
     */
    boost::interprocess::offset_ptr<double> ptr_;

    /** @brief The number of rows.
     */
    int n_rows_;

    /** @brief The number of columns.
     */
    int n_cols_;

    /** @brief The matrix is an alias or not.
     */
    bool is_alias_;

  public:

    /* @brief Returns whether the matrix is aliasing another location
     *        of memory.
     */
    bool is_alias() const {
      return is_alias_;
    }

    /** @brief Not a true copy constructor. Simply sets the matrix to
     *         be an alias.
     */
    void operator=(const DenseMatrix &dense_matrix_in) {
      ptr_ = const_cast<double *>(dense_matrix_in.ptr());
      n_rows_ = dense_matrix_in.n_rows();
      n_cols_ = dense_matrix_in.n_cols();
      is_alias_ = true;
    }

    /** @brief Saves the matrix.
     */
    template<class Archive>
    void save(Archive &ar, const unsigned int version) const {
      ar & n_rows_;
      ar & n_cols_;
      int num_elements = n_rows_ * n_cols_;
      for(int i = 0; i < num_elements; i++) {
        ar & ptr_.get()[i];
      }
    }

    /** @brief Unserializes the matrix.
     */
    template<class Archive>
    void load(Archive &ar, const unsigned int version) {
      ar & n_rows_;
      ar & n_cols_;
      int num_elements = n_rows_ * n_cols_;
      if(is_alias_ == false) {
        ptr_ =
          (core::table::global_m_file_) ?
          core::table::global_m_file_->ConstructArray<double>(num_elements) :
          new double[num_elements];
      }
      for(int i = 0; i < num_elements; i++) {
        ar & ptr_[i];
      }
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    /** @brief Prints the matrix.
     */
    void Print() const {
      for(int i = 0; i < n_rows_; i++) {
        for(int j = 0; j < n_cols_; j++) {
          printf("%g, ", this->get(i, j));
        }
        printf("\n");
      }
    }

    /** @brief Sets everything to zero.
     */
    void SetZero() {
      if(ptr_.get() != NULL && n_rows_ > 0 && n_cols_ > 0) {
        memset(ptr_.get(), 0, sizeof(double) * n_rows_ * n_cols_);
      }
    }

    /** @brief Sets every entry of the matrix to the specified value
     *         val.
     */
    void SetAll(double val) {
      for(int j = 0; j < n_cols_; j++) {
        for(int i = 0; i < n_rows_; i++) {
          this->set(i, j, val);
        }
      }
    }

    /** @brief Returns the raw pointer.
     */
    const double *ptr() const {
      return ptr_.get();
    }

    /** @brief Returns the raw pointer.
     */
    double *ptr() {
      return ptr_.get();
    }

    /** @brief The number of rows returned.
     */
    int n_rows() const {
      return n_rows_;
    }

    /** @brief The number of columns returned.
     */
    int n_cols() const {
      return n_cols_;
    }

    /** @brief Swap the contents of the given two columns.
     */
    void swap_cols(int first_col, int second_col) {
      double *first_ptr = ptr_.get() + first_col * n_rows_;
      double *second_ptr = ptr_.get() + second_col * n_rows_;
      for(int i = 0; i < n_rows_; first_ptr++, second_ptr++, i++) {
        std::swap(*first_ptr, *second_ptr);
      }
    }

    /** @brief Sets the value of the particular location of the
     *         matrix.
     */
    void set(int row, int col, double val) {
      ptr_[col * n_rows_ + row] = val;
    }

    /** @brief Gets the value stored at (row, col) location.
     */
    double get(int row, int col) const {
      return ptr_[col * n_rows_ + row];
    }

    /** @brief Resets the matrix to be a zero by zero matrix.
     */
    void Reset() {
      ptr_ = NULL;
      n_rows_ = n_cols_ = 0;
      is_alias_ = false;
    }

    /** @brief The default constructor.
     */
    DenseMatrix() {
      Reset();
    }

    /** @brief The destructor.
     */
    ~DenseMatrix() {
      if(is_alias_ == false) {
        if(ptr_.get() != NULL) {
          if(core::table::global_m_file_) {
            core::table::global_m_file_->DestroyPtr(ptr_.get());
          }
          else {
            delete[] ptr_.get();
          }
        }
      }
      Reset();
    }

    /** @brief Initializes the matrix for a given dimension.
     */
    void Init(
      int n_rows_in, int n_cols_in) {

      ptr_ = (core::table::global_m_file_) ?
             core::table::global_m_file_->ConstructArray<double>(
               n_rows_in * n_cols_in) :
             new double[n_rows_in * n_cols_in];
      n_rows_ = n_rows_in;
      n_cols_ = n_cols_in;
    }

    /** @brief Returns the raw pointer to the given column.
     */
    const double *GetColumnPtr(int column_id) const {
      return ptr_.get() + column_id * n_rows_;
    }

    /** @brief Returns the raw pointer to the given column.
     */
    double *GetColumnPtr(int column_id) {
      return ptr_.get() + column_id * n_rows_;
    }

    /** @brief Makes a column vector alias.
     */
    void MakeColumnVector(
      int i, core::table::DensePoint *point_out) const {
      point_out->Alias(
        ptr_.get() + i * n_rows_, n_rows_);
    }

    /** @brief Makes a column vector alias.
     */
    void MakeColumnVector(int i, core::table::DensePoint *point_out) {
      point_out->Alias(
        ptr_.get() + i * n_rows_, n_rows_);
    }

    /** @brief Makes a column vector alias.
     */
    void MakeColumnVector(int i, arma::vec *vec_out) const {
      core::table::DoublePtrToArmaVec(
        ptr_.get() + i * n_rows_, n_rows_, vec_out);
    }

    /** @brief Makes a column vector alias.
     */
    void MakeColumnVector(int i, arma::vec *vec_out) {
      core::table::DoublePtrToArmaVec(
        ptr_.get() + i * n_rows_, n_rows_, vec_out);
    }

    /** @brief Makes the current object an alias of the given memory
     *         location.
     */
    void Alias(const double *ptr_in, int n_rows_in, int n_cols_in) {
      ptr_ = const_cast<double *>(ptr_in);
      n_rows_ = n_rows_in;
      n_cols_ = n_cols_in;
      is_alias_ = true;
    }

    template<typename MatrixType>
    void CopyValues(const MatrixType &matrix_in) {
      const double *point_in_ptr =
        core::table::PointerTrait<MatrixType>::ptr(matrix_in);
      n_rows_ = core::table::RowTrait<MatrixType>::n_rows(matrix_in);
      n_cols_ = core::table::ColTrait<MatrixType>::n_cols(matrix_in);
      memcpy(
        ptr_.get(), point_in_ptr, sizeof(double) * n_rows_ * n_cols_);
      is_alias_ = false;
    }

    template<typename MatrixType>
    void Copy(const MatrixType &matrix_in) {
      if(core::table::RowTrait<MatrixType>::n_rows(matrix_in) > 0 &&
          core::table::ColTrait<MatrixType>::n_cols(matrix_in) > 0) {
        if(ptr_.get() == NULL) {
          int length =
            core::table::RowTrait<MatrixType>::n_rows(matrix_in) *
            core::table::ColTrait<MatrixType>::n_cols(matrix_in);
          ptr_ =
            (core::table::global_m_file_) ?
            core::table::global_m_file_->ConstructArray<double>(
              length) : new double[ length ];
        }
        CopyValues(matrix_in);
      }
    }
};

/** @brief The trait class instantiation for determining the number of
 *         rows of an arma::mat object.
 */
template<>
class RowTrait<arma::mat> {
  public:
    static int n_rows(const arma::mat &p) {
      return p.n_rows;
    }
};

/** @brief The trait class instantiation for determining the number of
 *         rows of a core::table::DenseMatrix object.
 */
template<>
class RowTrait<core::table::DenseMatrix> {
  public:
    static int n_rows(const core::table::DenseMatrix &p) {
      return p.n_rows();
    }
};

/** @brief The trait class instantiation for determining the number of
 *         columns of an arma::mat object.
 */
template<>
class ColTrait<arma::mat> {
  public:
    static int n_cols(const arma::mat &p) {
      return p.n_cols;
    }
};

/** @brief The trait class instantiation for determining the number of
 *         columns of a core::table::DenseMatrix object.
 */
template<>
class ColTrait<core::table::DenseMatrix> {
  public:
    static int n_cols(const core::table::DenseMatrix &p) {
      return p.n_cols();
    }
};

template<>
class PointerTrait<arma::mat> {
  public:
    static const double *ptr(const arma::mat &p) {
      return p.memptr();
    }
};

template<>
class PointerTrait<core::table::DenseMatrix> {
  public:
    static const double *ptr(const core::table::DenseMatrix &p) {
      return p.ptr();
    }
};

/** @brief The function that takes a raw double pointer and creates an
 *         alias armadillo vector.
 */
template<typename T>
static void DoublePtrToArmaMat(
  const double *matrix_in, T n_rows_in, T n_cols_in, arma::mat *mat_out) {

  // This constructor uses the const_cast for a hack. For some reason,
  // Armadillo library does not allow creation of aliases for const
  // double pointers, so I used const_cast here.
  const_cast<arma::u32 &>(mat_out->n_rows) = n_rows_in;
  const_cast<arma::u32 &>(mat_out->n_cols) = n_cols_in;
  const_cast<arma::u32 &>(mat_out->n_elem) = n_rows_in * n_cols_in;
  const_cast<arma::u16 &>(mat_out->vec_state) = 0;
  const_cast<arma::u16 &>(mat_out->mem_state) = 2;
  const_cast<double *&>(mat_out->mem) = const_cast<double *>(matrix_in);
}

/** @brief The function that takes a raw pointer out from a
 *         pre-existing core::table::DenseMatrix object and creates an
 *         alias armadillo matrix.
 */
template<typename DenseMatrixType>
static void DenseMatrixToArmaMat(
  const DenseMatrixType &matrix_in, arma::mat *mat_out) {

  core::table::DoublePtrToArmaMat(
    matrix_in.ptr(), matrix_in.n_rows(), matrix_in.n_cols(), mat_out);
}
}
}

#endif

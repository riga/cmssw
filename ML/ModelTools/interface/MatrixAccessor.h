/*
 * Accessor for filling data into and reading data from matrices, i.e., vectors in vectors.
 */

#ifndef ML_MODELTOOLS_INTERFACE_MATRIXACCESSOR_H_
#define ML_MODELTOOLS_INTERFACE_MATRIXACCESSOR_H_

#include "common.h"
#include "DataAccessor.h"

namespace edm {

  namespace ml {

    template <typename T>
    class MatrixAccessor : public DataAccessor<T> {
    public:
      typedef std::vector<std::vector<T>> Matrix;

      MatrixAccessor(const FeatureContainer<T>& container) : DataAccessor<T>(container) { checkContainer(); }

      template <typename... Args>
      MatrixAccessor(const FeatureContainer<T>& container, Matrix& m, Args&&... args) : MatrixAccessor(container) {
        setData(m, std::forward<Args>(args)...);
      }

      MatrixAccessor(const MatrixAccessor& d) : MatrixAccessor(d.container_, d.data_) {}

      MatrixAccessor() = delete;

      void assertMatrixShape(const Matrix& m) const;

      Matrix createMatrix(size_t batchSize = 1);

      void setData(Matrix& matrix);

      void setData(Matrix& matrix, size_t i);

      void setData(Matrix& matrix, size_t i, size_t j);

    protected:
      virtual void checkContainer() const override;
    };

    template <typename T>
    void MatrixAccessor<T>::checkContainer() const {
      if (this->container_.getNDims() != 2) {
        throw cms::Exception("DimensionMismatch")
            << "matrix accessor requires container to have 2 dimensions but got " << this->container_.getNDims();
      }
    }

    template <typename T>
    void MatrixAccessor<T>::assertMatrixShape(const Matrix& m) const {
      // complain on empty shapes
      if (m.size() == 0) {
        throw cms::Exception("InvalidShape") << "matrix accessor requires matrix not to be empty";
      }

      // compare the first dimension size when not batched
      if (!this->isBatched() && m.size() != (size_t)this->container_.getDimSize(0)) {
        throw cms::Exception("DimensionMismatch") << "matrix accessor expects matrix dimension 0 to have size "
                                                  << this->container_.getDimSize(0) << " but got " << m.size();
      }

      // check size of each vector individually
      for (size_t i = 0; i < m.size(); i++) {
        // complain on empty dimension
        if (m[i].size() == 0) {
          throw cms::Exception("InvalidDimension")
              << "matrix accessor expects vector " << i << " in dimension 1 not to be empty";
        }

        // check dimension size
        if (m[i].size() != (size_t)this->container_.getDimSize(1)) {
          throw cms::Exception("DimensionMismatch")
              << "matrix accessor expects vector " << i << " in dimension 1 to have size "
              << this->container_.getDimSize(1) << " but got " << m[i].size();
        }
      }
    }

    template <typename T>
    typename MatrixAccessor<T>::Matrix MatrixAccessor<T>::createMatrix(size_t batchSize) {
      // infer the shape from the container
      Shape shape = this->container_.getExpandedShape(batchSize);

      // create a matrix with that shape
      Matrix m;
      for (size_t i = 0; i < shape[0]; i++) {
        m.emplace_back(shape[1]);
      }

      // assume this is our new data pointer, which remains valid outside this method via RVO
      setData(m);

      return m;
    }

    template <typename T>
    void MatrixAccessor<T>::setData(Matrix& m) {
      assertMatrixShape(m);
      this->data_ = &m[0][0];
    }

    template <typename T>
    void MatrixAccessor<T>::setData(Matrix& m, size_t i) {
      assertMatrixShape(m);
      Shape shape = this->container_.getExpandedShape(m.size());
      this->data_ = &m[0][0] + calculateDataOffset(shape, i);
    }

    template <typename T>
    void MatrixAccessor<T>::setData(Matrix& m, size_t i, size_t j) {
      assertMatrixShape(m);
      Shape shape = this->container_.getExpandedShape(m.size());
      this->data_ = &m[0][0] + calculateDataOffset(shape, i, j);
    }

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_MATRIXACCESSOR_H_

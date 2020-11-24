/*
 * Accessor for filling data into and reading data from TensorFlow objects.
 */

#ifndef ML_MODELTOOLS_INTERFACE_TFTENSORACCESSOR_H_
#define ML_MODELTOOLS_INTERFACE_TFTENSORACCESSOR_H_

#include "common.h"
#include "DataAccessor.h"

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"

namespace edm {

  namespace ml {

    template <typename T>
    class TFTensorAccessor : public DataAccessor<T> {
    public:
      using DataAccessor<T>::DataAccessor;

      template <typename... Args>
      TFTensorAccessor(const FeatureContainer<T>& container, tensorflow::Tensor& t, Args&&... args)
          : DataAccessor<T>(container) {
        setData(t, std::forward<Args>(args)...);
      }

      static tensorflow::DataType inferTensorType();

      void assertTensorType(const tensorflow::Tensor& t) const;

      void assertTensorShape(const tensorflow::Tensor& t) const;

      tensorflow::Tensor createTensor(size_t batchSize = 1);

      using DataAccessor<T>::setData;

      void setData(tensorflow::Tensor& t);

      void setData(tensorflow::Tensor& t, size_t i);

      void setData(tensorflow::Tensor& t, size_t i, size_t j);

      void setData(tensorflow::Tensor& t, size_t i, size_t j, size_t k);
    };

    template <typename T>
    tensorflow::DataType TFTensorAccessor<T>::inferTensorType() {
      if constexpr (std::is_same<T, float>::value) {
        return tensorflow::DT_FLOAT;
      } else if constexpr (std::is_same<T, double>::value) {
        return tensorflow::DT_DOUBLE;
      } else if constexpr (std::is_same<T, int32_t>::value) {
        return tensorflow::DT_INT32;
      } else if constexpr (std::is_same<T, int64_t>::value) {
        return tensorflow::DT_INT64;
      } else if constexpr (std::is_same<T, bool>::value) {
        return tensorflow::DT_BOOL;
      } else if constexpr (std::is_same<T, std::string>::value) {
        return tensorflow::DT_STRING;
      } else {
        throw cms::Exception("UnsupportedType")
            << "no tensorflow data type can be inferred from template type '" << typeid(T).name() << "'";
      }
    }

    template <typename T>
    void TFTensorAccessor<T>::assertTensorType(const tensorflow::Tensor& t) const {
      if (t.dtype() != inferTensorType()) {
        throw cms::Exception("TypeMismatch")
            << "tensorflow tensor accessor expects tensor with type " << tensorflow::DataTypeString(inferTensorType())
            << " but got " << tensorflow::DataTypeString(t.dtype());
      }
    }

    template <typename T>
    void TFTensorAccessor<T>::assertTensorShape(const tensorflow::Tensor& t) const {
      // complain on empty shapes
      if (t.dims() == 0) {
        throw cms::Exception("InvalidShape") << "tensorflow tensor accessor expects tensor with at least 1 dimension";
      }

      // check number of dimensions
      size_t nDims = this->container_.getNDims();
      if ((size_t)t.dims() != nDims) {
        throw cms::Exception("ShapeMismatch")
            << "tensorflow tensor accessor expects tensor with " << nDims << " dimensions but got " << t.dims();
      }

      // check size of each dimension individually
      for (size_t d = 0; d < nDims; d++) {
        auto tDimSize = t.dim_size(d);

        // complain on negative dimension
        if (tDimSize < 0) {
          throw cms::Exception("InvalidDimension") << "tensorflow tensor accessor expects tensor dimension " << d
                                                   << " to be non-negative but got " << tDimSize;
        }

        // complain on empty dimension
        if (tDimSize == 0) {
          throw cms::Exception("InvalidDimension")
              << "tensorflow tensor accessor expects tensor dimension " << d << " not to be empty";
        }

        // check dimension size
        bool isBatchDim = d == 0 && this->isBatched();
        if (!isBatchDim && this->container_.getDimSize(d) != tDimSize) {
          throw cms::Exception("DimensionMismatch")
              << "tensorflow tensor accessor expects tensor dimension " << d << " to have size "
              << this->container_.getDimSize(d) << " but got " << tDimSize;
        }
      }
    }

    template <typename T>
    tensorflow::Tensor TFTensorAccessor<T>::createTensor(size_t batchSize) {
      // determine the dtype based on the container type
      tensorflow::DataType dtype(inferTensorType());

      // build the shape
      tensorflow::TensorShape shape;
      for (const auto& d : this->container_.getExpandedShape(batchSize)) {
        shape.AddDim(d);
      }

      // create the tensor
      tensorflow::Tensor t(dtype, shape);

      // assume this is our new data pointer, which remains valid outside this method via RVO
      setData(t);

      return t;
    }

    template <typename T>
    void TFTensorAccessor<T>::setData(tensorflow::Tensor& t) {
      assertTensorType(t);
      assertTensorShape(t);
      T* data = t.flat<T>().data();
      this->data_ = data;
    }

    template <typename T>
    void TFTensorAccessor<T>::setData(tensorflow::Tensor& t, size_t i) {
      assertTensorType(t);
      assertTensorShape(t);
      Shape shape = this->container_.getExpandedShape((size_t)t.dim_size(0));
      T* data = t.flat<T>().data();
      this->data_ = data + calculateDataOffset(shape, i);
    }

    template <typename T>
    void TFTensorAccessor<T>::setData(tensorflow::Tensor& t, size_t i, size_t j) {
      assertTensorType(t);
      assertTensorShape(t);
      Shape shape = this->container_.getExpandedShape((size_t)t.dim_size(0));
      T* data = t.flat<T>().data();
      this->data_ = data + calculateDataOffset(shape, i, j);
    }

    template <typename T>
    void TFTensorAccessor<T>::setData(tensorflow::Tensor& t, size_t i, size_t j, size_t k) {
      assertTensorType(t);
      assertTensorShape(t);
      Shape shape = this->container_.getExpandedShape((size_t)t.dim_size(0));
      T* data = t.flat<T>().data();
      this->data_ = data + calculateDataOffset(shape, i, j, k);
    }

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_TFTENSORACCESSOR_H_

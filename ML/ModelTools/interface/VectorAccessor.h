/*
 * Accessor for filling data into and reading data from vectors.
 */

#ifndef ML_MODELTOOLS_INTERFACE_VECTORACCESSOR_H_
#define ML_MODELTOOLS_INTERFACE_VECTORACCESSOR_H_

#include "common.h"
#include "DataAccessor.h"

namespace edm {

  namespace ml {

    template <typename T>
    class VectorAccessor : public DataAccessor<T> {
    public:
      typedef std::vector<T> Vector;

      VectorAccessor(const FeatureContainer<T>& container) : DataAccessor<T>(container) { checkContainer(); }

      template <typename... Args>
      VectorAccessor(const FeatureContainer<T>& container, Vector& v, Args&&... args) : VectorAccessor(container) {
        setData(v, std::forward<Args>(args)...);
      }

      VectorAccessor(const VectorAccessor& d) : VectorAccessor(d.container_, d.data_) {}

      VectorAccessor() = delete;

      void assertVectorShape(const Vector& v) const;

      Vector createVector(size_t batchSize = 1);

      void setData(Vector& v);

      void setData(Vector& v, size_t i);

    protected:
      virtual const void checkContainer() const override;
    };

    template <typename T>
    const void VectorAccessor<T>::checkContainer() const {
      if (this->container_.getNDims() != 1) {
        throw cms::Exception("DimensionMismatch")
            << "vector accessor requires container to have 1 dimension but got " << this->container_.getNDims();
      }
    }

    template <typename T>
    void VectorAccessor<T>::assertVectorShape(const Vector& v) const {
      // complain on empty vector
      if (v.size() == 0) {
        throw cms::Exception("InvalidShape") << "vector accessor requires vector not to be empty";
      }

      // compare the first dimension size when not batched
      if (!this->isBatched() && v.size() != (size_t)this->container_.getDimSize(0)) {
        throw cms::Exception("DimensionMismatch") << "vector accessor expects vector to have size "
                                                  << this->container_.getDimSize(0) << " but got " << v.size();
      }
    }

    template <typename T>
    typename VectorAccessor<T>::Vector VectorAccessor<T>::createVector(size_t batchSize) {
      // infer size of dimension 0 from the container
      size_t ds0 = this->isBatched() ? batchSize : this->container_.getDimSize(0);

      // create a vector with that size
      Vector vec(ds0);

      // assume this is our new data pointer, which remains valid outside this method via RVO
      setData(vec);

      return vec;
    }

    template <typename T>
    void VectorAccessor<T>::setData(Vector& v) {
      assertVectorShape(v);
      this->data_ = &v[0];
    }

    template <typename T>
    void VectorAccessor<T>::setData(Vector& v, size_t i) {
      assertVectorShape(v);
      this->data_ = &v[0] + calculateDataOffset({v.size()}, i);
    }

    // template <typename T>
    // void VectorAccessor<T>::resetValues(size_t batchSize) const {
    //   // the strategy here is to loop only once over each features and set its default value
    //   // batchSize times to the proper data pointer, so this number becomes 0 without batching
    //   size_t batchSize = this->isBatched() ? batchSize : 0;
    //   size_t nFeatures = this->container_.getNFeatures();

    //   // loop over features
    //   for (const auto [_, feature] : this->container_.getFeatures()) {
    //     // skip when there is no default at all
    //     if (!feature.hasDefaultValue()) {
    //       continue;
    //     }
    //     const auto& defaultValue = feature.getDefaultValue();

    //     // set it batchSize times with the proper offset
    //     for (size_t b = 0; b < batchSize; b++) {
    //       feature.setValue(this->data_ + b * nFeatures, defaultValue, false);
    //     }
    //   }
    // }

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_VECTORACCESSOR_H_

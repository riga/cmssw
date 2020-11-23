/*
 * Data accessor for filling input and reading output data.
 */

#ifndef ML_MODELTOOLS_INTERFACE_DATAACCESSOR_H_
#define ML_MODELTOOLS_INTERFACE_DATAACCESSOR_H_

#include "common.h"
#include "Feature.h"

namespace edm {

  namespace ml {

    template <typename T>
    class FeatureContainer;

    template <typename T>
    class DataAccessor {
    public:
      DataAccessor(const FeatureContainer<T>& container)
          : container_(container), data_(nullptr), batched_(container_.isBatched()) {
        checkContainer();
      }

      DataAccessor(const FeatureContainer<T>& container, T* data)
          : container_(container), data_(data), batched_(container_.isBatched()) {
        checkContainer();
      }

      DataAccessor(const DataAccessor& d) : DataAccessor(d.container_, d.data_) {}

      DataAccessor() = delete;

      virtual ~DataAccessor() { data_ = nullptr; }

      T* getData() const { return data_; }

      void setData(T* data) { data_ = data; }

      bool hasData() const { return data_ != nullptr; }

      bool isBatched() const { return batched_; }

      void setValue(const Feature<T>& feature, const T& value, size_t batchIndex, bool check = true) const;

      void setValue(const Feature<T>& feature, const T& value) const { setValue(feature, value, 0); };

      template <typename... Args>
      void setValue(const std::string& featureName, Args&&... args) const {
        setValue(container_.getFeature(featureName), std::forward<Args>(args)...);
      };

      const T& getValue(const Feature<T>& feature, size_t batchIndex, bool check = false) const;

      const T& getValue(const Feature<T>& feature) const { return getValue(feature, 0); }

      template <typename... Args>
      const T& getValue(const std::string& featureName, Args&&... args) const {
        return getValue(container_.getFeature(featureName), std::forward<Args>(args)...);
      }

      void resetValues(size_t batchSize = 1) const;

    protected:
      const FeatureContainer<T>& container_;
      T* data_;
      bool batched_;

      void assertData() const;

      virtual void checkContainer() const;
    };

    template <typename T>
    void DataAccessor<T>::assertData() const {
      if (!hasData()) {
        throw cms::Exception("UndefinedDataPointer")
            << "cannot access data described by container '" << container_.getName() << "' with undefined pointer";
      }
    }

    template <typename T>
    void DataAccessor<T>::checkContainer() const {
      if (container_.getNFeatures() != container_.getNFeaturesMax()) {
        throw cms::Exception("IncompleteFeatureContainer")
            << "feature container '" << container_.getName() << "' is incomplete as it contains only "
            << container_.getNFeatures() << " from " << container_.getNFeaturesMax() << " possible features";
      }
    }

    template <typename T>
    void DataAccessor<T>::setValue(const Feature<T>& feature, const T& value, size_t batchIndex, bool check) const {
      assertData();
      size_t offset = feature.getIndex() + (batchIndex > 0 ? (batchIndex * container_.getNFeatures()) : 0);
      *(data_ + offset) = check ? feature.checkValue(value) : value;
    };

    template <typename T>
    const T& DataAccessor<T>::getValue(const Feature<T>& feature, size_t batchIndex, bool check) const {
      assertData();
      size_t offset = feature.getIndex() + (batchIndex > 0 ? (batchIndex * container_.getNFeatures()) : 0);
      return check ? feature.checkValue(*(data_ + offset)) : *(data_ + offset);
    }

    template <typename T>
    void DataAccessor<T>::resetValues(size_t batchSize) const {
      // the strategy here is to loop only once over each features and set its default value
      // batchSize times to the proper data pointer, so this number becomes 0 without batching
      assertData();

      // get the proper batchSize value and the number of features to calculate offsets
      batchSize = isBatched() ? batchSize : 0;
      size_t nFeatures = container_.getNFeatures();

      // loop over features
      for (const auto [_, feature] : container_.getFeatures()) {
        // skip when there is no default at all
        if (!feature.hasDefaultValue()) {
          continue;
        }
        const auto& defaultValue = feature.getDefaultValue();

        // set it batchSize times with the proper offset
        size_t offset = feature.getIndex();
        for (size_t b = 0; b < batchSize; b++) {
          *(data_ + offset) = defaultValue;
          offset += nFeatures;
        }
      }
    }

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_DATAACCESSOR_H_

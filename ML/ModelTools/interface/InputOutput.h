/*
 * Generic feature container (e.g. tensors or arrays) for ML purposes.
 */

#ifndef ML_MODELTOOLS_INTERFACE_INPUTOUTPUT_H_
#define ML_MODELTOOLS_INTERFACE_INPUTOUTPUT_H_

#include "common.h"
#include "Feature.h"
#include "DataAccessor.h"

namespace edm {

  namespace ml {

    template <typename T>
    class FeatureContainer {
    public:
      FeatureContainer(const std::string& name, const BatchableShape& shape, const std::string& resource = "")
          : name_(name),
            shape_(validateBatchableShape(shape)),
            resource_(resource.empty() ? name_ : resource),
            nFeaturesMax_(calculateMaxFeatures(shape_)) {}

      FeatureContainer(const FeatureContainer<T>& c) : FeatureContainer<T>(c.name_, c.shape_, c.resource_) {}

      FeatureContainer() = delete;

      virtual ~FeatureContainer() {}

      static BatchableShape validateBatchableShape(const BatchableShape& shape);

      const std::string& getName() const { return name_; }

      const BatchableShape& getShape() const { return shape_; }

      Shape getExpandedShape(size_t batchSize = 1) const;

      const std::string& getResource() const { return resource_; }

      bool isBatched() const { return getNDims() >= 1 and shape_[0] < 0; }

      size_t getNDims() const { return shape_.size(); }

      int getDimSize(size_t i) const { return shape_.at(i); }

      bool hasFeature(const std::string& name) const { return features_.find(name) != features_.end(); }

      template <typename... Args>
      Feature<T>& addFeature(const std::string& name, Args&&... args);

      const std::map<std::string, Feature<T>>& getFeatures() const { return features_; }

      std::map<std::string, Feature<T>>& getFeatures() { return features_; }

      const Feature<T>& getFeature(const std::string& name) const;

      Feature<T>& getFeature(const std::string& name) {
        return const_cast<Feature<T>&>(std::as_const(*this).getFeature(name));
      }

      size_t getNFeatures() const { return features_.size(); }

      size_t getNFeaturesMax() const { return nFeaturesMax_; }

      template <template <typename> class D = DataAccessor, typename... Args>
      D<T> createDataAccessor(Args&&... args) const {
        return D<T>(*this, std::forward<Args>(args)...);
      }

    protected:
      const std::string name_;
      const BatchableShape shape_;
      const std::string resource_;
      const size_t nFeaturesMax_;
      std::map<std::string, Feature<T>> features_;
    };

    template <typename T>
    BatchableShape FeatureContainer<T>::validateBatchableShape(const BatchableShape& shape) {
      BatchableShape fixedShape(shape.size());
      for (size_t d = 0; d < shape.size(); d++) {
        if (shape[d] == 0) {
          throw cms::Exception("InvalidShapeDimension") << "batchable shape dimension " << d << " must not be 0";
        } else if (shape[d] < 0) {
          if (d > 0) {
            throw cms::Exception("InvalidShapeDimension")
                << "negative dimension size is not valid for dimension " << d << " but only for dimension 0";
          }
          fixedShape[d] = -1;
        } else {
          fixedShape[d] = shape[d];
        }
      }
      return fixedShape;
    }

    template <typename T>
    Shape FeatureContainer<T>::getExpandedShape(size_t batchSize) const {
      Shape shape(getNDims());
      for (size_t d = 0; d < getNDims(); d++) {
        shape[d] = (d == 0 && getDimSize(d) < 0) ? batchSize : (size_t)getDimSize(d);
      }
      return shape;
    }

    template <typename T>
    template <typename... Args>
    Feature<T>& FeatureContainer<T>::addFeature(const std::string& name, Args&&... args) {
      // ensure that the number of features does not exceed the maximum according to the shape
      size_t index = getNFeatures();
      if (index >= getNFeaturesMax()) {
        throw cms::Exception("MaximumFeaturesReached")
            << "container '" << getName() << "' already reached maximum amount of " << getNFeaturesMax()
            << " features, cannot add feature '" << name << "'";
      }

      // check if there is already a feature with that name
      if (hasFeature(name)) {
        throw cms::Exception("DuplicateFeatureName")
            << "feature '" << name << "' already registered in container '" << getName() << "'";
      }

      // add the new feature and return a reference
      auto ret = features_.emplace(name, Feature<T>(name, index, std::forward<Args>(args)...));
      return ret.first->second;
    }

    template <typename T>
    const Feature<T>& FeatureContainer<T>::getFeature(const std::string& name) const {
      auto it = features_.find(name);
      if (it == features_.end()) {
        throw cms::Exception("UnknownFeature")
            << "no feature '" << name << "' registered in container '" << getName() << "'";
      }

      return it->second;
    }

    template <typename T>
    class Input : public FeatureContainer<T> {
    public:
      using FeatureContainer<T>::FeatureContainer;
    };

    template <typename T>
    class Output : public FeatureContainer<T> {
    public:
      using FeatureContainer<T>::FeatureContainer;
    };

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_INPUTOUTPUT_H_

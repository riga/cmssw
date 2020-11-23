/*
 * Generic feature for ML purposes.
 */

#ifndef ML_MODELTOOLS_INTERFACE_FEATURE_H_
#define ML_MODELTOOLS_INTERFACE_FEATURE_H_

#include "common.h"

namespace edm {

  namespace ml {

    template <typename T>
    class Feature {
    public:
      Feature(const std::string& name, size_t index) : name_(name), index_(index), defaultValueSet_(false) {}

      Feature(const std::string& name, size_t index, const T& defaultValue)
          : name_(name), index_(index), defaultValueSet_(true), defaultValue_(defaultValue) {}

      Feature(const Feature& f) : Feature(f.name_, f.index_, f.defaultValue_) { defaultValueSet_ = f.defaultValueSet_; }

      Feature() = delete;

      virtual ~Feature() {}

      const std::string& getName() const { return name_; }

      size_t getIndex() const { return index_; }

      const T& getDefaultValue() const { return defaultValue_; }

      void setDefaultValue(const T& value) {
        defaultValueSet_ = true;
        defaultValue_ = value;
      }

      bool hasDefaultValue() const { return defaultValueSet_; }

      const std::set<T>& getEmptyValues() const { return emptyValues_; }

      bool hasEmptyValues() const { return emptyValues_.size() > 0; }

      bool hasEmptyValue(const T& value) const {
        return hasEmptyValues() && emptyValues_.find(value) != emptyValues_.end();
      }

      void addEmptyValue(const T& value) { emptyValues_.insert(value); }

      const T& checkValue(const T& value) const {
        return (hasDefaultValue() && hasEmptyValue(value)) ? getDefaultValue() : value;
      }

    protected:
      const std::string name_;
      const size_t index_;
      bool defaultValueSet_;
      T defaultValue_;
      std::set<T> emptyValues_;
    };

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_FEATURE_H_

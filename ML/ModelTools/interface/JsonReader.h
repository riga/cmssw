/*
 * Json reader to deserialize models from files.
 */

#ifndef ML_MODELTOOLS_INTERFACE_JSONREADER_H_
#define ML_MODELTOOLS_INTERFACE_JSONREADER_H_

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "common.h"
#include "Model.h"
#include "Feature.h"

namespace edm {

  namespace ml {

    class JsonReader {
    public:
      JsonReader(Model& model, const std::string& fileName) : model_(model), fileName_(fileName) { read(); }

      JsonReader(const JsonReader& r) = delete;

      JsonReader() = delete;

      virtual ~JsonReader() {}

    protected:
      Model& model_;
      const std::string fileName_;

      void throwInvalidModelFile(const std::string& msg) const {
        throw cms::Exception("InvalidModelFile") << "model file '" << fileName_ << "' is invalid, " << msg;
      }

      void requireField(const json& j, const std::string& field, const std::string& parent) const {
        if (!j.contains(field)) {
          throwInvalidModelFile("field '" + field + "' is required but missing in " + parent);
        }
      }

      void read();

      void readV1(json& j);

      template <typename T>
      void readInputV1(json& j);

      template <typename T>
      void readOutputV1(json& j);

      template <typename T>
      void readFeatureV1(json& j, FeatureContainer<T>& container);
    };

    template <typename T>
    void JsonReader::readInputV1(json& j) {
      // require mandatory fields
      requireField(j, "name", "input");
      requireField(j, "shape", "input");
      requireField(j, "features", "input");

      // create the input container
      auto& input = model_.addInput<T>(j["name"], j["shape"], (j.contains("resource") ? j["resource"] : j["name"]));

      // read features
      for (auto& jFeature : j["features"]) {
        readFeatureV1(jFeature, input);
      }
    }

    template <typename T>
    void JsonReader::readOutputV1(json& j) {
      // require mandatory fields
      requireField(j, "name", "output");
      requireField(j, "shape", "output");
      requireField(j, "features", "output");

      // create the output container
      auto& output = model_.addOutput<T>(j["name"], j["shape"], (j.contains("resource") ? j["resource"] : j["name"]));

      // read features
      for (auto& jFeature : j["features"]) {
        readFeatureV1(jFeature, output);
      }
    }

    template <typename T>
    void JsonReader::readFeatureV1(json& j, FeatureContainer<T>& container) {
      // require mandatory fields
      requireField(j, "name", "feature of " + container.getName());

      // create the feature
      auto& feature = container.addFeature(j["name"]);

      // optional configuration
      if (j.contains("default")) {
        // replace "nan"
        if (j["default"] == "nan") {
          j["default"] = nanf;
        }
        feature.setDefaultValue(j["default"]);
      }
      if (j.contains("empty")) {
        for (auto& jValue : j["empty"]) {
          // replace "nan"
          if (jValue == "nan") {
            jValue = nanf;
          }
          feature.addEmptyValue(jValue);
        }
      }
    }

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_JSONREADER_H_

/*
 * Generic model description for ML purposes.
 * See the header file for documentation.
 */

#include "../interface/Model.h"
#include "../interface/JsonReader.h"

namespace edm {

  namespace ml {

    Model::Model(const std::string& name, const std::string& fileName) : name_(name) {
      if (!fileName.empty()) {
        loadJson(fileName);
      }
    }

    void Model::loadJson(const std::string& fileName) { JsonReader(*this, fileName); }

  }  // namespace ml

}  // namespace edm

/*
 * Json reader to deserialize models from files.
 * See the header file for documentation.
 */

#include "../interface/JsonReader.h"

namespace edm {

  namespace ml {

    void JsonReader::read() {
      // read the file into a json object
      json j;
      std::ifstream f(fileName_);
      f >> j;

      // get the syntax version
      std::string syntax = "v1";
      if (j.contains("syntax")) {
        syntax = j["syntax"];
      }

      // continue reading depending on the syntax version
      if (syntax == "v1") {
        readV1(j);
      } else {
        throw cms::Exception("UnknownModelFileSyntax") << "syntax version '" << syntax << "' is not supported";
      }
    }

    void JsonReader::readV1(json& j) {
      // require mandatory fields
      requireField(j, "inputs", "global scope");
      requireField(j, "outputs", "global scope");

      // read inputs
      for (auto& jInput : j["inputs"]) {
        // get the type of forward to the appropriate input reading function
        requireField(jInput, "type", "input");
        if (jInput["type"] == "float") {
          readInputV1<float>(jInput);
        } else if (jInput["type"] == "double") {
          readInputV1<double>(jInput);
        } else if (jInput["type"] == "int32") {
          readInputV1<int32_t>(jInput);
        } else if (jInput["type"] == "int64") {
          readInputV1<int64_t>(jInput);
        } else if (jInput["type"] == "bool") {
          readInputV1<bool>(jInput);
        } else if (jInput["type"] == "string") {
          readInputV1<std::string>(jInput);
        } else {
          throw cms::Exception("UnknownInputType") << "input type '" << jInput["type"] << "' is unknown";
        }
      }

      // read outputs
      for (auto& jOutput : j["outputs"]) {
        // get the type of forward to the appropriate output reading function
        requireField(jOutput, "type", "output");
        if (jOutput["type"] == "float") {
          readOutputV1<float>(jOutput);
        } else if (jOutput["type"] == "double") {
          readOutputV1<double>(jOutput);
        } else if (jOutput["type"] == "int32") {
          readOutputV1<int32_t>(jOutput);
        } else if (jOutput["type"] == "int64") {
          readOutputV1<int64_t>(jOutput);
        } else if (jOutput["type"] == "bool") {
          readOutputV1<bool>(jOutput);
        } else if (jOutput["type"] == "string") {
          readOutputV1<std::string>(jOutput);
        } else {
          throw cms::Exception("UnknownOutputType") << "output type '" << jOutput["type"] << "' is unknown";
        }
      }
    }

  }  // namespace ml

}  // namespace edm

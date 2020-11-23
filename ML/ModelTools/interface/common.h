/*
 * Common constants, types and helpers.
 */

#ifndef ML_MODELTOOLS_INTERFACE_COMMON_H_
#define ML_MODELTOOLS_INTERFACE_COMMON_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <variant>
#include <cmath>
#include <iostream>

#include "FWCore/Utilities/interface/Exception.h"

namespace edm {

  namespace ml {

    // float nan constant
    extern const float nanf;

    // double nan constant
    extern const double nand;

    // input container forward declaration
    template <typename T>
    class Input;

    // output container forward declaration
    template <typename T>
    class Output;

    // shape type
    typedef std::vector<size_t> Shape;

    // shape type allowing for nagative values to describe batching
    typedef std::vector<int64_t> BatchableShape;

    // variable input type
    typedef std::variant<Input<float>, Input<double>, Input<int32_t>, Input<int64_t>, Input<bool>, Input<std::string>>
        VarInput;

    // variable output type
    typedef std::
        variant<Output<float>, Output<double>, Output<int32_t>, Output<int64_t>, Output<bool>, Output<std::string>>
            VarOutput;

    // string to VarInput map type
    typedef std::map<std::string, VarInput> InputMap;

    // string to VarOutput map type
    typedef std::map<std::string, VarOutput> OutputMap;

    // returns whether the realized type of a VarInput or VarOutput matches a template type
    template <typename T, typename V>
    bool checkVarContainerType(const V& v) {
      if constexpr (std::is_same<T, float>::value) {
        return v.index() == 0;
      } else if constexpr (std::is_same<T, double>::value) {
        return v.index() == 1;
      } else if constexpr (std::is_same<T, int32_t>::value) {
        return v.index() == 2;
      } else if constexpr (std::is_same<T, int64_t>::value) {
        return v.index() == 3;
      } else if constexpr (std::is_same<T, bool>::value) {
        return v.index() == 4;
      } else if constexpr (std::is_same<T, std::string>::value) {
        return v.index() == 5;
      } else {
        return false;
      }
    }

    // calculates the maximum number of features given a batchable shape, i.e., calculates the
    // product of dimension sizes except for the first one when it is negative
    size_t calculateMaxFeatures(const BatchableShape& shape);

    // calculates the maximum number of features given a shape, i.e., calculates the product of
    // dimension sizes
    size_t calculateMaxFeatures(const Shape& shape);

    // calculates the integer offset relative to the start of a memory-contiguous data structure,
    // such as a vector or a tensor with a certain shape, given a coordinate i on the first axis
    size_t calculateDataOffset(const Shape& shape, size_t i);

    // calculates the integer offset relative to the start of a memory-contiguous data structure,
    // such as a vector or a tensor with a certain shape, given a coordinates i and j on the first
    // two axes
    size_t calculateDataOffset(const Shape& shape, size_t i, size_t j);

    // calculates the integer offset relative to the start of a memory-contiguous data structure,
    // such as a vector or a tensor with a certain shape, given a coordinates i, j and k on the
    // first three axes
    size_t calculateDataOffset(const Shape& shape, size_t i, size_t j, size_t k);

  }  // namespace ml

}  // namespace edm

#endif  // ML_MODELTOOLS_INTERFACE_COMMON_H_

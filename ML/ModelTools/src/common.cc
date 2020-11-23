/*
 * Common constants, types and helpers.
 * See the header file for documentation.
 */

#include "../interface/common.h"

namespace edm {

  namespace ml {

    const float nanf = std::nanf("");

    const double nand = std::nan("");

    size_t calculateMaxFeatures(const BatchableShape& shape) {
      size_t n = 1;
      for (size_t d = 0; d < shape.size(); d++) {
        if (shape[d] < 0) {
          if (d > 0) {
            throw cms::Exception("InvalidShapeDimension")
                << "negative dimension size is not valid for dimension " << d << " but only for dimension 0";
          }
          continue;
        }
        n *= shape[d];
      }
      return n;
    }

    size_t calculateMaxFeatures(const Shape& shape) {
      size_t n = 1;
      for (size_t d = 0; d < shape.size(); d++) {
        n *= shape[d];
      }
      return n;
    }

    size_t calculateDataOffset(const Shape& shape, size_t i) {
      if (shape.size() < 1) {
        throw cms::Exception("ShapeMismatch")
            << "cannot calculate offset (" << i << ",) for shape with " << shape.size() << " dimensions";
      }
      if (i >= shape[0]) {
        throw cms::Exception("DimensionOverflow") << i << " exceeds size " << shape[0] << " of shape dimension 0";
      }

      // product over all potential inner dimensions
      size_t innerProduct = 1;
      for (size_t d = 1; d < shape.size(); d++) {
        innerProduct *= shape[d];
      }

      return i * innerProduct;
    }

    size_t calculateDataOffset(const Shape& shape, size_t i, size_t j) {
      if (shape.size() < 2) {
        throw cms::Exception("ShapeMismatch")
            << "cannot calculate offset (" << i << "," << j << ") for shape with " << shape.size() << " dimensions";
      }
      if (i >= shape[0]) {
        throw cms::Exception("DimensionOverflow") << i << " exceeds size " << shape[0] << " of shape dimension 0";
      }
      if (j >= shape[1]) {
        throw cms::Exception("DimensionOverflow") << j << " exceeds size " << shape[1] << " of shape dimension 1";
      }

      // product over all potential inner dimensions
      size_t innerProduct = 1;
      for (size_t d = 2; d < shape.size(); d++) {
        innerProduct *= shape[d];
      }

      return (i * shape[1] + j) * innerProduct;
    }

    size_t calculateDataOffset(const Shape& shape, size_t i, size_t j, size_t k) {
      if (shape.size() < 3) {
        throw cms::Exception("ShapeMismatch") << "cannot calculate offset (" << i << "," << j << "," << k
                                              << ") for shape with " << shape.size() << " dimensions";
      }
      if (i >= shape[0]) {
        throw cms::Exception("DimensionOverflow") << i << " exceeds size " << shape[0] << " of shape dimension 0";
      }
      if (j >= shape[1]) {
        throw cms::Exception("DimensionOverflow") << j << " exceeds size " << shape[1] << " of shape dimension 1";
      }
      if (k >= shape[2]) {
        throw cms::Exception("DimensionOverflow") << k << " exceeds size " << shape[2] << " of shape dimension 2";
      }

      // product over all potential inner dimensions
      size_t innerProduct = 1;
      for (size_t d = 3; d < shape.size(); d++) {
        innerProduct *= shape[d];
      }

      return ((i * shape[1] + j) * shape[2] + k) * innerProduct;
    }

  }  // namespace ml

}  // namespace edm

/*
 * Tests showing some usage examples for the ML helper classes.
 */

#include "testBase.h"

#include "ML/ModelTools/interface/Model.h"
#include "ML/ModelTools/interface/MatrixAccessor.h"
#include "ML/ModelTools/interface/TFTensorAccessor.h"

using namespace edm;

class TestExamples : public TestBase {
  CPPUNIT_TEST_SUITE(TestExamples);
  CPPUNIT_TEST(testManualSetup);
  CPPUNIT_TEST(testJsonLoading);
  CPPUNIT_TEST(testMatrixAccessor);
  CPPUNIT_TEST(testTFTensorAccessor);

  CPPUNIT_TEST_SUITE_END();

public:
  using TestBase::TestBase;

  void testManualSetup();
  void testJsonLoading();
  void testMatrixAccessor();
  void testTFTensorAccessor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestExamples);

void TestExamples::testManualSetup() {
  logHeader("ManualSetup");

  // define a static model description first

  // create a model
  ml::Model model("myModel");

  // define an input with room for three features (leading -1 denotes batching)
  ml::Input<float>& input = model.addInput<float>("modelInput", {-1, 3});

  // add features with default values in the exact same order the model was trained with
  input.addFeature("eta", -5);
  input.addFeature("phi", -3.5);
  ml::Feature<float>& ptFeature = input.addFeature("pt", -1);

  // add so-called empty values to the pt feature, i.e., float values interpreted as missing values
  // leading to the default value instead (-1000 and nan in this example)
  ptFeature.addEmptyValue(-1000);
  ptFeature.addEmptyValue(ml::nanf);

  // define an output with 2 features (implementation-wise, input and output features are identical)
  ml::Output<float>& output = model.addOutput<float>("modelInput", {-1, 2});

  // add features to extract them later
  output.addFeature("signal_prob");
  output.addFeature("background_prob");

  // create a data structure and wrap it into a generic DataAccessor to fill it
  std::vector<float> values(3);
  float* data = &values[0];
  ml::DataAccessor<float> inputAccessor = input.createDataAccessor(data);

  // fill values
  inputAccessor.setValue("eta", 2.5f);
  inputAccessor.setValue("phi", 1);     // note the implicit int -> float cast
  inputAccessor.setValue("pt", -1000);  // note the implicit double -> float cast

  // -1000 was defined as an empty value, so we can expect that (data+2) points to the default pt
  std::cout << "pt value: " << inputAccessor.getValue("pt") << std::endl;
  // same as values[2]
  CPPUNIT_ASSERT_EQUAL(ptFeature.getDefaultValue(), inputAccessor.getValue("pt"));
  CPPUNIT_ASSERT_EQUAL(ptFeature.getDefaultValue(), values[2]);

  // now, one typically runs the model inference using the values vector
  // to simulate this behavior here we assume that the inference returns another float vector
  // but, in fact, this is fully arbitrary as long as the data structure is contiguous in memory
  std::vector<std::vector<float>> outValues = {{0.7, 0.3}};
  float* outData = &outValues[0][0];

  // read particular features
  ml::DataAccessor<float> outputAccessor = output.createDataAccessor(outData);
  std::cout << "signal prob.: " << outputAccessor.getValue("signal_prob") << std::endl;
  std::cout << "background prob.: " << outputAccessor.getValue("background_prob") << std::endl;
  CPPUNIT_ASSERT_EQUAL(outValues[0][0], outputAccessor.getValue("signal_prob"));
  CPPUNIT_ASSERT_EQUAL(outValues[0][1], outputAccessor.getValue("background_prob"));
}

void TestExamples::testJsonLoading() {
  logHeader("JsonLoading");

  // this test reads a json file containing the description of a model that is identical to the one
  // defined in testManualSetup does

  // read it
  std::cout << "reading model from " << modelFileGeneric_ << std::endl;
  ml::Model model("myModel", modelFileGeneric_);

  // show inputs
  std::cout << "inputs: " << model.getNInputs() << std::endl;
  for (ml::InputMap::const_iterator it = model.beginInputs(); it != model.endInputs(); it++) {
    const auto& input = std::get<ml::Input<float>>(it->second);
    std::cout << "  name: " << input.getName();
    std::cout << ", resource: " << input.getResource();
    std::cout << ", features: " << input.getNFeatures();
    std::cout << std::endl;
    for (const auto& [_, feature] : input.getFeatures()) {
      std::cout << "    name: " << feature.getName();
      std::cout << ", default: " << (feature.hasDefaultValue() ? std::to_string(feature.getDefaultValue()) : "not set");
      std::cout << ", empty values: " << feature.getEmptyValues().size();
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  // show outputs
  std::cout << "outputs: " << model.getNOutputs() << std::endl;
  for (ml::OutputMap::const_iterator it = model.beginOutputs(); it != model.endOutputs(); it++) {
    const auto& otuput = std::get<ml::Output<float>>(it->second);
    std::cout << "  name: " << otuput.getName();
    std::cout << ", resource: " << otuput.getResource();
    std::cout << ", features: " << otuput.getNFeatures();
    std::cout << std::endl;
    for (const auto& [_, feature] : otuput.getFeatures()) {
      std::cout << "    name: " << feature.getName();
      std::cout << ", default: " << (feature.hasDefaultValue() ? std::to_string(feature.getDefaultValue()) : "not set");
      std::cout << ", empty values: " << feature.getEmptyValues().size();
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

void TestExamples::testMatrixAccessor() {
  logHeader("MatrixAccessor");

  // this test does exactly what testManualSetup does, but uses MatrixAccessor's
  // to simplify the data access (here, a "Matrix" is a vector of vectors as e.g. used by ONNX)

  // create the model, inputs and outputs
  ml::Model model("myModel");
  auto& input = model.addInput<float>("modelInput", {-1, 3});
  input.addFeature("eta", -5);
  input.addFeature("phi", -3.5);
  auto& ptFeature = input.addFeature("pt", -1);
  ptFeature.addEmptyValue(-1000);
  ptFeature.addEmptyValue(ml::nanf);
  auto& output = model.addOutput<float>("modelInput", {-1, 2});
  output.addFeature("signal_prob");
  output.addFeature("background_prob");

  // create a data accessor which does type and shape checks when passed data structures to wrap
  auto inputAccessor = input.createDataAccessor<ml::MatrixAccessor>();

  // we could create a vector of float vectors of length 3 now (matching the input shape) and pass
  // it to the accessor via setData() (which would do shape checks), but we can also just let the
  // accessor do that with batch size 1
  auto values = inputAccessor.createMatrix(1);
  std::cout << "values dim size 0: " << values.size() << std::endl;
  std::cout << "values dim size 1: " << values[0].size() << std::endl;
  CPPUNIT_ASSERT_EQUAL((size_t)1, values.size());
  CPPUNIT_ASSERT_EQUAL((size_t)3, values[0].size());

  // reset all values to the feature defaults
  inputAccessor.resetValues();
  CPPUNIT_ASSERT_EQUAL(-5.f, values[0][0]);
  CPPUNIT_ASSERT_EQUAL(-3.5f, values[0][1]);
  CPPUNIT_ASSERT_EQUAL(-1.f, values[0][2]);

  // simulate the model inference as above
  std::vector<std::vector<float>> outValues = {{0.7, 0.3}};

  // create an output accessor
  auto outputAccessor = output.createDataAccessor<ml::MatrixAccessor>(outValues);

  // read particular features
  std::cout << "signal prob.: " << outputAccessor.getValue("signal_prob") << std::endl;
  std::cout << "background prob.: " << outputAccessor.getValue("background_prob") << std::endl;
  CPPUNIT_ASSERT_EQUAL(outValues[0][0], outputAccessor.getValue("signal_prob"));
  CPPUNIT_ASSERT_EQUAL(outValues[0][1], outputAccessor.getValue("background_prob"));
}

void TestExamples::testTFTensorAccessor() {
  logHeader("TFTensorAccessor");

  // this test does exactly what testManualSetup does, but uses TFTensorAccessor's
  // to simplify the access of data of a TF tensor

  // create the model, inputs and outputs
  ml::Model model("myModel");
  auto& input = model.addInput<float>("modelInput", {-1, 3});
  input.addFeature("eta", -5);
  input.addFeature("phi", -3.5);
  auto& ptFeature = input.addFeature("pt", -1);
  ptFeature.addEmptyValue(-1000);
  ptFeature.addEmptyValue(ml::nanf);
  auto& output = model.addOutput<float>("modelInput", {-1, 2});
  output.addFeature("signal_prob");
  output.addFeature("background_prob");

  // create a data accessor which does type and shape checks when passed data structures to wrap
  auto inputAccessor = input.createDataAccessor<ml::TFTensorAccessor>();

  // we could create a float tensor with shape {-1, 3} now (matching the input shape) and pass
  // it to the accessor via setData() (which would do shape and type checks), but we can also just
  // let the accessor do that for a batch size of 2
  auto t = inputAccessor.createTensor(2);
  std::cout << t.DebugString(-1) << std::endl;
  CPPUNIT_ASSERT_EQUAL((tensorflow::int64)2, t.dim_size(0));
  CPPUNIT_ASSERT_EQUAL((tensorflow::int64)3, t.dim_size(1));

  // reset all values to the feature defaults
  inputAccessor.resetValues(2);
  std::cout << t.DebugString(-1) << std::endl;
  CPPUNIT_ASSERT_EQUAL(-5.f, inputAccessor.getValue("eta", 0));
  CPPUNIT_ASSERT_EQUAL(-3.5f, inputAccessor.getValue("phi", 0));
  CPPUNIT_ASSERT_EQUAL(-1.f, inputAccessor.getValue("pt", 0));
  CPPUNIT_ASSERT_EQUAL(-5.f, inputAccessor.getValue("eta", 1));
  CPPUNIT_ASSERT_EQUAL(-3.5f, inputAccessor.getValue("phi", 1));
  CPPUNIT_ASSERT_EQUAL(-1.f, inputAccessor.getValue("pt", 1));

  // set the pt value of the second example, i.e., batch index 1
  inputAccessor.setValue("pt", 25.5, 1);
  CPPUNIT_ASSERT_EQUAL(25.5f, inputAccessor.getValue("pt", 1));
}

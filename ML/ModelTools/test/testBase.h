/*
 * Base class for ModelTools tests.
 */

#ifndef ML_MODELTOOLS_TEST_TESTBASE_H_
#define ML_MODELTOOLS_TEST_TESTBASE_H_

#include <boost/filesystem.hpp>
#include <cppunit/extensions/HelperMacros.h>

#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"

class TestBase : public CppUnit::TestFixture {
public:
  TestBase() : modelFileGeneric_(cmsswPath("/src/ML/ModelTools/test/generic_model.json")) {}

  static std::string cmsswPath(std::string path);
  static void logHeader(const std::string& header);

  void setUp() override {}
  void tearDown() override {}

protected:
  const std::string modelFileGeneric_;
};

std::string TestBase::cmsswPath(std::string path) {
  if (path.size() > 0 && path.substr(0, 1) != "/") {
    path = "/" + path;
  }

  std::string base = std::string(std::getenv("CMSSW_BASE"));
  std::string releaseBase = std::string(std::getenv("CMSSW_RELEASE_BASE"));

  return (boost::filesystem::exists(base.c_str()) ? base : releaseBase) + path;
}

void TestBase::logHeader(const std::string& header) {
  std::cout << std::endl << "running test '" << header << "'" << std::endl;
}

#endif  // ML_MODELTOOLS_TEST_TESTBASE_H_

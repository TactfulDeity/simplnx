#include "complex/Core/Parameters/utils/FilePathGenerator.hpp"
#include "complex/Core/Parameters/GeneratedFileListParameter.hpp"

#include "complex/unit_test/complex_test_dirs.h"

#include <catch2/catch.hpp>

#include <fmt/core.h>

using namespace complex;

TEST_CASE("FilePathGenerator")
{
  GeneratedFileListParameter::ValueType value;
  value.startIndex = 1;
  value.endIndex = 2;
  value.incrementIndex = 1;
  value.paddingDigits = 1;
  value.filePrefix = "TestFilter";
  value.fileSuffix = "";
  value.fileExtension = ".cpp";
  value.inputPath = fmt::format("{}/src/complex/Core/Filters/", unit_test::k_ComplexSourceDir.view());
  value.ordering = GeneratedFileListParameter::Ordering::LowToHigh;

  // Generate the file list but do *NOT* validate the paths. this is a test after all
  auto&& [fileList, missingFiles] = FilePathGenerator::GenerateFileList(value.startIndex, value.endIndex, value.incrementIndex, value.ordering, value.inputPath, value.filePrefix, value.fileSuffix,
                                                                        value.fileExtension, value.paddingDigits);
  REQUIRE(fileList.size() == 2);
}

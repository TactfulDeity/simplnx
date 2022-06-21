#pragma once

#include "DREAM3DReview/DREAM3DReview_export.hpp"

#include "complex/DataStructure/DataPath.hpp"
#include "complex/DataStructure/DataStructure.hpp"
#include "complex/Filter/IFilter.hpp"

/**
* This is example code to put in the Execute Method of the filter.
  ImportVolumeGraphicsFileInputValues inputValues;

  inputValues.VGHeaderFile = filterArgs.value<FileSystemPathParameter::ValueType>(k_VGHeaderFile_Key);
  inputValues.DataContainerName = filterArgs.value<StringParameter::ValueType>(k_DataContainerName_Key);
  inputValues.CellAttributeMatrixName = filterArgs.value<DataPath>(k_CellAttributeMatrixName_Key);
  inputValues.DensityArrayName = filterArgs.value<DataPath>(k_DensityArrayName_Key);

  return ImportVolumeGraphicsFile(dataStructure, messageHandler, shouldCancel, &inputValues)();
*/

namespace complex
{

struct DREAM3DREVIEW_EXPORT ImportVolumeGraphicsFileInputValues
{
  FileSystemPathParameter::ValueType VGHeaderFile;
  StringParameter::ValueType DataContainerName;
  DataPath CellAttributeMatrixName;
  DataPath DensityArrayName;

};

/**
 * @class ConditionalSetValue
 * @brief This filter replaces values in the target array with a user specified value
 * where a bool mask array specifies.
 */

class DREAM3DREVIEW_EXPORT ImportVolumeGraphicsFile
{
public:
  ImportVolumeGraphicsFile(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ImportVolumeGraphicsFileInputValues* inputValues);
  ~ImportVolumeGraphicsFile() noexcept;

  ImportVolumeGraphicsFile(const ImportVolumeGraphicsFile&) = delete;
  ImportVolumeGraphicsFile(ImportVolumeGraphicsFile&&) noexcept = delete;
  ImportVolumeGraphicsFile& operator=(const ImportVolumeGraphicsFile&) = delete;
  ImportVolumeGraphicsFile& operator=(ImportVolumeGraphicsFile&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ImportVolumeGraphicsFileInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace complex

#pragma once

#include "SimplnxCore/SimplnxCore_export.hpp"

#include "simplnx/Common/StringLiteral.hpp"
#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
class SIMPLNXCORE_EXPORT CropImageGeometryFilter : public IFilter
{
public:
  CropImageGeometryFilter();
  ~CropImageGeometryFilter() noexcept override;

  CropImageGeometryFilter(const CropImageGeometryFilter&) = delete;
  CropImageGeometryFilter(CropImageGeometryFilter&&) noexcept = delete;

  CropImageGeometryFilter& operator=(const CropImageGeometryFilter&) = delete;
  CropImageGeometryFilter& operator=(CropImageGeometryFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_UsePhysicalBounds_Key = "use_physical_bounds";
  static inline constexpr StringLiteral k_CropXDim_Key = "crop_x_dim";
  static inline constexpr StringLiteral k_CropYDim_Key = "crop_y_dim";
  static inline constexpr StringLiteral k_CropZDim_Key = "crop_z_dim";
  static inline constexpr StringLiteral k_MinVoxel_Key = "min_voxel";
  static inline constexpr StringLiteral k_MaxVoxel_Key = "max_voxel";
  static inline constexpr StringLiteral k_MinCoord_Key = "min_coord";
  static inline constexpr StringLiteral k_MaxCoord_Key = "max_coord";
  // static inline constexpr StringLiteral k_UpdateOrigin_Key = "update_origin";
  static inline constexpr StringLiteral k_SelectedImageGeometryPath_Key = "input_image_geometry_path";
  static inline constexpr StringLiteral k_CreatedImageGeometryPath_Key = "output_image_geometry_path";
  static inline constexpr StringLiteral k_RenumberFeatures_Key = "renumber_features";
  static inline constexpr StringLiteral k_CellFeatureIdsArrayPath_Key = "feature_ids_path";
  static inline constexpr StringLiteral k_FeatureAttributeMatrixPath_Key = "cell_feature_attribute_matrix_path";
  static inline constexpr StringLiteral k_RemoveOriginalGeometry_Key = "remove_original_geometry";

  /**
   * @brief Reads SIMPL json and converts it simplnx Arguments.
   * @param json
   * @return Result<Arguments>
   */
  static Result<Arguments> FromSIMPLJson(const nlohmann::json& json);

  /**
   * @brief
   * @return std::string
   */
  std::string name() const override;

  /**
   * @brief Returns the C++ classname of this filter.
   * @return std::string
   */
  std::string className() const override;

  /**
   * @brief
   * @return Uuid
   */
  Uuid uuid() const override;

  /**
   * @brief
   * @return std::string
   */
  std::string humanName() const override;

  /**
   * @brief Returns the default tags for this filter.
   * @return std::vector<std::string>
   */
  std::vector<std::string> defaultTags() const override;

  /**
   * @brief
   * @return Parameters
   */
  Parameters parameters() const override;

  /**
   * @brief Returns parameters version integer.
   * Initial version should always be 1.
   * Should be incremented everytime the parameters change.
   * @return VersionType
   */
  VersionType parametersVersion() const override;

  /**
   * @brief
   * @return UniquePointer
   */
  UniquePointer clone() const override;

protected:
  /**
   * @brief
   * @param dataStructure
   * @param filterArgs
   * @param messageHandler
   * @param shouldCancel
   * @return PreflightResult
   */
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief
   * @param dataStructure
   * @param args
   * @param pipelineNode
   * @param messageHandler
   * @param shouldCancel
   * @return Result<>
   */
  Result<> executeImpl(DataStructure& dataStructure, const Arguments& args, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                       const std::atomic_bool& shouldCancel) const override;

private:
  int32 m_InstanceId;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, CropImageGeometryFilter, "e6476737-4aa7-48ba-a702-3dfab82c96e2");

#pragma once

#include "SimplnxCore/SimplnxCore_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class ComputeSurfaceFeaturesFilter
 * @brief This Filter determines whether a Feature touches an outer surface of the sample.
 * This is accomplished by simply querying the Feature owners of the Cells that sit at either.
 * Any Feature that owns one of those Cells is said to touch an outer surface and all other
 * Features are said to not touch an outer surface of the sample.
 *
 * This Filter determines whether a Feature touches an outer Surface of the sample volume. A
 * Feature is considered touching the Surface of the sample if either of the following conditions
 * are met:
 * Any cell location is xmin, xmax, ymin, ymax, zmin or zmax.
 * Any cell has Feature ID = 0 as a neighbor.
 *
 * The output of this filter is a Feature level array of booleans where 0=Interior/Not touching
 * and 1=Surface/Touching.
 *
 * Note: If there are voxels within the volume that have Feature ID=0 then any feature touching
 * those voxels will be considered a Surface feature.
 */
class SIMPLNXCORE_EXPORT ComputeSurfaceFeaturesFilter : public IFilter
{
public:
  ComputeSurfaceFeaturesFilter() = default;
  ~ComputeSurfaceFeaturesFilter() noexcept override = default;

  ComputeSurfaceFeaturesFilter(const ComputeSurfaceFeaturesFilter&) = delete;
  ComputeSurfaceFeaturesFilter(ComputeSurfaceFeaturesFilter&&) noexcept = delete;

  ComputeSurfaceFeaturesFilter& operator=(const ComputeSurfaceFeaturesFilter&) = delete;
  ComputeSurfaceFeaturesFilter& operator=(ComputeSurfaceFeaturesFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_MarkFeature0Neighbors = "mark_feature_0_neighbors";
  static inline constexpr StringLiteral k_FeatureGeometryPath_Key = "input_image_geometry_path";
  static inline constexpr StringLiteral k_CellFeatureIdsArrayPath_Key = "feature_ids_path";
  static inline constexpr StringLiteral k_CellFeatureAttributeMatrixPath_Key = "feature_attribute_matrix_path";
  static inline constexpr StringLiteral k_SurfaceFeaturesArrayName_Key = "surface_features_array_name";

  /**
   * @brief Reads SIMPL json and converts it simplnx Arguments.
   * @param json
   * @return Result<Arguments>
   */
  static Result<Arguments> FromSIMPLJson(const nlohmann::json& json);

  /**
   * @brief Returns the name of the filter.
   * @return
   */
  std::string name() const override;

  /**
   * @brief Returns the C++ classname of this filter.
   * @return
   */
  std::string className() const override;

  /**
   * @brief Returns the uuid of the filter.
   * @return
   */
  Uuid uuid() const override;

  /**
   * @brief Returns the human readable name of the filter.
   * @return
   */
  std::string humanName() const override;

  /**
   * @brief Returns the default tags for this filter.
   * @return
   */
  std::vector<std::string> defaultTags() const override;

  /**
   * @brief Returns the parameters of the filter (i.e. its inputs)
   * @return
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
   * @brief Returns a copy of the filter.
   * @return
   */
  UniquePointer clone() const override;

protected:
  /**
   * @brief Takes in a DataStructure and checks that the filter can be run on it with the given arguments.
   * Returns any warnings/errors. Also returns the changes that would be applied to the DataStructure.
   * Some parts of the actions may not be completely filled out if all the required information is not available at preflight time.
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief Applies the filter's algorithm to the DataStructure with the given arguments. Returns any warnings/errors.
   * On failure, there is no guarantee that the DataStructure is in a correct state.
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  Result<> executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                       const std::atomic_bool& shouldCancel) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, ComputeSurfaceFeaturesFilter, "0893e490-5d24-4c21-95e7-e8372baa8948");

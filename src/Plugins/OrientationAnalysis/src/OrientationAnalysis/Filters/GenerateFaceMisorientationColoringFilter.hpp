#pragma once

#include "OrientationAnalysis/OrientationAnalysis_export.hpp"

#include "complex/Filter/FilterTraits.hpp"
#include "complex/Filter/IFilter.hpp"

namespace complex
{
/**
 * @class GenerateFaceMisorientationColoringFilter
 * @brief This filter will generate a 3 component vector for each Triangle in a Triangle Geometry that is the axis-angle of the misorientation associated with the Features that lie on either side of
 * the Triangle. The axis is normalized, so if the magnitude of the vector is viewed, it will be the misorientation angle in degrees.
 */
class ORIENTATIONANALYSIS_EXPORT GenerateFaceMisorientationColoringFilter : public IFilter
{
public:
  GenerateFaceMisorientationColoringFilter() = default;
  ~GenerateFaceMisorientationColoringFilter() noexcept override = default;

  GenerateFaceMisorientationColoringFilter(const GenerateFaceMisorientationColoringFilter&) = delete;
  GenerateFaceMisorientationColoringFilter(GenerateFaceMisorientationColoringFilter&&) noexcept = delete;

  GenerateFaceMisorientationColoringFilter& operator=(const GenerateFaceMisorientationColoringFilter&) = delete;
  GenerateFaceMisorientationColoringFilter& operator=(GenerateFaceMisorientationColoringFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_SurfaceMeshFaceLabelsArrayPath_Key = "surface_mesh_face_labels_array_path";
  static inline constexpr StringLiteral k_AvgQuatsArrayPath_Key = "avg_quats_array_path";
  static inline constexpr StringLiteral k_FeaturePhasesArrayPath_Key = "feature_phases_array_path";
  static inline constexpr StringLiteral k_CrystalStructuresArrayPath_Key = "crystal_structures_array_path";
  static inline constexpr StringLiteral k_SurfaceMeshFaceMisorientationColorsArrayName_Key = "surface_mesh_face_misorientation_colors_array_name";

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
  Result<> executeImpl(DataStructure& data, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;
};
} // namespace complex

COMPLEX_DEF_FILTER_TRAITS(complex, GenerateFaceMisorientationColoringFilter, "f3473af9-db77-43db-bd25-60df7230ea73");
/* LEGACY UUID FOR THIS FILTER 7cd30864-7bcf-5c10-aea7-d107373e2d40 */

#include "FindElementCentroids.hpp"

#include "complex/DataStructure/DataPath.hpp"
#include "complex/Parameters/ArrayCreationParameter.hpp"
#include "complex/Parameters/BoolParameter.hpp"
#include "complex/Parameters/DataGroupCreationParameter.hpp"

using namespace complex;

namespace complex
{
//------------------------------------------------------------------------------
std::string FindElementCentroids::name() const
{
  return FilterTraits<FindElementCentroids>::name.str();
}

//------------------------------------------------------------------------------
std::string FindElementCentroids::className() const
{
  return FilterTraits<FindElementCentroids>::className;
}

//------------------------------------------------------------------------------
Uuid FindElementCentroids::uuid() const
{
  return FilterTraits<FindElementCentroids>::uuid;
}

//------------------------------------------------------------------------------
std::string FindElementCentroids::humanName() const
{
  return "Find Element Centroids";
}

//------------------------------------------------------------------------------
std::vector<std::string> FindElementCentroids::defaultTags() const
{
  return {"#DREAM3D Review", "#Geometry"};
}

//------------------------------------------------------------------------------
Parameters FindElementCentroids::parameters() const
{
  Parameters params;
  // Create the parameter descriptors that are needed for this filter
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_CreateVertexDataContainer_Key, "Create Vertex Geometry from Centroids", "", false));
  params.insert(std::make_unique<DataGroupCreationParameter>(k_NewDataContainerName_Key, "Vertex Data Container", "", DataPath{}));
  params.insert(std::make_unique<ArrayCreationParameter>(k_VertexAttributeMatrixName_Key, "Vertex Attribute Matrix", "", DataPath{}));
  params.insertSeparator(Parameters::Separator{"Element Data"});
  params.insert(std::make_unique<ArrayCreationParameter>(k_CellCentroidsArrayPath_Key, "Element Centroids", "", DataPath{}));
  // Associate the Linkable Parameter(s) to the children parameters that they control
  params.linkParameters(k_CreateVertexDataContainer_Key, k_NewDataContainerName_Key, true);
  params.linkParameters(k_CreateVertexDataContainer_Key, k_VertexAttributeMatrixName_Key, true);

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer FindElementCentroids::clone() const
{
  return std::make_unique<FindElementCentroids>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult FindElementCentroids::preflightImpl(const DataStructure& ds, const Arguments& filterArgs, const MessageHandler& messageHandler) const
{
  /****************************************************************************
   * Write any preflight sanity checking codes in this function
   ***************************************************************************/
  auto pCreateVertexDataContainerValue = filterArgs.value<bool>(k_CreateVertexDataContainer_Key);
  auto pNewDataContainerNameValue = filterArgs.value<DataPath>(k_NewDataContainerName_Key);
  auto pVertexAttributeMatrixNameValue = filterArgs.value<DataPath>(k_VertexAttributeMatrixName_Key);
  auto pCellCentroidsArrayPathValue = filterArgs.value<DataPath>(k_CellCentroidsArrayPath_Key);

  OutputActions actions;
#if 0
  // Define a custom class that generates the changes to the DataStructure.
  auto action = std::make_unique<FindElementCentroidsAction>();
  actions.actions.push_back(std::move(action));
#endif
  return {std::move(actions)};
}

//------------------------------------------------------------------------------
Result<> FindElementCentroids::executeImpl(DataStructure& data, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler) const
{
  /****************************************************************************
   * Extract the actual input values from the 'filterArgs' object
   ***************************************************************************/
  auto pCreateVertexDataContainerValue = filterArgs.value<bool>(k_CreateVertexDataContainer_Key);
  auto pNewDataContainerNameValue = filterArgs.value<DataPath>(k_NewDataContainerName_Key);
  auto pVertexAttributeMatrixNameValue = filterArgs.value<DataPath>(k_VertexAttributeMatrixName_Key);
  auto pCellCentroidsArrayPathValue = filterArgs.value<DataPath>(k_CellCentroidsArrayPath_Key);

  /****************************************************************************
   * Write your algorithm implementation in this function
   ***************************************************************************/

  return {};
}
} // namespace complex

#include "MultiThresholdObjectsFilter.hpp"

#include "simplnx/Common/TypeTraits.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArrayThresholdsParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/DataTypeParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Parameters/NumericTypeParameter.hpp"
#include "simplnx/Utilities/ArrayThreshold.hpp"
#include "simplnx/Utilities/FilterUtilities.hpp"
#include "simplnx/Utilities/SIMPLConversion.hpp"

#include <algorithm>

namespace nx::core
{
namespace
{
template <typename U>
class ThresholdFilterHelper
{
public:
  ThresholdFilterHelper(nx::core::ArrayThreshold::ComparisonType compType, nx::core::ArrayThreshold::ComparisonValue compValue, std::vector<U>& output)
  : m_ComparisonOperator(compType)
  , m_ComparisonValue(compValue)
  , m_Output(output)
  {
  }

  ~ThresholdFilterHelper() = default;

  /**
   * @brief
   */
  template <typename T>
  void filterDataLessThan(const AbstractDataStore<T>& m_Input, T trueValue, T falseValue)
  {
    size_t m_NumValues = m_Input.getNumberOfTuples();
    T value = static_cast<T>(m_ComparisonValue);
    for(size_t i = 0; i < m_NumValues; ++i)
    {
      m_Output[i] = (m_Input[i] < value) ? trueValue : falseValue;
    }
  }

  /**
   * @brief
   */
  template <typename T>
  void filterDataGreaterThan(const AbstractDataStore<T>& m_Input, T trueValue, T falseValue)
  {
    size_t m_NumValues = m_Input.getNumberOfTuples();
    T value = static_cast<T>(m_ComparisonValue);
    for(size_t i = 0; i < m_NumValues; ++i)
    {
      m_Output[i] = (m_Input[i] > value) ? trueValue : falseValue;
    }
  }

  /**
   * @brief
   */
  template <typename T>
  void filterDataEqualTo(const AbstractDataStore<T>& m_Input, T trueValue, T falseValue)
  {
    size_t m_NumValues = m_Input.getNumberOfTuples();
    T value = static_cast<T>(m_ComparisonValue);
    for(size_t i = 0; i < m_NumValues; ++i)
    {
      m_Output[i] = (m_Input[i] == value) ? trueValue : falseValue;
    }
  }

  /**
   * @brief
   */
  template <typename T>
  void filterDataNotEqualTo(const AbstractDataStore<T>& m_Input, T trueValue, T falseValue)
  {
    size_t m_NumValues = m_Input.getNumberOfTuples();
    T value = static_cast<T>(m_ComparisonValue);
    for(size_t i = 0; i < m_NumValues; ++i)
    {
      m_Output[i] = (m_Input[i] != value) ? trueValue : falseValue;
    }
  }

  template <typename Type>
  void filterData(const AbstractDataStore<Type>& input, Type trueValue, Type falseValue)
  {
    if(m_ComparisonOperator == ArrayThreshold::ComparisonType::LessThan)
    {
      filterDataLessThan<Type>(input, trueValue, falseValue);
    }
    else if(m_ComparisonOperator == ArrayThreshold::ComparisonType::GreaterThan)
    {
      filterDataGreaterThan<Type>(input, trueValue, falseValue);
    }
    else if(m_ComparisonOperator == ArrayThreshold::ComparisonType::Operator_Equal)
    {
      filterDataEqualTo<Type>(input, trueValue, falseValue);
    }
    else if(m_ComparisonOperator == ArrayThreshold::ComparisonType::Operator_NotEqual)
    {
      filterDataNotEqualTo<Type>(input, trueValue, falseValue);
    }
    else
    {
      std::string errorMessage = fmt::format("MultiThresholdObjects Comparison Operator not understood: '{}'", static_cast<int>(m_ComparisonOperator));
      throw std::runtime_error(errorMessage);
    }
  }

private:
  nx::core::ArrayThreshold::ComparisonType m_ComparisonOperator;
  nx::core::ArrayThreshold::ComparisonValue m_ComparisonValue;
  std::vector<U>& m_Output;

public:
  ThresholdFilterHelper(const ThresholdFilterHelper&) = delete;            // Copy Constructor Not Implemented
  ThresholdFilterHelper(ThresholdFilterHelper&&) = delete;                 // Move Constructor Not Implemented
  ThresholdFilterHelper& operator=(const ThresholdFilterHelper&) = delete; // Copy Assignment Not Implemented
  ThresholdFilterHelper& operator=(ThresholdFilterHelper&&) = delete;      // Move Assignment Not Implemented
};

struct ExecuteThresholdHelper
{
  template <typename Type, typename MaskType>
  void operator()(ThresholdFilterHelper<MaskType>& helper, const IDataArray* iDataArray, Type trueValue, Type falseValue)
  {
    const auto& dataStore = iDataArray->template getIDataStoreRefAs<AbstractDataStore<Type>>();
    helper.template filterData<Type>(dataStore, trueValue, falseValue);
  }
};

/**
 * @brief InsertThreshold
 * @param numItems
 * @param currentArrayPtr
 * @param unionOperator
 * @param newArrayPtr
 * @param inverse
 */
template <typename T>
void InsertThreshold(usize numItems, AbstractDataStore<T>& currentStore, nx::core::IArrayThreshold::UnionOperator unionOperator, std::vector<T>& newArrayPtr, bool inverse, T trueValue, T falseValue)
{
  for(usize i = 0; i < numItems; i++)
  {
    // invert the current comparison if necessary
    if(inverse)
    {
      newArrayPtr[i] = (newArrayPtr[i] == trueValue) ? falseValue : trueValue;
    }

    if(nx::core::IArrayThreshold::UnionOperator::Or == unionOperator)
    {
      currentStore[i] = (currentStore[i] == trueValue || newArrayPtr[i] == trueValue) ? trueValue : falseValue;
    }
    else if(currentStore[i] == falseValue || newArrayPtr[i] == falseValue)
    {
      currentStore[i] = falseValue;
    }
  }
}

template <typename T>
void ThresholdValue(std::shared_ptr<ArrayThreshold>& comparisonValue, const DataStructure& dataStructure, AbstractDataStore<T>& outputResultStore, int32_t& err, bool replaceInput, bool inverse,
                    T trueValue, T falseValue)
{
  if(nullptr == comparisonValue)
  {
    err = -1;
    return;
  }

  // Get the total number of tuples, create and initialize an array with FALSE to use for these results
  size_t totalTuples = outputResultStore.getNumberOfTuples();
  std::vector<T> tempResultVector(totalTuples, falseValue);

  nx::core::ArrayThreshold::ComparisonType compOperator = comparisonValue->getComparisonType();
  nx::core::ArrayThreshold::ComparisonValue compValue = comparisonValue->getComparisonValue();
  nx::core::IArrayThreshold::UnionOperator unionOperator = comparisonValue->getUnionOperator();

  DataPath inputDataArrayPath = comparisonValue->getArrayPath();

  ThresholdFilterHelper<T> helper(compOperator, compValue, tempResultVector);

  const auto* iDataArray = dataStructure.getDataAs<IDataArray>(inputDataArrayPath);

  ExecuteDataFunction(ExecuteThresholdHelper{}, iDataArray->getDataType(), helper, iDataArray, trueValue, falseValue);

  if(replaceInput)
  {
    if(inverse)
    {
      std::reverse(tempResultVector.begin(), tempResultVector.end());
    }
    // copy the temp uint8 vector to the final uint8 result array
    for(size_t i = 0; i < totalTuples; i++)
    {
      outputResultStore[i] = tempResultVector[i];
    }
  }
  else
  {
    // insert into current threshold
    InsertThreshold<T>(totalTuples, outputResultStore, unionOperator, tempResultVector, inverse, trueValue, falseValue);
  }
}

struct ThresholdValueFunctor
{
  template <typename T>
  void operator()(std::shared_ptr<ArrayThreshold>& comparisonValue, const DataStructure& dataStructure, IDataArray* outputResultArray, int32_t& err, bool replaceInput, bool inverse, T trueValue,
                  T falseValue)
  {
    // Traditionally we would do a check to ensure we get a valid pointer, I'm forgoing that check because it
    // was essentially done in the preflight part.
    ThresholdValue(comparisonValue, dataStructure, outputResultArray->template getIDataStoreRefAs<AbstractDataStore<T>>(), err, replaceInput, inverse, trueValue, falseValue);
  }
};

template <typename T>
void ThresholdSet(std::shared_ptr<ArrayThresholdSet>& inputComparisonSet, const DataStructure& dataStructure, AbstractDataStore<T>& outputResultStore, int32_t& err, bool replaceInput, bool inverse,
                  T trueValue, T falseValue)
{
  if(nullptr == inputComparisonSet)
  {
    return;
  }

  // Get the total number of tuples, create and initialize an array with FALSE to use for these results
  size_t totalTuples = outputResultStore.getNumberOfTuples();
  std::vector<T> tempResultVector(totalTuples, falseValue);

  T firstValueFound = 0;

  ArrayThresholdSet::CollectionType thresholds = inputComparisonSet->getArrayThresholds();
  for(const std::shared_ptr<IArrayThreshold>& threshold : thresholds)
  {
    if(std::dynamic_pointer_cast<ArrayThresholdSet>(threshold))
    {
      std::shared_ptr<ArrayThresholdSet> comparisonSet = std::dynamic_pointer_cast<ArrayThresholdSet>(threshold);
      ThresholdSet<T>(comparisonSet, dataStructure, outputResultStore, err, !firstValueFound, false, trueValue, falseValue);
      firstValueFound = true;
    }
    else if(std::dynamic_pointer_cast<ArrayThreshold>(threshold))
    {
      std::shared_ptr<ArrayThreshold> comparisonValue = std::dynamic_pointer_cast<ArrayThreshold>(threshold);

      ThresholdValue<T>(comparisonValue, dataStructure, outputResultStore, err, !firstValueFound, false, trueValue, falseValue);
      firstValueFound = true;
    }
  }

  if(replaceInput)
  {
    if(inverse)
    {
      std::reverse(tempResultVector.begin(), tempResultVector.end());
    }
    // copy the temp uint8 vector to the final uint8 result array
    for(size_t i = 0; i < totalTuples; i++)
    {
      outputResultStore[i] = tempResultVector[i];
    }
  }
  else
  {
    // insert into current threshold
    InsertThreshold<T>(totalTuples, outputResultStore, inputComparisonSet->getUnionOperator(), tempResultVector, inverse, trueValue, falseValue);
  }
}

struct ThresholdSetFunctor
{
  template <typename T>
  void operator()(std::shared_ptr<ArrayThresholdSet>& inputComparisonSet, const DataStructure& dataStructure, IDataArray* outputResultArray, int32_t& err, bool replaceInput, bool inverse, T trueValue,
                  T falseValue)
  {
    if(outputResultArray == nullptr)
    {
      return;
    }

    // Traditionally we would do a check to ensure we get a valid pointer, I'm forgoing that check because it
    // was essentially done in the preflight part.
    ThresholdSet<T>(inputComparisonSet, dataStructure, outputResultArray->template getIDataStoreRefAs<AbstractDataStore<T>>(), err, replaceInput, inverse, trueValue, falseValue);
  }
};

struct CheckCustomValueInBounds
{
  template <typename T>
  Result<> operator()(float64 customValue)
  {
    float64 minValue;
    float64 maxValue;
    if constexpr(std::is_floating_point_v<T>)
    {
      // Floating Point Types
      minValue = static_cast<float64>(-std::numeric_limits<T>::max());
      maxValue = static_cast<float64>(std::numeric_limits<T>::max());
    }
    else
    {
      // Everything Else
      minValue = static_cast<float64>(std::numeric_limits<T>::min());
      maxValue = static_cast<float64>(std::numeric_limits<T>::max());
    }

    if(customValue < minValue || customValue > maxValue)
    {
      return MakeErrorResult(-100, "Custom value is outside the bounds of the chosen data type!");
    }

    return {};
  }
};
} // namespace

// -----------------------------------------------------------------------------
std::string MultiThresholdObjectsFilter::name() const
{
  return FilterTraits<MultiThresholdObjectsFilter>::name;
}

//------------------------------------------------------------------------------
std::string MultiThresholdObjectsFilter::className() const
{
  return FilterTraits<MultiThresholdObjectsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid MultiThresholdObjectsFilter::uuid() const
{
  return FilterTraits<MultiThresholdObjectsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string MultiThresholdObjectsFilter::humanName() const
{
  return "Multi-Threshold Objects";
}

//------------------------------------------------------------------------------
std::vector<std::string> MultiThresholdObjectsFilter::defaultTags() const
{
  return {className(), "Find Outliers", "Threshold", "Isolate", "Data Management"};
}

//------------------------------------------------------------------------------
Parameters MultiThresholdObjectsFilter::parameters() const
{
  Parameters params;

  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insert(std::make_unique<ArrayThresholdsParameter>(k_ArrayThresholdsObject_Key, "Data Thresholds", "DataArray thresholds to mask", ArrayThresholdSet{},
                                                           ArrayThresholdsParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<DataTypeParameter>(k_CreatedMaskType_Key, "Mask Type", "DataType used for the created Mask Array", DataType::boolean));
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseCustomTrueValue, "Use Custom TRUE Value", "Specifies whether to output a custom TRUE value (the default value is 1)", false));
  params.insert(std::make_unique<NumberParameter<float64>>(k_CustomTrueValue, "Custom TRUE Value", "This is the custom TRUE value that will be output to the mask array", 1.0));
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseCustomFalseValue, "Use Custom FALSE Value", "Specifies whether to output a custom FALSE value (the default value is 0)", false));
  params.insert(std::make_unique<NumberParameter<float64>>(k_CustomFalseValue, "Custom FALSE Value", "This is the custom FALSE value that will be output to the mask array", 0.0));
  params.insert(std::make_unique<DataObjectNameParameter>(k_CreatedDataName_Key, "Mask Array", "DataPath to the created Mask Array", "Mask"));

  params.linkParameters(k_UseCustomTrueValue, k_CustomTrueValue, true);
  params.linkParameters(k_UseCustomFalseValue, k_CustomFalseValue, true);

  return params;
}

//------------------------------------------------------------------------------
IFilter::VersionType MultiThresholdObjectsFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer MultiThresholdObjectsFilter::clone() const
{
  return std::make_unique<MultiThresholdObjectsFilter>();
}

// -----------------------------------------------------------------------------
IFilter::PreflightResult MultiThresholdObjectsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& args, const MessageHandler& messageHandler,
                                                                    const std::atomic_bool& shouldCancel) const
{
  auto thresholdsObject = args.value<ArrayThresholdSet>(k_ArrayThresholdsObject_Key);
  auto maskArrayName = args.value<std::string>(k_CreatedDataName_Key);
  auto maskArrayType = args.value<DataType>(k_CreatedMaskType_Key);
  auto useCustomTrueValue = args.value<BoolParameter::ValueType>(k_UseCustomTrueValue);
  auto useCustomFalseValue = args.value<BoolParameter::ValueType>(k_UseCustomFalseValue);
  auto customTrueValue = args.value<NumberParameter<float64>::ValueType>(k_CustomTrueValue);
  auto customFalseValue = args.value<NumberParameter<float64>::ValueType>(k_CustomFalseValue);

  auto thresholdPaths = thresholdsObject.getRequiredPaths();
  // If the paths are empty just return now.
  if(thresholdPaths.empty())
  {
    return MakePreflightErrorResult(-4000, "No data arrays were found for calculating threshold");
  }

  for(const auto& path : thresholdPaths)
  {
    if(dataStructure.getData(path) == nullptr)
    {
      auto errorMessage = fmt::format("Could not find DataArray at path {}.", path.toString());
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::PathNotFoundError), errorMessage}})};
    }
  }

  // Check for Scalar arrays
  for(const auto& dataPath : thresholdPaths)
  {
    const auto* currentDataArray = dataStructure.getDataAs<IDataArray>(dataPath);
    if(currentDataArray != nullptr && currentDataArray->getNumberOfComponents() != 1)
    {
      auto errorMessage = fmt::format("Data Array is not a Scalar Data Array. Data Arrays must only have a single component. '{}:{}'", dataPath.toString(), currentDataArray->getNumberOfComponents());
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::NonScalarArrayFound), errorMessage}})};
    }
  }

  // Check that all arrays the number of tuples match
  DataPath firstDataPath = *(thresholdPaths.begin());
  const auto* dataArray = dataStructure.getDataAs<IDataArray>(firstDataPath);
  size_t numTuples = dataArray->getNumberOfTuples();

  for(const auto& dataPath : thresholdPaths)
  {
    const auto* currentDataArray = dataStructure.getDataAs<IDataArray>(dataPath);
    if(numTuples != currentDataArray->getNumberOfTuples())
    {
      auto errorMessage =
          fmt::format("Data Arrays do not have same equal number of tuples. '{}:{}' and '{}'", firstDataPath.toString(), numTuples, dataPath.toString(), currentDataArray->getNumberOfTuples());
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::UnequalTuples), errorMessage}})};
    }
  }

  if(maskArrayType == DataType::boolean)
  {
    if(useCustomTrueValue)
    {
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::CustomTrueWithBoolean), "Cannot use custom TRUE value with a boolean Mask Type."}})};
    }

    if(useCustomFalseValue)
    {
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::CustomFalseWithBoolean), "Cannot use custom FALSE value with a boolean Mask Type."}})};
    }
  }

  if(useCustomTrueValue)
  {
    Result<> result = ExecuteDataFunction(CheckCustomValueInBounds{}, maskArrayType, customTrueValue);
    if(result.invalid())
    {
      auto errorMessage = fmt::format("Custom TRUE value ({}) is outside the bounds of the chosen Mask Type ({}).", customTrueValue, DataTypeToString(maskArrayType));
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::CustomTrueOutOfBounds), errorMessage}})};
    }
  }

  if(useCustomFalseValue)
  {
    Result<> result = ExecuteDataFunction(CheckCustomValueInBounds{}, maskArrayType, customFalseValue);
    if(result.invalid())
    {
      auto errorMessage = fmt::format("Custom FALSE value ({}) is outside the bounds of the chosen Mask Type ({}).", customFalseValue, DataTypeToString(maskArrayType));
      return {nonstd::make_unexpected(std::vector<Error>{Error{to_underlying(ErrorCodes::CustomFalseOutOfBounds), errorMessage}})};
    }
  }

  // Create the output boolean array
  auto action =
      std::make_unique<CreateArrayAction>(maskArrayType, dataArray->getIDataStore()->getTupleShape(), std::vector<usize>{1}, firstDataPath.replaceName(maskArrayName), dataArray->getDataFormat());

  OutputActions actions;
  actions.appendAction(std::move(action));

  return {std::move(actions)};
}

// -----------------------------------------------------------------------------
Result<> MultiThresholdObjectsFilter::executeImpl(DataStructure& dataStructure, const Arguments& args, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                  const std::atomic_bool& shouldCancel) const
{
  auto thresholdsObject = args.value<ArrayThresholdSet>(k_ArrayThresholdsObject_Key);
  auto maskArrayName = args.value<std::string>(k_CreatedDataName_Key);
  auto maskArrayType = args.value<DataType>(k_CreatedMaskType_Key);
  auto useCustomTrueValue = args.value<BoolParameter::ValueType>(k_UseCustomTrueValue);
  auto useCustomFalseValue = args.value<BoolParameter::ValueType>(k_UseCustomFalseValue);
  auto customTrueValue = args.value<NumberParameter<float64>::ValueType>(k_CustomTrueValue);
  auto customFalseValue = args.value<NumberParameter<float64>::ValueType>(k_CustomFalseValue);

  float64 trueValue = useCustomTrueValue ? customTrueValue : 1.0;
  float64 falseValue = useCustomFalseValue ? customFalseValue : 0.0;

  bool firstValueFound = false;
  DataPath maskArrayPath = (*thresholdsObject.getRequiredPaths().begin()).replaceName(maskArrayName);
  int32_t err = 0;
  ArrayThresholdSet::CollectionType thresholdSet = thresholdsObject.getArrayThresholds();
  for(const std::shared_ptr<IArrayThreshold>& threshold : thresholdSet)
  {
    if(std::dynamic_pointer_cast<ArrayThresholdSet>(threshold))
    {
      std::shared_ptr<ArrayThresholdSet> comparisonSet = std::dynamic_pointer_cast<ArrayThresholdSet>(threshold);
      ExecuteDataFunction(ThresholdSetFunctor{}, maskArrayType, comparisonSet, dataStructure, dataStructure.getDataAs<IDataArray>(maskArrayPath), err, !firstValueFound, thresholdsObject.isInverted(),
                          trueValue, falseValue);
      firstValueFound = true;
    }
    else if(std::dynamic_pointer_cast<ArrayThreshold>(threshold))
    {
      std::shared_ptr<ArrayThreshold> comparisonValue = std::dynamic_pointer_cast<ArrayThreshold>(threshold);
      ExecuteDataFunction(ThresholdValueFunctor{}, maskArrayType, comparisonValue, dataStructure, dataStructure.getDataAs<IDataArray>(maskArrayPath), err, !firstValueFound,
                          thresholdsObject.isInverted(), trueValue, falseValue);
      firstValueFound = true;
    }
  }

  return {};
}

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_SelectedThresholdsKey = "SelectedThresholds";
constexpr StringLiteral k_ScalarTypeKey = "ScalarType";
constexpr StringLiteral k_DestinationArrayNameKey = "DestinationArrayName";
} // namespace SIMPL
} // namespace

Result<Arguments> MultiThresholdObjectsFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = MultiThresholdObjectsFilter().getDefaultArguments();
  static constexpr StringLiteral k_FilterUuidKey = "Filter_Uuid";
  static constexpr StringLiteral v1Uuid = "{014b7300-cf36-5ede-a751-5faf9b119dae}";

  std::vector<Result<>> results;

  bool isAdvanced = json[k_FilterUuidKey].get<std::string>() != v1Uuid;

  if(isAdvanced)
  {
    results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::ComparisonSelectionAdvancedFilterParameterConverter>(args, json, SIMPL::k_SelectedThresholdsKey, k_ArrayThresholdsObject_Key));
  }
  else
  {
    results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::ComparisonSelectionFilterParameterConverter>(args, json, SIMPL::k_SelectedThresholdsKey, k_ArrayThresholdsObject_Key));
  }
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::ScalarTypeParameterToNumericTypeConverter>(args, json, SIMPL::k_ScalarTypeKey, k_CreatedMaskType_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_DestinationArrayNameKey, k_CreatedDataName_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}
} // namespace nx::core

#include "DataStructure.hpp"

#include "complex/DataStructure/BaseGroup.hpp"
#include "complex/DataStructure/DataGroup.hpp"
#include "complex/DataStructure/LinkedPath.hpp"
#include "complex/DataStructure/Messaging/DataAddedMessage.hpp"
#include "complex/DataStructure/Messaging/DataRemovedMessage.hpp"
#include "complex/DataStructure/Messaging/DataReparentedMessage.hpp"
#include "complex/DataStructure/Observers/AbstractDataStructureObserver.hpp"
#include "complex/Filter/DataParameter.hpp"
#include "complex/Filter/ValueParameter.hpp"
#include "complex/Utilities/Parsing/HDF5/H5DataStructureReader.hpp"
#include "complex/Utilities/Parsing/HDF5/H5DataStructureWriter.hpp"
#include "complex/Utilities/Parsing/HDF5/H5FileReader.hpp"
#include "complex/Utilities/Parsing/HDF5/H5FileWriter.hpp"

#include <numeric>
#include <stdexcept>

namespace complex
{

namespace Constants
{
inline const std::string DataStructureTag = "DataStructure";
}

DataStructure::DataStructure()
: m_IsValid(true)
{
}

DataStructure::DataStructure(const DataStructure& ds)
: m_DataObjects(ds.m_DataObjects)
, m_RootGroup(ds.m_RootGroup)
, m_IsValid(ds.m_IsValid)
, m_NextId(ds.m_NextId)
{
  // Hold a shared_ptr copy of the DataObjects long enough for
  // m_RootGroup.setDataStructure(this) to operate.
  std::map<DataObject::IdType, std::shared_ptr<DataObject>> sharedData;
  for(auto& [id, dataPtr] : ds.m_DataObjects)
  {
    auto copy = std::shared_ptr<DataObject>(dataPtr.lock()->shallowCopy());
    sharedData[id] = copy;
    m_DataObjects[id] = copy;
  }
  // Updates all DataMaps with the corresponding m_DataObjects pointers.
  // Updates all DataObjects with their new DataStructure
  m_RootGroup.setDataStructure(this);
}

DataStructure::DataStructure(DataStructure&& ds) noexcept
: m_DataObjects(std::move(ds.m_DataObjects))
, m_RootGroup(std::move(ds.m_RootGroup))
, m_IsValid(std::move(ds.m_IsValid))
, m_NextId(std::move(ds.m_NextId))
{
  m_RootGroup.setDataStructure(this);
}

DataStructure::~DataStructure()
{
  m_IsValid = false;
}

DataObject::IdType DataStructure::generateId()
{
  return m_NextId++;
}

void DataStructure::setNextId(DataObject::IdType nextDataId)
{
  m_NextId = nextDataId;
}

size_t DataStructure::getSize() const
{
  return m_DataObjects.size();
}

void DataStructure::clear()
{
  auto topDataIds = m_RootGroup.getKeys();
  for(auto dataId : topDataIds)
  {
    removeData(dataId);
  }
  m_DataObjects.clear();
}

std::optional<DataObject::IdType> DataStructure::getId(const DataPath& path) const
{
  return getData(path)->getId();
}

LinkedPath DataStructure::getLinkedPath(const DataPath& path) const
{
  try
  {
    std::vector<DataObject::IdType> pathIds;
    const DataObject* data = m_RootGroup[path[0]];
    const BaseGroup* parent = dynamic_cast<const BaseGroup*>(data);
    pathIds.push_back(data->getId());

    for(usize i = 1; i < path.getLength(); i++)
    {
      std::string name = path[i];
      data = (*parent)[name];
      pathIds.push_back(data->getId());

      parent = dynamic_cast<const BaseGroup*>(data);
    }

    return LinkedPath(this, pathIds);
  } catch(std::exception e)
  {
    return LinkedPath();
  }
}

bool DataStructure::containsData(DataObject::IdType id) const
{
  return getData(id) != nullptr;
}

Result<LinkedPath> DataStructure::makePath(const DataPath& path)
{
  try
  {
    std::vector<DataObject::IdType> pathIds;
    std::string name = path[0];
    const DataObject* data = m_RootGroup[name];
    if(data == nullptr)
    {
      data = complex::DataGroup::Create(*this, name);
    }
    const BaseGroup* parent = dynamic_cast<const BaseGroup*>(data);
    pathIds.push_back(data->getId());

    for(usize i = 1; i < path.getLength(); i++)
    {
      name = path[i];
      data = (*parent)[name];
      if(data == nullptr)
      {
        data = DataGroup::Create(*this, name, pathIds.back());
      }
      pathIds.push_back(data->getId());

      parent = dynamic_cast<const BaseGroup*>(data);
    }

    return {LinkedPath(this, pathIds)};
  } catch(const std::exception& e)
  {
    return complex::MakeErrorResult<LinkedPath>(-2, "Exception thrown when attempting to create a path in the DataStructure");
  }
}

DataObject* DataStructure::getData(DataObject::IdType id)
{
  auto iter = m_DataObjects.find(id);
  if(m_DataObjects.end() == iter)
  {
    return nullptr;
  }
  return iter->second.lock().get();
}

DataObject* DataStructure::getData(const std::optional<DataObject::IdType>& id)
{
  if(!id)
  {
    return nullptr;
  }

  auto iter = m_DataObjects.find(id.value());
  if(m_DataObjects.end() == iter)
  {
    return nullptr;
  }
  return iter->second.lock().get();
}

DataObject* traversePath(DataObject* obj, const DataPath& path, usize index)
{
  if(path.getLength() == index)
  {
    return obj;
  }
  auto col = dynamic_cast<BaseGroup*>(obj);
  if(col == nullptr)
  {
    return nullptr;
  }
  DataObject* child = (*col)[path[index]];
  return traversePath(child, path, index + 1);
}

DataObject* DataStructure::getData(const DataPath& path)
{
  auto topLevel = getTopLevelData();
  for(DataObject* obj : topLevel)
  {
    if(obj == nullptr)
    {
      continue;
    }
    if(obj->getName() == path[0])
    {
      return traversePath(obj, path, 1);
    }
  }
  return nullptr;
}

DataObject* DataStructure::getData(const LinkedPath& path)
{
  return getData(path.getId());
}

const DataObject* DataStructure::getData(DataObject::IdType id) const
{
  auto iter = m_DataObjects.find(id);
  if(m_DataObjects.end() == iter)
  {
    return nullptr;
  }
  return iter->second.lock().get();
}

const DataObject* DataStructure::getData(const std::optional<DataObject::IdType>& id) const
{
  if(!id)
  {
    return nullptr;
  }

  auto iter = m_DataObjects.find(id.value());
  if(m_DataObjects.end() == iter)
  {
    return nullptr;
  }
  return iter->second.lock().get();
}

const DataObject* DataStructure::getData(const DataPath& path) const
{
  auto topLevel = getTopLevelData();
  for(DataObject* obj : topLevel)
  {
    if(obj == nullptr)
    {
      continue;
    }
    if(obj->getName() == path[0])
    {
      return traversePath(obj, path, 1);
    }
  }
  return nullptr;
}

const DataObject* DataStructure::getData(const LinkedPath& path) const
{
  return path.getData();
}

std::shared_ptr<DataObject> DataStructure::getSharedData(DataObject::IdType id) const
{
  if(m_DataObjects.find(id) == m_DataObjects.end())
  {
    return nullptr;
  }
  return m_DataObjects.at(id).lock();
}

bool DataStructure::removeData(DataObject::IdType id)
{
  DataObject* data = getData(id);
  return removeData(data);
}

bool DataStructure::removeData(const std::optional<DataObject::IdType>& id)
{
  if(!id)
  {
    return false;
  }
  else
  {
    return removeData(id.value());
  }
}

bool DataStructure::removeData(const DataPath& path)
{
  DataObject* data = getData(path);
  return removeData(data);
}

bool DataStructure::removeData(DataObject* data)
{
  if(data == nullptr)
  {
    return false;
  }

  auto pathsToData = data->getDataPaths();
  auto parents = data->getParents();
  if(parents.size() == 0)
  {
    return removeTopLevel(data);
  }
  for(BaseGroup* parent : parents)
  {
    if(!parent->remove(data))
    {
      return false;
    }
  }

  return true;
}

void DataStructure::dataDeleted(DataObject::IdType id, const std::string& name)
{
  if(!m_IsValid)
  {
    return;
  }

  m_DataObjects.erase(id);
  auto msg = std::make_shared<DataRemovedMessage>(this, id, name);
  notify(msg);
}

std::vector<DataObject*> DataStructure::getTopLevelData() const
{
  std::vector<DataObject*> topLevel;
  for(auto& iter : m_RootGroup)
  {
    auto obj = iter.second;
    topLevel.push_back(obj.get());
  }

  return topLevel;
}

const DataMap& DataStructure::getDataMap() const
{
  return m_RootGroup;
}

DataMap& DataStructure::getRootGroup()
{
  return m_RootGroup;
}

bool DataStructure::insertTopLevel(const std::shared_ptr<DataObject>& obj)
{
  if(obj == nullptr)
  {
    return false;
  }

  return m_RootGroup.insert(obj);
}

bool DataStructure::removeTopLevel(DataObject* data)
{
  std::string name = data->getName();
  if(!m_RootGroup.remove(data))
  {
    return false;
  }

  DataPath path({name});
  std::vector<DataPath> paths({path});
  return true;
}

bool DataStructure::finishAddingObject(const std::shared_ptr<DataObject>& obj, const std::optional<DataObject::IdType>& parent)
{
  if(parent.has_value())
  {
    auto parentContainer = dynamic_cast<BaseGroup*>(getData(parent.value()));
    if(!parentContainer->insert(obj))
    {
      return false;
    }
  }
  else if(!insertTopLevel(obj))
  {
    return false;
  }

  m_DataObjects[obj->getId()] = obj;
  auto msg = std::make_shared<DataAddedMessage>(this, obj->getId());
  notify(msg);
  return true;
}

DataStructure::Iterator DataStructure::begin()
{
  return m_RootGroup.begin();
}

DataStructure::Iterator DataStructure::end()
{
  return m_RootGroup.end();
}

DataStructure::ConstIterator DataStructure::begin() const
{
  return m_RootGroup.begin();
}

DataStructure::ConstIterator DataStructure::end() const
{
  return m_RootGroup.end();
}

bool DataStructure::setAdditionalParent(DataObject::IdType targetId, DataObject::IdType newParentId)
{
  auto& target = m_DataObjects[targetId];
  auto newParent = dynamic_cast<BaseGroup*>(getData(newParentId));
  if(newParent == nullptr)
  {
    return false;
  }

  if(!newParent->insert(target))
  {
    return false;
  }

  notify(std::make_shared<DataReparentedMessage>(this, targetId, newParentId, true));
  return true;
}

bool DataStructure::removeParent(DataObject::IdType targetId, DataObject::IdType parentId)
{
  const auto& target = m_DataObjects[targetId];
  auto parent = dynamic_cast<BaseGroup*>(getData(parentId));
  auto targetPtr = target.lock();
  if(targetPtr == nullptr)
  {
    return false;
  }
  return parent->remove(targetPtr.get());
}

DataStructure::SignalType& DataStructure::getSignal()
{
  return m_Signal;
}

void DataStructure::notify(const std::shared_ptr<AbstractDataStructureMessage>& msg)
{
  m_Signal(this, msg);
}

DataStructure& DataStructure::operator=(const DataStructure& rhs)
{
  m_DataObjects = rhs.m_DataObjects;
  m_RootGroup = rhs.m_RootGroup;
  m_IsValid = rhs.m_IsValid;
  m_NextId = rhs.m_NextId;

  // Hold a shared_ptr copy of the DataObjects long enough for
  // m_RootGroup.setDataStructure(this) to operate.
  std::map<DataObject::IdType, std::shared_ptr<DataObject>> sharedData;
  for(auto& [id, dataPtr] : rhs.m_DataObjects)
  {
    auto copy = std::shared_ptr<DataObject>(dataPtr.lock()->shallowCopy());
    sharedData[id] = copy;
    m_DataObjects[id] = copy;
  }
  // Updates all DataMaps with the corresponding m_DataObjects pointers.
  // Updates all DataObjects with their new DataStructure
  m_RootGroup.setDataStructure(this);
  return *this;
}

DataStructure& DataStructure::operator=(DataStructure&& rhs) noexcept
{
  m_DataObjects = std::move(rhs.m_DataObjects);
  m_RootGroup = std::move(rhs.m_RootGroup);
  m_IsValid = std::move(rhs.m_IsValid);
  m_NextId = std::move(rhs.m_NextId);
  m_RootGroup.setDataStructure(this);
  return *this;
}

H5::ErrorType DataStructure::writeHdf5(H5::GroupWriter& parentGroupWriter) const
{
  H5::DataStructureWriter dataStructureWriter;
  auto groupWriter = parentGroupWriter.createGroupWriter(Constants::k_DataStructureTag);
  auto idAttribute = groupWriter.createAttribute(Constants::k_NextIdTag);
  H5::ErrorType err = idAttribute.writeValue(m_NextId);
  if(err < 0)
  {
    return err;
  }

  err = m_RootGroup.writeH5Group(dataStructureWriter, groupWriter);
  return err;
}

DataStructure DataStructure::readFromHdf5(const H5::GroupReader& groupReader, H5::ErrorType& err)
{
  H5::DataStructureReader dataStructureReader;
  return dataStructureReader.readH5Group(groupReader, err);
}

} // namespace complex

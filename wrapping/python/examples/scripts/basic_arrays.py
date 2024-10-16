"""
Important Note
==============

This python file can be used as an example of how to execute a number of DREAM3D-NX
filters one after another, if you plan to use the codes below (and you are welcome to),
there are a few things that you, the developer, should take note of:

Import Statements
-----------------

You will most likely *NOT* need to include the following code:

   .. code:: python
      
      import simplnx_test_dirs as nxtest

Filter Error Detection
----------------------

In each section of code a filter is created and executed immediately. This may or
may *not* be what you want to do. You can also preflight the filter to verify the
correctness of the filters before executing the filter **although** this is done
for you when the filter is executed. As such, you will want to check the 'result'
variable to see if there are any errors or warnings. If there **are** any then
you, as the developer, should act appropriately on the errors or warnings. 
More specifically, this bit of code:

   .. code:: python

      nxtest.check_filter_result(nxor.ReadAngDataFilter, result)

is used by the simplnx unit testing framework and should be replaced by your own
error checking code. You are welcome to look up the function definition and use
that.

"""
import simplnx as nx

import itkimageprocessing as nxitk
import orientationanalysis as nxor
import simplnx_test_dirs as nxtest

import numpy as np

#------------------------------------------------------------------------------
# Print the various filesystem paths that are pregenerated for this machine.
#------------------------------------------------------------------------------
nxtest.print_all_paths()

data_path = nx.DataPath("root")
print(f'{data_path}' )
print(f'length: {len(data_path)}')

data_path = nx.DataPath("/root/foo/bar")
print(f'{data_path}' )

path_from_list = ["root", "foo", "bar"]
data_path = nx.DataPath(path_from_list)
print(f'{data_path}' )

data_path = nx.DataPath(["root", "foo", "bar"])
print(f'{data_path}' )

path_as_string = str(data_path)
print(f'__str__: {path_as_string}' )
print(f'__repr__: {data_path}' )

# List type access
for item in data_path:
    print(f'  {item}')

# Access Item by Index
print(f'access by index: {data_path[1]}')

# Use a different delimiter to generate the string.
print(f'{data_path.to_string("|")}')

# Generate a child path:
deeper_path = data_path.create_child_path("more stuff")
print(f'deeper_path: {deeper_path}' )

# pathlib operations
# Get the parts as a list
path_parts = data_path.parts()
print(f'path_parts: {path_parts}' )
# Return Parent
parent_path = data_path.parent()
print(f'parent_path: {parent_path}' )
# Return Name (the last part of the path)
last_path = data_path.name()
print(f'last_path: {last_path}' )
# Change the name
name_change = data_path.with_name("NEW NAME")
print(f'name_change: {name_change}' )

# Create a Data Structure
data_structure = nx.DataStructure()

#------------------------------------------------------------------------------
# Create a top level group: (Not needed)
#------------------------------------------------------------------------------
result = nx.CreateDataGroupFilter.execute(data_structure=data_structure,
                                    data_object_path=nx.DataPath(['Group']))
nxtest.check_filter_result(nx.CreateDataGroupFilter, result)


result = nx.CreateDataGroupFilter.execute(data_structure=data_structure, 
                                    data_object_path=nx.DataPath("/Some/Path/To/Group"))
nxtest.check_filter_result(nx.CreateDataGroupFilter, result)


#------------------------------------------------------------------------------
# Check if that object exists in the DataStructure
#------------------------------------------------------------------------------
value = data_structure.exists("/Some/Path/To/Group")
print(f'The path "/Some/Path/To/Group" exists: {value} ')

value = data_structure.exists("/Some/Path/To/NonExistantGroup")
print(f'The path "/Some/Path/To/NonExistantGroup" exists: {value} ')

#------------------------------------------------------------------------------
# Create 1D Array 
#------------------------------------------------------------------------------
# Create the Input Parameters to the CreateDataArrayFilter filter
output_array_path = nx.DataPath(["Group", "1D Array"])
array_type = nx.NumericType.float32
tuple_dims = [[10]]
create_array_nx_filter = nx.CreateDataArrayFilter()
result  = create_array_nx_filter.execute(data_structure=data_structure, 
                                          component_count=1, 
                                          data_format="", 
                                          initialization_value_str="10", 
                                          numeric_type_index=array_type, 
                                          output_array_path=output_array_path, 
                                          tuple_dimensions=tuple_dims)
nxtest.check_filter_result(nx.CreateDataArrayFilter, result)


# We can check the output of the filter by simply printing the array
npdata = data_structure[output_array_path].npview()
print(npdata)

#------------------------------------------------------------------------------
# Create 2D Array 
#------------------------------------------------------------------------------

# Create a 2D Array with dimensions 2, 5 where the 5 is the fastest moving dimension.
# Example, and Image where 5 wide x 2 High
output_array_path = nx.DataPath(["2D Array"])
tuple_dims = [[2,5]]
create_array_nx_filter = nx.CreateDataArrayFilter()
result  = create_array_nx_filter.execute(data_structure=data_structure, 
                                      component_count=1, 
                                      data_format="", 
                                      initialization_value_str="10", 
                                        numeric_type_index=array_type, 
                                        output_array_path=output_array_path, 
                                        tuple_dimensions=tuple_dims)
nxtest.check_filter_result(nx.CreateDataArrayFilter, result)


data_array = data_structure[output_array_path]
print(f'name: {data_array.name}')
print(f'tuple_shape: {data_array.tuple_shape}')
print(f'component_shape: {data_array.component_shape}')
print(f'dtype: {data_array.dtype}')

npdata = data_structure[output_array_path].npview()
print(npdata)


#------------------------------------------------------------------------------
# Create 3D Array 
#------------------------------------------------------------------------------

# Create a 3D Array with dimensions 3, 2, 5 where the 5 is the fastest moving dimension.
# Example, and Image where 5 wide x 2 High
output_array_path = nx.DataPath(["3D Array"])
tuple_dims = [[3, 2, 5]]
create_array_nx_filter = nx.CreateDataArrayFilter()
result = create_array_nx_filter.execute(data_structure=data_structure, 
                                      component_count=1, 
                                      data_format="", 
                                      initialization_value_str="10", 
                                        numeric_type_index=array_type, 
                                        output_array_path=output_array_path, 
                                        tuple_dimensions=tuple_dims)
nxtest.check_filter_result(nx.CreateDataArrayFilter, result)


npdata = data_structure[output_array_path].npview()
print(npdata)

result = nx.CreateAttributeMatrixFilter.execute(data_structure=data_structure, 
                                                data_object_path=nx.DataPath(["New Attribute Matrix"]), 
                                                tuple_dimensions = [[100., 200., 300.]])
nxtest.check_filter_result(nx.CreateAttributeMatrixFilter, result)



output_file_path = nxtest.get_test_temp_directory() / "output_file_example.dream3d"
result = nx.WriteDREAM3DFilter.execute(data_structure=data_structure, 
                                        export_file_path=output_file_path, 
                                        write_xdmf_file=True)
nxtest.check_filter_result(nx.WriteDREAM3DFilter, result)



#------------------------------------------------------------------------------
# If you need to show the hierarchy of the DataStructure you can do one of the
# following:
#------------------------------------------------------------------------------

# This will generate the hierarchy as a GraphViz formatted string that you can
# print or save to a file
graphviz_content = data_structure.hierarchy_to_graphviz()
print(graphviz_content)

# This will generate the hierarchy as an ASCI Formatted string.
hierarchy_as_str = data_structure.hierarchy_to_str()
print(hierarchy_as_str)

#------------------------------------------------------------------------------
# If you want to list out the children at a specific level of the DataStruture
#------------------------------------------------------------------------------
# Use an empty path for the top level objects
children_paths = data_structure.get_children(nx.DataPath(""))
print(children_paths)

children_paths = data_structure.get_children(nx.DataPath("Some/Path/To"))
print(children_paths)

print(f'data_structure.size: {data_structure.size}')

print(f'Removing Data Array')
result = nx.DeleteDataFilter.execute(data_structure=data_structure,
                               removed_data_path=[nx.DataPath("Group/1D Array")])
nxtest.check_filter_result(nx.WriteDREAM3DFilter, result)
# This will generate the hierarchy as an ASCI Formatted string.
hierarchy_as_str = data_structure.hierarchy_to_str()
print(hierarchy_as_str)

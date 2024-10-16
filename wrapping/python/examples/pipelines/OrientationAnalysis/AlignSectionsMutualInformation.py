import simplnx as nx

import itkimageprocessing as cxitk
import orientationanalysis as cxor
import simplnx_test_dirs as nxtest

import numpy as np

# Create a Data Structure
data_structure = nx.DataStructure()

# Filter 1

# Create the ReadH5EbsdFileParameter and assign values to it.
h5ebsdParameter = cxor.ReadH5EbsdFileParameter.ValueType()
h5ebsdParameter.euler_representation=0
h5ebsdParameter.end_slice=117
h5ebsdParameter.selected_array_names=["Confidence Index", "EulerAngles", "Fit", "Image Quality", "Phases", "SEM Signal", "X Position", "Y Position"]
h5ebsdParameter.input_file_path=str(nxtest.get_data_directory() / "Output/Reconstruction/Small_IN100.h5ebsd")
h5ebsdParameter.start_slice=1
h5ebsdParameter.use_recommended_transform=True

# Instantiate Filter
nx_filter = cxor.ReadH5EbsdFilter()
# Execute Filter with Parameters
result = nx_filter.execute(
    data_structure=data_structure,
    cell_attribute_matrix_name="Cell Data",
    cell_ensemble_attribute_matrix_name="Cell Ensemble Data",
    output_image_geometry_path =nx.DataPath("DataContainer"),
    read_h5_ebsd_object=h5ebsdParameter
)
nxtest.check_filter_result(nx_filter, result)

# Filter 2
# Set Up Thresholds and Instantiate Filter
threshold_1 = nx.ArrayThreshold()
threshold_1.array_path = nx.DataPath("DataContainer/Cell Data/Image Quality")
threshold_1.comparison = nx.ArrayThreshold.ComparisonType.GreaterThan
threshold_1.value = 120

threshold_2 = nx.ArrayThreshold()
threshold_2.array_path = nx.DataPath("DataContainer/Cell Data/Confidence Index")
threshold_2.comparison = nx.ArrayThreshold.ComparisonType.GreaterThan
threshold_2.value = 0.1

threshold_set = nx.ArrayThresholdSet()
threshold_set.thresholds = [threshold_1, threshold_2]
dt = nx.DataType.boolean

# Instantiate Filter
nx_filter = nx.MultiThresholdObjectsFilter()
# Execute Filter with Parameters
result = nx_filter.execute(
    data_structure=data_structure,
    array_thresholds_object=threshold_set,
    output_data_array_name="Mask",
    created_mask_type=nx.DataType.boolean
)
nxtest.check_filter_result(nx_filter, result)

# Filter 3
# Instantiate Filter
nx_filter = cxor.ConvertOrientationsFilter()
# Execute Filter with Parameters
result = nx_filter.execute(
    data_structure=data_structure,
    input_orientation_array_path=nx.DataPath("DataContainer/Cell Data/EulerAngles"),
    input_representation_index=0,
    output_orientation_array_name="Quats",
    output_representation_index=2
)
nxtest.check_filter_result(nx_filter, result)

# Filter 4
# Instantiate Filter
nx_filter = cxor.AlignSectionsMutualInformationFilter()
# Execute Filter with Parameters
result = nx_filter.execute(
    data_structure=data_structure,
    alignment_shift_file_name=nxtest.get_data_directory() / "Output/OrientationAnalysis/Alignment_By_Mutual_Information_Shifts.txt",
    cell_phases_array_path=nx.DataPath("DataContainer/Cell Data/Phases"),
    crystal_structures_array_path=nx.DataPath("DataContainer/Cell Ensemble Data/CrystalStructures"),
    mask_array_path=nx.DataPath("DataContainer/Cell Data/Mask"),
    misorientation_tolerance=5.0,
    quats_array_path=nx.DataPath("DataContainer/Cell Data/Quats"),
    input_image_geometry_path=nx.DataPath("DataContainer"),
    use_mask=True,
    write_alignment_shifts=True
)
nxtest.check_filter_result(nx_filter, result)

# Filter 5
# Instantiate Filter
nx_filter = nx.WriteDREAM3DFilter()
# Set Output File Path
output_file_path = nxtest.get_data_directory() / "Output/AlignSectionsMutualInformation/SmallIN100_AlignSectionsMutualInformation.dream3d"
# Execute Filter with Parameters
result = nx_filter.execute(
    data_structure=data_structure,
    export_file_path=output_file_path,
    write_xdmf_file=True
)
nxtest.check_filter_result(nx_filter, result)

# *****************************************************************************
# THIS SECTION IS ONLY HERE FOR CLEANING UP THE CI Machines
# If you are using this code, you should COMMENT out the next line
nxtest.cleanup_test_file(output_file_path)
# *****************************************************************************


print("===> Pipeline Complete")

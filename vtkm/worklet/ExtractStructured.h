//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 Sandia Corporation.
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#ifndef vtk_m_worklet_ExtractStructured_h
#define vtk_m_worklet_ExtractStructured_h

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCartesianProduct.h>
#include <vtkm/cont/CellSetStructured.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DynamicArrayHandle.h>
#include <vtkm/cont/ErrorBadValue.h>
#include <vtkm/cont/Field.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/ScatterCounting.h>

#include <vtkm/Math.h>

namespace vtkm {
namespace worklet {

//
// Distribute input point/cell data to subset output point/cell data
//
struct DistributeData : public vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<> inIndices,
                                FieldOut<> outIndices);
  typedef void ExecutionSignature(_1, _2);

  typedef vtkm::worklet::ScatterCounting ScatterType;

  VTKM_CONT
  ScatterType GetScatter() const { return this->Scatter; }

  template <typename CountArrayType, typename DeviceAdapter>
  VTKM_CONT
  DistributeData(const CountArrayType &countArray,
                 DeviceAdapter device) :
                         Scatter(countArray, device) {  }

  template <typename T>
  VTKM_EXEC
  void operator()(T inputIndex,
                  T &outputIndex) const
  {
    outputIndex = inputIndex;
  }
private:
  ScatterType Scatter;
};

//
// Extract subset of structured grid and/or resample
//
class ExtractStructured
{
public:
  ExtractStructured() {}

  //
  // Determine if index is within range of the subset and subsampling
  //
  class CreateMap : public vtkm::worklet::WorkletMapField
  {
  public:
    typedef void ControlSignature(FieldIn<IdType> index,
                                  FieldOut<IdComponentType> passValue);
    typedef   _2 ExecutionSignature(_1);

    vtkm::Id RowSize;
    vtkm::Id PlaneSize;
    vtkm::Bounds OutBounds;
    vtkm::Id3 Sample;

    VTKM_CONT
    CreateMap(const vtkm::Id3 inDimension,
              const vtkm::Bounds &outBounds,
              const vtkm::Id3 &sample) :
                         RowSize(inDimension[1]),
                         PlaneSize(inDimension[1] * inDimension[0]),
                         OutBounds(outBounds),
                         Sample(sample) {}

    VTKM_EXEC
    vtkm::IdComponent operator()(const vtkm::Id index) const
    {
      vtkm::IdComponent passValue = 0;

      // Position of this point or cell in the grid
      vtkm::IdComponent k = index / PlaneSize;
      vtkm::IdComponent j = (index % PlaneSize) / RowSize; 
      vtkm::IdComponent i = index % RowSize;
std::cout << "Cell index " << index << " i " << i << " j " << j << " k " << k << std::endl;
std::cout << "i " << i << " minbound " << OutBounds.X.Min << " maxbound " << OutBounds.X.Max << " sample " << Sample[0] << std::endl;
std::cout << "j " << j << " minbound " << OutBounds.Y.Min << " maxbound " << OutBounds.Y.Max << " sample " << Sample[1] << std::endl;
std::cout << "k " << k << " minbound " << OutBounds.Z.Min << " maxbound " << OutBounds.Z.Max << " sample " << Sample[2] << std::endl;

      // Within the subset range
      vtkm::Id3 ijk = vtkm::make_Vec(i, j, k);
      if (OutBounds.Contains(ijk))
      {
std::cout << "Within bounds" << std::endl;
        // Within the subsampling criteria
        vtkm::Id3 minPt = vtkm::make_Vec(OutBounds.X.Min,
                                         OutBounds.Y.Min,
                                         OutBounds.Z.Min);
        if (((i - minPt[0]) % Sample[0]) == 0 &&
            ((j - minPt[1]) % Sample[1]) == 0 &&
            ((k - minPt[2]) % Sample[2]) == 0)
        {
          passValue = 1;
        }
      }
      return passValue;
    }
  };

  //
  // Create maps for mapping point and cell data to subset
  //
  template <typename DeviceAdapter>
  void CreateDataMaps(const vtkm::Id3 &pointDimension,
                      const vtkm::Id &numberOfPoints,
                      const vtkm::Id &numberOfCells,
                      const vtkm::Bounds &outBounds,
                      const vtkm::Id3 &sample,
                      const DeviceAdapter)
  {
    vtkm::cont::ArrayHandleIndex pointIndices(numberOfPoints);
    vtkm::cont::ArrayHandleIndex cellIndices(numberOfCells);
std::cout << "POINT BOUNDS " << outBounds << std::endl;

    // Create the map for the input point data to output
    CreateMap pointWorklet(pointDimension, outBounds, sample);
    vtkm::worklet::DispatcherMapField<CreateMap> pointDispatcher(pointWorklet);
    pointDispatcher.Invoke(pointIndices,
                           this->PointMap);
vtkm::Id count = 0;
for (vtkm::Id i = 0; i < numberOfPoints; i++)
{
std::cout << "Point " << i << " passed " << PointMap.GetPortalControl().Get(i) << std::endl;
if (PointMap.GetPortalControl().Get(i) == 1)
count++;
}
std::cout << "Count Point " << count << std::endl;

    // Create the map for the input cell data to output
    vtkm::Id3 cellDimension = pointDimension - vtkm::Id3(1,1,1);
    vtkm::Bounds cellBounds = outBounds;
    if (cellBounds.X.Max > 0)
      cellBounds.X.Max -= 1;
    if (cellBounds.Y.Max > 0)
      cellBounds.Y.Max -= 1;
    if (cellBounds.Z.Max > 0)
      cellBounds.Z.Max -= 1;
std::cout << "CELL BOUNDS " << cellBounds << std::endl;

    CreateMap cellWorklet(cellDimension, cellBounds, sample);
    vtkm::worklet::DispatcherMapField<CreateMap> cellDispatcher(cellWorklet);
    cellDispatcher.Invoke(cellIndices,
                          this->CellMap);
count = 0;
for (vtkm::Id i = 0; i < numberOfCells; i++)
{
std::cout << "Cell " << i << " passed " << CellMap.GetPortalControl().Get(i) << std::endl;
if (CellMap.GetPortalControl().Get(i) == 1)
count++;
}
std::cout << "Count Cell " << count << std::endl;
  }

  //
  // Uniform Structured
  //
  template <typename CellSetType,
            typename DeviceAdapter>
  vtkm::cont::DataSet ExtractUniform(
                          const vtkm::IdComponent outDim,
                          const CellSetType &cellSet,
                          const vtkm::cont::CoordinateSystem &coordinates,
                          const vtkm::Bounds outBounds,
                          const vtkm::Id3 &sample,
                          DeviceAdapter)
  {
    typedef vtkm::cont::ArrayHandleUniformPointCoordinates UniformArrayHandle;
    typedef typename UniformArrayHandle::ExecutionTypes<DeviceAdapter>::PortalConst UniformConstPortal;

    // Data in the Field attached to CoordinateSystem is dynamic
    vtkm::cont::DynamicArrayHandleCoordinateSystem coordinateData = coordinates.GetData();

    // Cast dynamic coordinate data to Uniform type
    UniformArrayHandle vertices;
    vertices = coordinateData.Cast<UniformArrayHandle>();

    std::cout << "Uniform vertices:" << std::endl;
    printSummary_ArrayHandle(vertices, std::cout);

    // Portal to access data in the input coordinate system
    UniformConstPortal Coordinates = vertices.PrepareForInput(DeviceAdapter());

    // Sizes and values of input Uniform Structured
    vtkm::Id3 inDimension = Coordinates.GetDimensions();
    vtkm::Vec<vtkm::FloatDefault,3> inOrigin = Coordinates.GetOrigin();
    vtkm::Vec<vtkm::FloatDefault,3> inSpacing = Coordinates.GetSpacing();
    std::cout << "UNIFORM IN DIMENSION " << inDimension << std::endl;
    std::cout << "UNIFORM IN ORIGIN " << inOrigin << std::endl;
    std::cout << "UNIFORM IN SPACING " << inSpacing << std::endl;

    // Sizes of output Uniform with subsets and sampling
    // VTK calculates the map with subset, sample and intersect boundary flag
    // before deciding on size of the output
    vtkm::Id3 outDimension = vtkm::make_Vec(outBounds.X.Max - outBounds.X.Min + 1,
                                            outBounds.Y.Max - outBounds.Y.Min + 1,
                                            outBounds.Z.Max - outBounds.Z.Min + 1);
    for (vtkm::IdComponent dim = 0; dim < outDim; dim++)
    {
      if (sample[dim] > 1)
      {
        outDimension[dim] = outDimension[dim] / sample[dim] + 1;
      }
    }
    vtkm::Vec<vtkm::FloatDefault,3> outOrigin = vtkm::make_Vec(0,0,0);
    vtkm::Vec<vtkm::FloatDefault,3> outSpacing = vtkm::make_Vec(1,1,1);
    std::cout << "UNIFORM OUT DIMENSION " << outDimension << std::endl;
    std::cout << "UNIFORM OUT ORIGIN " << outOrigin << std::endl;
    std::cout << "UNIFORM OUT SPACING " << outSpacing << std::endl;

    // Create output dataset which needs modified coordinate system and cellset
    vtkm::cont::DataSet output;

    // Set the output CoordinateSystem information
    UniformArrayHandle outCoordinateData(outDimension, outOrigin, outSpacing);
    vtkm::cont::CoordinateSystem outCoordinates(coordinates.GetName(), outCoordinateData);
    output.AddCoordinateSystem(outCoordinates);

    std::cout << "CoordinateSystem for output:" << std::endl;
    outCoordinates.PrintSummary(std::cout);

    // Set the size of the cell set for Uniform
    if (outDim == 1) {
      vtkm::cont::CellSetStructured<1> outCellSet(cellSet.GetName());
      outCellSet.SetPointDimensions(outDimension[0]);
      output.AddCellSet(outCellSet);
    }
    else if (outDim == 2)
    {
      vtkm::cont::CellSetStructured<2> outCellSet(cellSet.GetName());
      outCellSet.SetPointDimensions(vtkm::make_Vec(outDimension[0], 
                                                   outDimension[1]));
      output.AddCellSet(outCellSet);
    }
    else if (outDim == 3)
    {
      vtkm::cont::CellSetStructured<3> outCellSet(cellSet.GetName());
      outCellSet.SetPointDimensions(vtkm::make_Vec(outDimension[0], 
                                                   outDimension[1], 
                                                   outDimension[2]));
      output.AddCellSet(outCellSet);
    }

std::cout << "Number of Cells " << output.GetCellSet(0).GetNumberOfCells() << std::endl;
std::cout << "Number of Points " << output.GetCellSet(0).GetNumberOfPoints() << std::endl;

    // Calculate and save the maps of point and cell data to subset
    CreateDataMaps(inDimension, 
                   cellSet.GetNumberOfPoints(),
                   cellSet.GetNumberOfCells(),
                   outBounds,
                   sample,
                   DeviceAdapter());

    return output;
  }

  //
  // Rectilinear Structured
  //
  template <typename CellSetType,
            typename DeviceAdapter>
  vtkm::cont::DataSet ExtractRectilinear(
                          const vtkm::IdComponent outDim,
                          const CellSetType &cellSet,
                          const vtkm::cont::CoordinateSystem &coordinates,
                          const vtkm::Bounds &outBounds,
                          const vtkm::Id3 &sample,
                          DeviceAdapter)
  {
    typedef vtkm::cont::ArrayHandle<vtkm::FloatDefault> DefaultHandle;
    typedef vtkm::cont::ArrayHandleCartesianProduct<DefaultHandle,DefaultHandle,DefaultHandle> CartesianArrayHandle;
    typedef typename DefaultHandle::ExecutionTypes<DeviceAdapter>::PortalConst DefaultConstHandle;
    typedef typename CartesianArrayHandle::ExecutionTypes<DeviceAdapter>::PortalConst CartesianConstPortal;

    // Data in the Field attached to CoordinateSystem is dynamic
    vtkm::cont::DynamicArrayHandleCoordinateSystem coordinateData = coordinates.GetData();

    // Cast dynamic coordinate data to Rectilinear type
    CartesianArrayHandle vertices;
    vertices = coordinateData.Cast<CartesianArrayHandle>();

    std::cout << "Recilinear vertices:" << std::endl;
    printSummary_ArrayHandle(vertices, std::cout);

    CartesianConstPortal Coordinates = vertices.PrepareForInput(DeviceAdapter());

    vtkm::Id NumberOfValues = Coordinates.GetNumberOfValues();
    std::cout << "RECTILINEAR NumberOfValues " << NumberOfValues << std::endl;

    DefaultConstHandle X = Coordinates.GetFirstPortal();
    DefaultConstHandle Y = Coordinates.GetSecondPortal();
    DefaultConstHandle Z = Coordinates.GetThirdPortal();

    vtkm::Id3 inDimension(X.GetNumberOfValues(), 
                          Y.GetNumberOfValues(), 
                          Z.GetNumberOfValues());
    std::cout << "Number of x coordinates " << inDimension[0] << std::endl;
    std::cout << "Number of y coordinates " << inDimension[1] << std::endl;
    std::cout << "Number of z coordinates " << inDimension[2] << std::endl;

    for (vtkm::Id x = 0; x < inDimension[0]; x++)
      std::cout << "X " << x << " = " << X.Get(x) << std::endl;
    for (vtkm::Id y = 0; y < inDimension[1]; y++)
      std::cout << "Y " << y << " = " << Y.Get(y) << std::endl;
    for (vtkm::Id z = 0; z < inDimension[2]; z++)
      std::cout << "Z " << z << " = " << Z.Get(z) << std::endl;

    vtkm::cont::DataSet output;

    // Sizes and values of output Rectilinear Structured
    vtkm::Id3 outDimension = vtkm::make_Vec(outBounds.X.Max - outBounds.X.Min + 1,
                                            outBounds.Y.Max - outBounds.Y.Min + 1,
                                            outBounds.Z.Max - outBounds.Z.Min + 1);
    std::cout << "RECTILINEAR OUT DIMENSIONS " << outDimension << std::endl;

    // Set output coordinate system
    DefaultHandle Xc, Yc, Zc;
    Xc.Allocate(outDimension[0]);
    Yc.Allocate(outDimension[1]);
    Zc.Allocate(outDimension[2]);

/*
    vtkm::Id indx = 0;
    for (vtkm::Id x = minBound[0]; x <= maxBound[0]; x++)
    {
      Xc.GetPortalControl().Set(indx++, X.Get(x));
    }
    indx = 0;
    for (vtkm::Id y = minBound[1]; y <= maxBound[1]; y++)
    {
      Yc.GetPortalControl().Set(indx++, Y.Get(y));
    }
    indx = 0;
    for (vtkm::Id z = minBound[2]; z <= maxBound[2]; z++)
    {
      Zc.GetPortalControl().Set(indx++, Z.Get(z));
    }

    for (vtkm::Id x = 0; x < outDimension[0]; x++)
      std::cout << "Xc " << x << " = " << Xc.GetPortalControl().Get(x) << std::endl;
    for (vtkm::Id y = 0; y < outDimension[1]; y++)
      std::cout << "Yc " << y << " = " << Yc.GetPortalControl().Get(y) << std::endl;
    for (vtkm::Id z = 0; z < outDimension[2]; z++)
      std::cout << "Zc " << z << " = " << Zc.GetPortalControl().Get(z) << std::endl;
*/

    // PKF TO DO
    // Fix coordinate system for subsampling

    CartesianArrayHandle outCoordinateData(Xc, Yc, Zc);
    vtkm::cont::CoordinateSystem outCoordinates(coordinates.GetName(), outCoordinateData);
    output.AddCoordinateSystem(outCoordinates);

    std::cout << "CoordinateSystem for output:" << std::endl;
    outCoordinates.PrintSummary(std::cout);

    // Set the size of the cell set for Rectilinear
    if (outDim == 1) {
      vtkm::cont::CellSetStructured<1> outCellSet(cellSet.GetName());
      outCellSet.SetPointDimensions(outDimension[0]);
      output.AddCellSet(outCellSet);
    }
    else if (outDim == 2)
    {
      vtkm::cont::CellSetStructured<2> outCellSet(cellSet.GetName());
      outCellSet.SetPointDimensions(vtkm::make_Vec(outDimension[0], 
                                                   outDimension[1]));
      output.AddCellSet(outCellSet);
    }
    else if (outDim == 3)
    {
      vtkm::cont::CellSetStructured<3> outCellSet(cellSet.GetName());
      outCellSet.SetPointDimensions(vtkm::make_Vec(outDimension[0], 
                                                   outDimension[1], 
                                                   outDimension[2]));
      output.AddCellSet(outCellSet);
    }

std::cout << "Number of Cells " << output.GetCellSet(0).GetNumberOfCells() << std::endl;
std::cout << "Number of Points " << output.GetCellSet(0).GetNumberOfPoints() << std::endl;

    // Calculate and save the maps of point and cell data to subset
    CreateDataMaps(inDimension, 
                   cellSet.GetNumberOfPoints(),
                   cellSet.GetNumberOfCells(),
                   outBounds,
                   sample,
                   DeviceAdapter());

    return output;
  }

  //
  // Run extract structured on uniform or rectilinear, subset and/or subsample
  //
  template <typename DeviceAdapter>
  vtkm::cont::DataSet Run(const vtkm::cont::DynamicCellSet &cellSet,
                          const vtkm::cont::CoordinateSystem &coordinates,
                          const vtkm::Bounds &boundingBox,
                          const vtkm::Id3 &sample,
                          DeviceAdapter)
  {
    // Check legality of input cellset and set input dimension
    vtkm::IdComponent inDim = 0;
    if (cellSet.IsSameType(vtkm::cont::CellSetStructured<1>()))
    {
      inDim = 1;
    }
    else if (cellSet.IsSameType(vtkm::cont::CellSetStructured<2>()))
    {
      inDim = 2;
    }
    else if (cellSet.IsSameType(vtkm::cont::CellSetStructured<3>()))
    {
      inDim = 3;
    }
    else
    {
      throw vtkm::cont::ErrorBadType("Only Structured cell sets allowed");
      return vtkm::cont::DataSet();
    }
    std::cout << "INPUT DIMENSION " << inDim << std::endl;

    // Check legality of requested bounds
    if (boundingBox.IsNonEmpty() == false)
    {
      throw vtkm::cont::ErrorBadValue("Requested bounding box is not valid");
      return vtkm::cont::DataSet();
    }

    // Check legality of sampling
    if (sample[0] < 1 || sample[1] < 1 || sample[2] < 1)
    {
      throw vtkm::cont::ErrorBadValue("Requested sampling is not valid");
      return vtkm::cont::DataSet();
    }

    // Requested bounding box intersection with input bounding box
    vtkm::Bounds inBounds = coordinates.GetBounds();
    vtkm::Bounds outBounds = boundingBox;

    std::cout << "INPUT BOUNDING BOX " << inBounds << std::endl;
    std::cout << "ORIGINAL BOUNDING BOX " << boundingBox << std::endl;
    std::cout << "SAMPLE " << sample << std::endl;

    if (outBounds.X.Min < inBounds.X.Min)
      outBounds.X.Min = inBounds.X.Min;
    if (outBounds.X.Max > inBounds.X.Max)
      outBounds.X.Max = inBounds.X.Max;
    if (outBounds.Y.Min < inBounds.Y.Min)
      outBounds.Y.Min = inBounds.Y.Min;
    if (outBounds.Y.Max > inBounds.Y.Max)
      outBounds.Y.Max = inBounds.Y.Max;
    if (outBounds.Z.Min < inBounds.Z.Min)
      outBounds.Z.Min = inBounds.Z.Min;
    if (outBounds.Z.Max > inBounds.Z.Max)
      outBounds.Z.Max = inBounds.Z.Max;

    // Bounding box intersects
    if (outBounds.IsNonEmpty() == false)
    {
      throw vtkm::cont::ErrorBadValue("Bounding box does not intersect input");
      return vtkm::cont::DataSet();
    }

    // Uniform, Regular or Rectilinear
    typedef vtkm::cont::ArrayHandleUniformPointCoordinates UniformArrayHandle;
    bool IsUniformDataSet = 0;
    if (coordinates.GetData().IsSameType(UniformArrayHandle()))
    {
      IsUniformDataSet = true;
    }
    std::cout << "IsUniformDataSet " << IsUniformDataSet << std::endl;

    if (IsUniformDataSet)
    {
      return ExtractUniform(inDim,
                            cellSet,
                            coordinates,
                            outBounds,
                            sample,
                            DeviceAdapter());
    }
    else
    {
      return ExtractRectilinear(inDim,
                                cellSet,
                                coordinates,
                                outBounds,
                                sample,
                                DeviceAdapter());
    }
  }

  //
  // Subset and/or subsampling of Point Data
  //
  template <typename T,
            typename StorageType,
            typename DeviceAdapter>
  vtkm::cont::ArrayHandle<T, StorageType> ProcessPointField(
                                            const vtkm::cont::ArrayHandle<T, StorageType> &input,
                                            const DeviceAdapter& device)
  {
    vtkm::cont::ArrayHandle<T, StorageType> output;

    DistributeData distribute(this->PointMap, device);
    vtkm::worklet::DispatcherMapField<DistributeData, DeviceAdapter> dispatch(distribute);
    dispatch.Invoke(input, output);
    return output;
  }

  //
  // Subset and/or subsampling of Cell Data
  //
  template <typename T,
            typename StorageType,
            typename DeviceAdapter>
  vtkm::cont::ArrayHandle<T, StorageType> ProcessCellField(
                                            const vtkm::cont::ArrayHandle<T, StorageType> &input,
                                            const DeviceAdapter& device)
  {
    vtkm::cont::ArrayHandle<T, StorageType> output;

    DistributeData distribute(this->CellMap, device);
    vtkm::worklet::DispatcherMapField<DistributeData, DeviceAdapter> dispatch(distribute);
    dispatch.Invoke(input, output);
    return output;
  }

private:
  vtkm::cont::ArrayHandle<vtkm::IdComponent> PointMap;
  vtkm::cont::ArrayHandle<vtkm::IdComponent> CellMap;
};

}
} // namespace vtkm::worklet

#endif // vtk_m_worklet_ExtractStructured_h

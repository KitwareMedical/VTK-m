//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/worklet/ExtractPoints.h>

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

#include <vtkm/cont/ArrayPortalToIterators.h>
#include <vtkm/cont/CellSet.h>

#include <algorithm>
#include <iostream>
#include <vector>

using vtkm::cont::testing::MakeTestDataSet;

template <typename DeviceAdapter>
class TestingExtractPoints
{
public:
  void TestExtractPointsWithSphere() const
  {
    std::cout << "Testing extract points with implicit function (sphere):" << std::endl;

    typedef vtkm::cont::CellSetStructured<3> CellSetType;
    typedef vtkm::cont::CellSetSingleType<> OutCellSetType;

    // Input data set created
    vtkm::cont::DataSet dataset = MakeTestDataSet().Make3DUniformDataSet1();
    CellSetType cellset;
    dataset.GetCellSet(0).CopyTo(cellset);
  
    // Implicit function
    vtkm::Vec<vtkm::FloatDefault, 3> center(2, 2, 2);
    vtkm::FloatDefault radius(1.8);
    vtkm::Sphere sphere(center, radius);
  
    // Output dataset contains input coordinate system and point data
    vtkm::cont::DataSet outDataSet;
    outDataSet.AddCoordinateSystem(dataset.GetCoordinateSystem(0));
    for (vtkm::Id indx = 0; indx < dataset.GetNumberOfFields(); indx++)
    {
      vtkm::cont::Field field = dataset.GetField(indx);
      if (field.GetAssociation() == vtkm::cont::Field::ASSOC_POINTS)
      {
        outDataSet.AddField(field);
      }
    }

    // Output data set with cell set containing extracted points
    vtkm::worklet::ExtractPoints extractPoints;
    OutCellSetType outCellSet;
    vtkm::cont::CellSetSingleType<> outputCellSet =
    outCellSet = extractPoints.Run(dataset.GetCellSet(0),
                                   sphere,
                                   dataset.GetCoordinateSystem("coords"),
                                   DeviceAdapter());
    outDataSet.AddCellSet(outCellSet);

    VTKM_TEST_ASSERT(test_equal(outCellSet.GetNumberOfCells(), 27), "Wrong result for ExtractPoints");
  }

  void TestExtractPointsWithBox() const
  {
    std::cout << "Testing extract points with implicit function (box):" << std::endl;

    typedef vtkm::cont::CellSetStructured<3> CellSetType;
    typedef vtkm::cont::CellSetSingleType<> OutCellSetType;

    // Input data set created
    vtkm::cont::DataSet dataset = MakeTestDataSet().Make3DUniformDataSet1();
    CellSetType cellset;
    dataset.GetCellSet(0).CopyTo(cellset);
  
    // Implicit function
    vtkm::Vec<vtkm::FloatDefault, 3> minPoint(1.0, 1.0, 1.0);
    vtkm::Vec<vtkm::FloatDefault, 3> maxPoint(3.0, 3.0, 3.0);
    vtkm::Box box(minPoint, maxPoint);
  
    // Output dataset contains input coordinate system and point data
    vtkm::cont::DataSet outDataSet;
    outDataSet.AddCoordinateSystem(dataset.GetCoordinateSystem(0));
    for (vtkm::Id indx = 0; indx < dataset.GetNumberOfFields(); indx++)
    {
      vtkm::cont::Field field = dataset.GetField(indx);
      if (field.GetAssociation() == vtkm::cont::Field::ASSOC_POINTS)
      {
        outDataSet.AddField(field);
      }
    }

    // Output data set with cell set containing extracted points
    vtkm::worklet::ExtractPoints extractPoints;
    OutCellSetType outCellSet;
    vtkm::cont::CellSetSingleType<> outputCellSet =
    outCellSet = extractPoints.Run(dataset.GetCellSet(0),
                                   box,
                                   dataset.GetCoordinateSystem("coords"),
                                   DeviceAdapter());
    outDataSet.AddCellSet(outCellSet);

    VTKM_TEST_ASSERT(test_equal(outCellSet.GetNumberOfCells(), 27), "Wrong result for ExtractPoints");
  }
  void operator()() const
  {
    this->TestExtractPointsWithSphere();
    this->TestExtractPointsWithBox();
  }
};

int UnitTestExtractPoints(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(
      TestingExtractPoints<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>());
}

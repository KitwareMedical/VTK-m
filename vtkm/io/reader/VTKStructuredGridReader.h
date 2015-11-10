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
#ifndef vtk_m_io_reader_VTKStructuredGridReader_h
#define vtk_m_io_reader_VTKStructuredGridReader_h

#include <vtkm/io/reader/VTKDataSetReaderBase.h>

namespace vtkm {
namespace io {
namespace reader {

class VTKStructuredGridReader : public VTKDataSetReaderBase
{
public:
  explicit VTKStructuredGridReader(const char *fileName)
    : VTKDataSetReaderBase(fileName)
  { }

private:
  virtual void Read()
  {
    if (this->DataFile->Structure != internal::DATASET_STRUCTURED_GRID)
    {
      throw vtkm::io::ErrorIO("Incorrect DataSet type");
    }

    std::string tag;

    // Read structured grid specific meta-data
    vtkm::Id3 dim;
    vtkm::Vec<vtkm::Float32, 3> origin, spacing;
    this->DataFile->Stream >> tag >> dim[0] >> dim[1] >> dim[2] >> std::ws;
    internal::parseAssert(tag == "DIMENSIONS");

    vtkm::cont::CellSetStructured<3> cs("cells");
    cs.SetPointDimensions(vtkm::make_Vec(dim[0], dim[1], dim[2]));
    this->DataSet.AddCellSet(cs);

    // Read the points
    this->ReadPoints();

    // Read points and cell attributes
    this->ReadAttributes();
  }
};

}
}
} // namespace vtkm::io:reader

#endif // vtk_m_io_reader_VTKStructuredGridReader_h

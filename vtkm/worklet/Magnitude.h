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
#ifndef vtk_m_worklet_Magnitude_h
#define vtk_m_worklet_Magnitude_h

#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/VectorAnalysis.h>

namespace vtkm
{
namespace worklet
{

class Magnitude : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<VecAll>, FieldOut<Scalar>);
  typedef void ExecutionSignature(_1, _2);

  template <typename T, typename T2>
  VTKM_EXEC void operator()(const T& inValue, T2& outValue) const
  {
    outValue = vtkm::Magnitude(inValue);
  }
};
}
} // namespace vtkm::worklet

#endif // vtk_m_worklet_Magnitude_h

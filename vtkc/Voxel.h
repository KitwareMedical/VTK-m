//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.md for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#ifndef vtk_c_Voxel_h
#define vtk_c_Voxel_h

#include <vtkc/ErrorCode.h>
#include <vtkc/Hexahedron.h>

#include <vtkc/internal/Common.h>

namespace vtkc
{

class Voxel : public Hexahedron
{
public:
  constexpr VTKC_EXEC Voxel() : Hexahedron(Cell(ShapeId::VOXEL, 8)) {}
  constexpr VTKC_EXEC explicit Voxel(const Cell& cell) : Hexahedron(cell) {}
};

namespace internal
{

template <typename Points, typename T>
VTKC_EXEC inline int getVoxelSpacing(const Points& points, T spacing[3])
{
  int zeros = 0;
  for (int i = 0; i < 3; ++i)
  {
    spacing[i] = static_cast<T>(points.getValue(6, i) - points.getValue(0, i));
    if (spacing[i] == T(0))
    {
      zeros |= 1 << i;
    }
  }
  return zeros;
}

} // internal

template <typename Points, typename Values, typename CoordType, typename Result>
VTKC_EXEC inline vtkc::ErrorCode derivative(
  Voxel,
  const Points& points,
  const Values& values,
  const CoordType& pcoords,
  Result&& dx,
  Result&& dy,
  Result&& dz) noexcept
{
  VTKC_STATIC_ASSERT_PCOORDS_IS_FLOAT_TYPE(CoordType);

  using ProcessingType = internal::ClosestFloatType<typename Values::ValueType>;
  using ResultCompType = ComponentType<Result>;

  ProcessingType spacing[3];
  if (internal::getVoxelSpacing(points, spacing) != 0)
  {
    return ErrorCode::DEGENERATE_CELL_DETECTED;
  }

  for (IdComponent c = 0; c < values.getNumberOfComponents(); ++c)
  {
    ProcessingType dvdp[3];
    internal::parametricDerivative(Hexahedron{}, values, c, pcoords, dvdp);

    component(dx, c) = static_cast<ResultCompType>(dvdp[0] / spacing[0]);
    component(dy, c) = static_cast<ResultCompType>(dvdp[1] / spacing[1]);
    component(dz, c) = static_cast<ResultCompType>(dvdp[2] / spacing[2]);
  }

  return ErrorCode::SUCCESS;
}

template <typename Points, typename PCoordType, typename WCoordType>
VTKC_EXEC vtkc::ErrorCode parametricToWorld(
  Voxel,
  const Points& points,
  const PCoordType& pcoords,
  WCoordType&& wcoords) noexcept
{
  VTKC_STATIC_ASSERT_PCOORDS_IS_FLOAT_TYPE(PCoordType);

  using T = ComponentType<WCoordType>;

  T spacing[3];
  if (internal::getVoxelSpacing(points, spacing) != 0)
  {
    return ErrorCode::DEGENERATE_CELL_DETECTED;
  }

  component(wcoords, 0) = points.getValue(0, 0) +
                          (spacing[0] * static_cast<T>(component(pcoords, 0)));
  component(wcoords, 1) = points.getValue(0, 1) +
                          (spacing[1] * static_cast<T>(component(pcoords, 1)));
  component(wcoords, 2) = points.getValue(0, 2) +
                          (spacing[2] * static_cast<T>(component(pcoords, 2)));
  return ErrorCode::SUCCESS;
}

template <typename Points, typename WCoordType, typename PCoordType>
VTKC_EXEC vtkc::ErrorCode worldToParametric(
  Voxel,
  const Points& points,
  const WCoordType& wcoords,
  PCoordType&& pcoords) noexcept
{
  VTKC_STATIC_ASSERT_PCOORDS_IS_FLOAT_TYPE(PCoordType);

  using T = ComponentType<PCoordType>;

  T spacing[3];
  if (internal::getVoxelSpacing(points, spacing) != 0)
  {
    return ErrorCode::DEGENERATE_CELL_DETECTED;
  }

  component(pcoords, 0) = static_cast<T>(component(wcoords, 0) - points.getValue(0, 0)) /
                          spacing[0];
  component(pcoords, 1) = static_cast<T>(component(wcoords, 1) - points.getValue(0, 1)) /
                          spacing[1];
  component(pcoords, 2) = static_cast<T>(component(wcoords, 2) - points.getValue(0, 2)) /
                          spacing[2];
  return ErrorCode::SUCCESS;
}

} //namespace vtkc

#endif //vtk_c_Voxel_h

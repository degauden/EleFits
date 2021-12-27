/**
 * @copyright (C) 2012-2020 Euclid Science Ground Segment
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3.0 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#ifndef _ELEFITSDATA_RASTER_H
#define _ELEFITSDATA_RASTER_H

#include "EleFitsData/DataContainer.h"
#include "EleFitsData/Position.h"
#include "EleFitsData/Region.h"

#include <complex>
#include <cstdint>
#include <string>
#include <vector>

namespace Euclid {
namespace Fits {

/**
 * @ingroup image_data_classes
 * @brief Loop over supported raster types.
 * @param MACRO A two-parameter macro: the C++ type and a valid variable name to represent it.
 * @see Program EleFitsPrintSupportedTypes to display all supported types
 * @see ELEFITS_FOREACH_RECORD_TYPE
 * @see ELEFITS_FOREACH_COLUMN_TYPE
 */
#define ELEFITS_FOREACH_RASTER_TYPE(MACRO) \
  MACRO(char, char) \
  MACRO(std::int16_t, int16) \
  MACRO(std::int32_t, int32) \
  MACRO(std::int64_t, int64) \
  MACRO(float, float) \
  MACRO(double, double) \
  MACRO(unsigned char, uchar) \
  MACRO(std::uint16_t, uint16) \
  MACRO(std::uint32_t, uint32) \
  MACRO(std::uint64_t, uint64)

// Forward declaration for Raster::subraster()
template <typename T, long N, typename TContainer>
class Subraster;

/**
 * @ingroup image_data_classes
 * @brief Raster of a _n_-dimensional image (2D by default).
 * @tparam T The value type, which can be `const`-qualified for read-only rasters
 * @tparam N The dimension, which can be >= 0 for fixed dimension, or -1 for variable dimension
 * @details
 * A raster is a contiguous container for the pixel data of an image.
 * It features access and view services.
 * 
 * This class is abstract, as the actual pixel container is not defined.
 * Two concrete implementations are provided:
 * - `PtrRaster` doesn's itself own data: it is just a shell which stores a shape, and a pointer to some actual data;
 * - `VecRaster` owns an `std::vector` (and is compatible with move semantics, which allows borrowing the `vector`).
 * 
 * There are two ways of defining the dimension of a `Raster`:
 * - when the dimension is knwon at compile-time,
 *   by giving the dimension parameter a positive or null value;
 * - when the dimension is known at run-time only,
 *   by assigning `N = -1`.
 * 
 * In the former case, index and size computations are optimized, and the dimension is enforced.
 * For example, it is not possible to read a 3D image HDU as a 2D `Raster`.
 * Which is nice, because an exception will be raised early!
 * In contrast, it is possible to read a 2D image HDU as a 3D `Raster` of third axis lenght =1.
 * 
 * In the latter case, the dimension may vary or be deduced from the file,
 * which is also nice sometimes but puts more responsibility on the shoulders of the user code,
 * as it should check that the returned dimension is acceptable.
 * 
 * `Raster` ensures constant-time access to elements, whatever the dimension of the data,
 * through subscipt operator `Raster::operator[]()`.
 * Bound checking and backward indexing (index <0) are enabled in `Raster::at()`.
 * 
 * Example usages:
 * \code
 * Position<2> shape {2, 3};
 * 
 * // Read/write Raster
 * float data[] = {1, 2, 3, 4, 5, 6};
 * Raster<float> raster(shape, data);
 * 
 * // Read-only Raster
 * const float cData[] = {1, 2, 3, 4, 5, 6};
 * Raster<const float> cRaster(shape, cData);
 * 
 * // Read/write VecRaster
 * std::vector<float> vec(&data[0], &data[6]);
 * VecRaster<float> vecRaster(shape, std::move(vec));
 * 
 * // Read-only VecRaster
 * std::vector<const float> cVec(&data[0], &data[6]);
 * VecRaster<const float> cVecRaster(shape, std::move(cVec));
 * \endcode
 * 
 * The raster data can be viewed region-wise as a `PtrRaster`,
 * given that the region is contiguous in memory.
 * Reading and writing non contiguous region is possible: see `ImageRaster`.
 * 
 * @note
 * Why "raster" and not simply image or array?
 * Mostly for disambiguation purpose:
 * "image" refers to the extension type, while "array" has already several meanings in C/C++.
 * `NdArray` would have been an option, but every related class should have been prefixed with `Nd` for homogeneity:
 * `NdPosition`, `NdRegion`, `VecNdArray`...
 * From the cathodic television era, raster also historically carries the concept of contiguous pixels,
 * is very common in the field of Earth observation,
 * and also belongs to the Java library.
 * All in all, `Raster` seems to be a fair compromise.
 * 
 * @see Position for details on the fixed- and variable-dimension cases.
 * @see makeRaster() for creation shortcuts.
 */
template <typename T, long N, typename TContainer>
class Raster;

/**
 * @ingroup image_data_classes
 * @brief `Raster` which points to some external data.
 * @copydetails Raster
 */
template <typename T, long N = 2>
using PtrRaster = Raster<T, N, T*>;

/**
 * @ingroup image_data_classes
 * @brief `Raster` which owns the data as an `std::vector`.
 * @copydetails Raster
 */
template <typename T, long N = 2>
using VecRaster = Raster<T, N, std::vector<T>>;

template <typename T, long N, typename TContainer>
class Raster : public DataContainerBase<T, TContainer, Raster<T, N, TContainer>> {
  friend class ImageRaster; // FIXME rm when Subraster is removed

public:
  /**
   * @brief The pixel value type.
   */
  using Value = T;

  /**
   * @brief The dimension template parameter.
   * @details
   * The value of `Raster<T, N>::Dim` is always `N`, irrespective of its sign.
   * In contrast, dimension() provides the actual dimension of the Raster,
   * even in the case of a variable dimension.
   */
  static constexpr long Dim = N;

  /**
   * @name Constructors
   */
  /// @{

  ELEFITS_VIRTUAL_DTOR(Raster)
  ELEFITS_COPYABLE(Raster)
  ELEFITS_MOVABLE(Raster)

  /**
   * @brief Constructor.
   * @param shape The raster shape
   */
  Raster(Position<N> shape) :
      DataContainerBase<T, TContainer, Raster<T, N, TContainer>>(
          ContainerAllocator<TContainer>::alloc(shapeSize(shape))),
      m_shape(std::move(shape)) {}

  /**
   * @brief Constructor.
   * @param shape The raster shape
   * @param args Arguments to be forwarded to the container
   */
  template <typename... Ts>
  Raster(Position<N> shape, Ts&&... args) :
      DataContainerBase<T, TContainer, Raster<T, N, TContainer>>(std::forward<Ts>(args)...), m_shape(std::move(shape)) {
  }

  /// @}
  /**
   * @name Size
   */
  /// @{

  /**
   * @brief Get the raster shape.
   */
  const Position<N>& shape() const;

  /**
   * @brief Get raster domain.
   * @details
   * The domain is the region which spans from the first to the last pixel position.
   * It can be used to loop over all pixels, e.g.:
   * \code
   * for (auto pos : raster.domain()) {
   *   processPixel(pos, raster[pos]);
   * }
   * \endcode
   */
  Region<N> domain() const;

  /**
   * @brief Dimension.
   * @details
   * This corresponds to the `N` template parameter in general,
   * or to the current dimension if variable.
   */
  long dimension() const;

  /**
   * @brief Number of pixels.
   */
  long size() const;

  /**
   * @brief Length along given axis.
   */
  template <long I>
  long length() const;

  /// @}
  /**
   * @name Element access
   */
  /// @{

  using DataContainerBase<T, TContainer, Raster<T, N, TContainer>>::operator[];

  /**
   * @brief Pixel at given index.
   */
  // const T& operator[](long index) const;

  /**
   * @brief Pixel at given index.
   */
  // T& operator[](long index);

  /**
   * @brief Raw index of a position.
   */
  long index(const Position<N>& pos) const;

  /**
   * @brief Pixel at given position.
   */
  const T& operator[](const Position<N>& pos) const;

  /**
   * @brief Pixel at given position.
   */
  T& operator[](const Position<N>& pos);

  /**
   * @brief Access the value at given position.
   * @details
   * As opposed to `operator[]()`, negative indices are supported for backward indexing,
   * and bounds are checked.
   * @see operator[]()
   */
  const T& at(const Position<N>& pos) const;

  /**
   * @copydoc at()
   */
  T& at(const Position<N>& pos);

  /// @}
  /**
   * @name Views
   */
  /// @{

  /**
   * @brief Create a slice from a given region.
   * @tparam M The dimension of the slice (cannot be -1)
   * @see isContiguous()
   * @see section()
   */
  template <long M = 2>
  const PtrRaster<const T, M> slice(const Region<N>& region) const;

  /**
   * @copydoc slice()
   */
  template <long M = 2>
  PtrRaster<T, M> slice(const Region<N>& region);

  /**
   * @brief Create a section at given index.
   * @param front The section front index along the last axis
   * @param back The section back index along the last axis
   * @param index The section index along the last axis
   * @details
   * A section is a maximal slice of dimension `N` or `N`-1.
   * For example, a 3D section of a 3D raster of shape (x, y, z)
   * is a 3D raster of shape (x, y, t) where `t` < `z`,
   * while a 2D section of it is a 2D raster of shape (x, y).
   * 
   * If needed, `section()` can be applied recursively,
   * e.g. to get the x-line at `z` = 4 and `y` = 2:
   * \code
   * auto line = raster.section(4).section(2);
   * \endcode
   * 
   * @see slice()
   */
  const PtrRaster<const T, N> section(long front, long back) const;

  /**
   * @copydoc section()
   */
  PtrRaster<T, N> section(long front, long back);

  /**
   * @copydoc section()
   */
  const PtrRaster<const T, N == -1 ? -1 : N - 1> section(long index) const;

  /**
   * @copydoc section()
   */
  PtrRaster<T, N == -1 ? -1 : N - 1> section(long index);

  /**
   * @brief Check whether a region is made of contiguous values in memory.
   * @tparam M The actual region dimension
   * @details
   * A region is contiguous if and only if:
   * - For `i` < `M-1`, `front[i]` = 0 and `back[i]` = -1;
   * - For `i` > `M`, `front[i]` = `back[i]`.
   */
  template <long M = 2>
  bool isContiguous(const Region<N>& region) const;

  /// @}

private:
  /**
   * @brief Create a subraster from given region.
   * @details
   * A subraster is a view of the raster data contained in a region.
   * As opposed to a slice or a section, a subraster is not necessarily contiguous in memory.
   * @see isContiguous()
   * @see slice()
   * @see section()
   */
  const Subraster<T, N, TContainer> subraster(const Region<N>& region) const; // FIXME rm?

  /**
   * @copydoc subraster().
   */
  Subraster<T, N, TContainer> subraster(const Region<N>& region); // FIXME rm?

  /**
   * @brief Raster shape, i.e. length along each axis.
   * @warning Will be made private and accessed through method `shape()` in 4.0
   */
  Position<N> m_shape;
};

/**
 * @ingroup image_data_classes
 * @brief Shortcut to create a raster from a shape and data without specifying the template parameters.
 * @tparam T The pixel type, should not be specified (automatically deduced)
 * @tparam Longs The axes lengths, should not be specified (automatically deduced)
 * @param data The raster values, which can be either a pointer (or C array) or a vector
 * @param shape The shape as a comma-separated list of `long`s
 * @details
 * Example usages:
 * \code
 * Given:
 * - long width, height, depth: The axes lengths;
 * - float* ptr: The pixel values as a pointer;
 * - std::vector<float> vec: The pixel values as a vector;
 * 
 * auto ptrRaster2D = makeRaster(ptr, width, height);
 * auto ptrRaster3D = makeRaster(ptr, width, height, depth);
 * auto vecRaster2D = makeRaster(vec, width, height); // The vector is copied
 * auto vecRaster3D = makeRaster(std::move(vec), width, height, depth); // The vector is moved
 * \endcode
 */
template <typename T, typename... Longs>
PtrRaster<T, sizeof...(Longs)> makeRaster(T* data, Longs... shape) {
  return {{shape...}, data};
}

/**
 * @ingroup image_data_classes
 * @copydoc makeRaster
 */
template <typename T, typename... Longs>
VecRaster<T, sizeof...(Longs)> makeRaster(std::vector<T> data, Longs... shape) {
  return {{shape...}, std::move(data)};
}

} // namespace Fits
} // namespace Euclid

#include "EleFitsData/PositionIterator.h"
#include "EleFitsData/Subraster.h"

/// @cond INTERNAL
#define _ELEFITSDATA_RASTER_IMPL
#include "EleFitsData/impl/Raster.hpp"
#undef _ELEFITSDATA_RASTER_IMPL
/// @endcond

#endif

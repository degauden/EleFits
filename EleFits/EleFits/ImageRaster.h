// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _ELEFITS_IMAGERASTER_H
#define _ELEFITS_IMAGERASTER_H

#include "EleFits/FileMemRegions.h"
#include "EleFitsData/Raster.h"

#include <fitsio.h>
#include <functional>

namespace Euclid {
namespace Fits {

/**
 * @ingroup image_handlers
 * @brief Reader-writer for the image data unit.
 * @details
 * This handler provides methods to access image metadata (image-related keyword records) and data.
 * 
 * Reading methods either return a `VecRaster` or fill an existing `Raster`.
 * 
 * Data can be read and written region-wise.
 * Source and destination regions are specified by a `FileMemRegions` object.
 * 
 * @see Raster
 * @see Subraster
 */
class ImageRaster {
private:
  friend class ImageHdu;

  /**
   * @brief Constructor.
   */
  ImageRaster(fitsfile*& fptr, std::function<void(void)> touch, std::function<void(void)> edit);

public:
  /**
   * @name Image properties.
   */
  /// @{

  /**
   * @brief Read the image pixel value type.
   */
  const std::type_info& read_typeid() const;

  /**
   * @brief Read the `BITPIX` or `ZBITPIX` value.
   */
  long read_bitpix() const;

  /**
   * @brief Read the number of pixels in the image.
   */
  long read_size() const;

  /**
   * @brief Read the image shape.
   */
  template <long N = 2>
  Position<N> read_shape() const;

  /**
   * @brief Update the image shape.
   */
  template <long N = 2>
  void update_shape(const Position<N>& shape) const;

  /**
   * @brief Update the image type and shape.
   */
  template <typename T, long N = 2>
  void update_type_shape(const Position<N>& shape) const;

  /// @}
  /**
   * @name Read the whole data unit.
   */
  /// @{

  /**
   * @brief Read the whole data unit as a new `VecRaster`.
   * @details
   * There are several options to read the whole data unit:
   * - as a new `VecRaster` object;
   * - by filling an existing `Raster` object;
   * - by filling an existing `Subraster` object.
   * 
   * In the last two cases, the raster or subraster is assumed to already have a conforming shape.
   * 
   * @warning
   * Filling a `Subraster` is much slower than filling a `Raster`.
   */
  template <typename T, long N = 2>
  VecRaster<T, N> read() const;

  /**
   * @brief Read the whole data unit into an existing `Raster`.
   * @copydetails read()
   */
  template <typename TRaster>
  void read_to(TRaster& raster) const;

  /// @}
  /**
   * @name Read a region of the data unit.
   */
  /// @{

  /**
   * @brief Read a region as a new `VecRaster`.
   * @tparam T The desired raster type
   * @tparam M The desired raster dimension, which can be smaller than the data dimension in file
   * @tparam N The region dimension, which corresponds to the data dimension in file
   * @param region The in-file region
   * @param regions The in-file and in-memory regions
   * @param raster The destination raster
   * @details
   * There are several options to read a region of the data unit:
   * - as a new `VecRaster` object;
   * - by filling an existing `Raster` object;
   * - by filling an existing `Subraster` object.
   * In the last two cases, the in-file and in-memory regions are given as a `FileMemRegions` object.
   * 
   * For example, to read the HDU region from position (50, 80) to position (100, 120)
   * into an existing raster at position (25, 40), do:
   * \code
   * const FileMemRegions<2> regions({25, 40}, {{50, 80}, {100, 120}});
   * image.read_region_to(regions, raster);
   * \endcode
   * where `image` is the `ImageRaster` and `raster` is the `Raster`.
   * 
   * In simpler cases, where the in-file or in-memory front position is 0,
   * factories can be used, e.g. to read into position 0 of the raster:
   * \code
   * image.read_region_to<2>({{50, 80}, {100, 120}}, raster);
   * \endcode
   */
  template <typename T, long M, long N>
  VecRaster<T, M> read_region(const Region<N>& region) const;

  /**
   * @brief Read a region of the data unit into a region of an existing `Raster`.
   * @copydetails read_region()
   */
  template <typename TRaster>
  void read_region_to(FileMemRegions<TRaster::Dim> regions, TRaster& raster) const;

  /// @}
  /**
   * @name Write the whole data unit.
   */
  /// @{

  /**
   * @brief Write the whole data unit.
   */
  template <typename TRaster>
  void write(const TRaster& raster) const;

  /// @}
  /**
   * @name Write a region of the data unit.
   */
  /// @{

  /**
   * @brief Write a `Raster` at a given position of the data unit.
   * @param regions The in-file and in-memory regions
   * @param raster The raster to be written
   * @details
   * In-file and in-memory (raster) regions are specified as the first parameter.
   * Max bounds (-1) can be used in one, several, or all axes.
   * Shortcuts offered by `FileMemRegions` and `Region` can be used to implement special cases:
   * \code
   * // Write the whole raster at position (10, 20, 30)
   * du.write_region<3>({10, 20, 30}, raster);
   * 
   * // Write the whole HDU with a region of the raster starting at (10, 20, 30)
   * du.write_region<3>({Region<3>::whole(), {10, 20, 30}}, raster);
   * \endcode
   * 
   * Note that the raster dimension can be lower than the HDU dimension.
   * For example, it is possible to write a 2D raster in a 3D HDU.
   * \code
   * // Write the 3rd plane of raster into the 5th plane of the HDU
   * du.write_region<3>({{0, 0, 4}}, raster.section(2));
   * \endcode
   */
  template <typename TRaster, long N>
  void write_region(FileMemRegions<N> regions, const TRaster& raster) const; // TODO return bool = is_contiguous()?

  /// @}

  /**
   * @deprecated
   */
  const std::type_info& readTypeid() const {
    return read_typeid();
  }

  /**
   * @deprecated
   */
  long readSize() const {
    return read_size();
  }

  template <long N = 2>
  Position<N> readShape() const {
    return read_shape<2>();
  }

  /**
   * @deprecated
   */
  template <long N = 2>
  void updateShape(const Position<N>& shape) const {
    return update_shape(shape);
  }

  /**
   * @deprecated
  */
  template <typename T, long N = 2>
  [[deprecated("Use update_type_shape()")]] void reinit(const Position<N>& shape) const {
    return update_type_shape<T>(shape);
  }

  /**
   * @deprecated
   */
  template <typename T, long M, long N>
  VecRaster<T, M> readRegion(const Region<N>& region) const {
    read_region<T, M>(region);
  }

  /**
   * @deprecated
   */
  template <typename TRaster>
  void readRegionTo(FileMemRegions<TRaster::Dim> regions, TRaster& raster) const {
    return read_region_to(regions, raster);
  }

  /**
   * @deprecated
   */
  template <typename TRaster, long N>
  void writeRegion(FileMemRegions<N> regions, const TRaster& raster) const {
    return write_region(regions, raster);
  }

private:
  /**
   * @brief Read a region of the data unit into an existing `Raster`.
   * @copydetails read_region()
   */
  template <typename TRaster, long N>
  void read_region_to_slice(const Position<N>& front_position, TRaster& raster) const;

  /**
   * @brief Read a region of the data unit into an existing `Subraster`.
   * @copydetails read_region()
   */
  template <typename T, long M, long N, typename TContainer>
  void read_region_to_subraster(const Position<N>& front_position, Subraster<T, M, TContainer>& subraster)
      const; // FIXME rm?

  /**
   * @brief Read a region of the data unit into an existing `Subraster`.
   * @details
   * The region is that of the subraster.
   * This function is equivalent to:
   * \code
   * read_region_to({region, region}, raster);
   * \endcode
   * where `region` would be the subraster region, and `raster` the subraster parent.
   */
  template <typename T, long N, typename TContainer>
  void read_region_to(Subraster<T, N, TContainer>& subraster) const; // FIXME rm?

  /**
   * @brief Write a `Raster` at a given position of the data unit.
   */
  template <typename TRaster, long N>
  void write_slice(const Position<N>& front_position, const TRaster& raster) const;

  /**
   * @brief Write a `Subraster` at a corresponding position of the data unit.
   * @copydetails write_region()
   */
  template <typename T, long N, typename TContainer>
  void write_region(const Subraster<T, N, TContainer>& subraster) const; // FIXME rm?

  /**
   * @brief Write a `Subraster` at a given position of the data unit.
   */
  template <typename T, long M, long N, typename TContainer>
  void
  write_subraster(const Position<N>& front_position, const Subraster<T, M, TContainer>& subraster) const; // FIXME rm?

private:
  /**
   * @brief The fitsfile.
   */
  fitsfile*& m_fptr;

  /**
   * @brief The function to declare that the header was touched.
   */
  std::function<void(void)> m_touch;

  /**
   * @brief The function to declare that the header was edited.
   */
  std::function<void(void)> m_edit;
};

} // namespace Fits
} // namespace Euclid

/// @cond INTERNAL
#define _ELEFITS_IMAGERASTER_IMPL
#include "EleFits/impl/ImageRaster.hpp"
#undef _ELEFITS_IMAGERASTER_IMPL
/// @endcond

#endif

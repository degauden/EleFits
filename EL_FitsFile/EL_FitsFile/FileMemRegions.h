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

#ifndef _EL_FITSFILE_FILEMEMREGIONS_H
#define _EL_FITSFILE_FILEMEMREGIONS_H

#include "EL_FitsData/Region.h"

namespace Euclid {
namespace FitsIO {

/**
 * @ingroup image_handlers
 * @brief Mapping between an in-file (Fits data unit) region and an in-memory (raster or column) region
 * for reading and writing data unit regions.
 * @warning
 * In-file and in-memory regions have the same shape.
 * Both back positions cannot be -1 at the same index at the same time.
 */
template <long n = 2>
class FileMemRegions {

public:
  /**
   * @brief Create a mapping from an in-file region and an in-memory position.
   */
  FileMemRegions(const Region<n>& fileRegion, const Position<n>& memoryPosition = Position<n>::zero()) :
      m_file(fileRegion), m_memory(Region<n>::fromShape(memoryPosition, fileRegion.shape())) {
    if (m_file.back.isMax()) {
      m_memory.back = Position<n>::zero();
    }
  }
  // Not explicit to allow casting, e.g. readRegionTo(region, raster)

  /**
   * @brief Create a mapping from an in-file position and an in-memory region.
   */
  FileMemRegions(const Position<n>& filePosition, const Region<n>& memoryRegion = Region<n>::whole()) :
      m_file(Region<n>::fromShape(filePosition, memoryRegion.shape())), m_memory(memoryRegion) {
    if (m_memory.back.isMax()) {
      m_file.back = Position<n>::zero();
    }
  }

  /**
   * @brief Get the in-file region.
   * @details
   * See the warning at class level.
   */
  const Region<n>& file() const {
    return m_file;
  }

  /**
   * @brief Get the in-memory region.
   * @details
   * See the warning at class level.
   */
  const Region<n>& memory() const {
    return m_memory;
  }

  void resolve(const Position<n>& fileBack, const Position<n>& memoryBack) { // FIXME back or shape?
    const auto ftom = fileToMemory();
    for (auto fit = m_file.back.begin(),
              fitEnd = m_file.back.end(),
              mit = m_memory.back.begin(),
              fbit = fileBack.begin(),
              mbit = memoryBack.begin(),
              ftomit = ftom.begin();
         fit != fitEnd;
         ++fit, ++mit, ++fbit, ++mbit, ++ftomit) {
      // TODO check that fit and mit are not -1 at the same time? In Ctor?
      if (*fit == -1) {
        *fit = *fbit;
        *mit = *fit + *ftomit;
      } else if (*mit == -1) {
        *mit = *mbit;
        *fit = *mit - *ftomit;
      }
    }
  }

  /**
   * @brief Compute the translation vector from in-file region to in-memory region.
   */
  Position<n> fileToMemory() const {
    return m_memory.front - m_file.front;
  }

  /**
   * @brief Compute the translation vector from in-memory region to in-file region.
   */
  Position<n> memoryToFile() const {
    return m_file.front - m_memory.front;
  }

private:
  /**
   * @brief The in-file region.
   */
  Region<n> m_file;

  /**
   * @brief The in-memory region.
   */
  Region<n> m_memory;
};

/**
 * @ingroup handlers
 * @brief Create a `FileMemRegions` with in-file region at origin.
 */
template <long n>
FileMemRegions<n> makeMemRegion(const Region<n>& memoryRegion) {
  return FileMemRegions<n>(Position<n>::zero(), memoryRegion);
}

/**
 * @ingroup handlers
 * @brief Create a `FileMemRegions` with in-memory region at origin.
 */
template <long n>
FileMemRegions<n> makeFileRegion(const Region<n>& fileRegion) {
  return FileMemRegions<n>(fileRegion, Position<n>::zero());
}

} // namespace FitsIO
} // namespace Euclid

#endif

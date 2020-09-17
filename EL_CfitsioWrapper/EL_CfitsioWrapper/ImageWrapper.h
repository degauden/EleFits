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

#ifndef _EL_CFITSIOWRAPPER_IMAGEWRAPPER_H
#define _EL_CFITSIOWRAPPER_IMAGEWRAPPER_H

#include <fitsio.h>
#include <string>

#include "EL_FitsData/Raster.h"

#include "ErrorWrapper.h"
#include "FileWrapper.h"
#include "TypeWrapper.h"

namespace Euclid {
namespace Cfitsio {

/**
 * @brief Image-related functions.
 */
namespace Image {

/**
 * @brief Resize the Raster of the current Image HDU.
 */
template <typename T, long n = 2>
void resize(fitsfile *fptr, const FitsIO::Position<n> &shape);

/**
 * @brief Read a Raster in current Image HDU.
 */
template <typename T, long n = 2>
FitsIO::VecRaster<T, n> readRaster(fitsfile *fptr);

/**
 * @brief Write a Raster in current Image HDU.
 */
template <typename T, long n = 2>
void writeRaster(fitsfile *fptr, const FitsIO::Raster<T, n> &raster);

} // namespace Image
} // namespace Cfitsio
} // namespace Euclid

#include "impl/ImageWrapper.hpp"

#endif

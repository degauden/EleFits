// Copyright (C) 2019-2023, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMPRESSIONWRAPPER_H
#define COMPRESSIONWRAPPER_H

#include "EleFitsData/Compression.h"

#include <fitsio.h>
#include <memory> // unique_ptr
#include <string>

namespace Euclid {
namespace Cfitsio {

/**
 * @brief Know if the cfitsio image compression is turned on.
 */
inline bool isCompressing(fitsfile* fptr);

/**
 * @brief Read the current compression parameters.
 */
inline std::unique_ptr<Fits::Compression> readCompression(fitsfile* fptr);

/**
 * @brief Read the current compression tiling.
 */
inline Fits::Position<-1> readCompressionTiling(fitsfile* fptr);

/**
 * @brief Set the compression algorithm to NoCompression.
 */
inline void compress(fitsfile* fptr, const Fits::NoCompression& algo);

/**
 * @brief Set the compression algorithm to Rice.
 */
inline void compress(fitsfile* fptr, const Fits::Rice& algo);

/**
 * @brief Set the compression algorithm to HCompress.
 */
inline void compress(fitsfile* fptr, const Fits::HCompress& algo);

/**
 * @brief Set the compression algorithm to Plio.
 */
inline void compress(fitsfile* fptr, const Fits::Plio& algo);

/**
 * @brief Set the compression algorithm to Gzip.
 */
inline void compress(fitsfile* fptr, const Fits::Gzip& algo);

/**
 * @brief Set the compression algorithm to ShuffledGzip.
 */
inline void compress(fitsfile* fptr, const Fits::ShuffledGzip& algo);

} // namespace Cfitsio
} // namespace Euclid

/// @cond INTERNAL
#define COMPRESSIONWRAPPER_IMPL
#include "EleCfitsioWrapper/impl/CompressionWrapper.hpp"
#undef COMPRESSIONWRAPPER_IMPL
/// @endcond

#endif
// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#if defined(_ELEFITS_IMAGERASTER_IMPL) || defined(CHECK_QUALITY)

#include "EleCfitsioWrapper/ImageWrapper.h"
#include "EleFits/ImageRaster.h"

namespace Euclid {
namespace Fits {

template <long N>
Position<N> ImageRaster::read_shape() const {
  m_edit();
  return Cfitsio::ImageIo::read_shape<N>(m_fptr);
}

template <long N>
void ImageRaster::update_shape(const Position<N>& shape) const {
  m_edit();
  Cfitsio::ImageIo::update_shape<N>(m_fptr, shape);
}

template <typename T, long N>
void ImageRaster::update_type_shape(const Position<N>& shape) const {
  m_edit();
  Cfitsio::ImageIo::update_type_shape<T, N>(m_fptr, shape);
}

template <typename T, long N>
VecRaster<T, N> ImageRaster::read() const {
  VecRaster<T, N> raster(read_shape<N>());
  read_to(raster);
  return raster;
}

template <typename TRaster>
void ImageRaster::read_to(TRaster& raster) const {
  m_touch();
  Cfitsio::ImageIo::read_raster_to(m_fptr, raster);
}

template <typename T, long M, long N>
VecRaster<T, M> ImageRaster::read_region(const Region<N>& region) const {
  VecRaster<T, M> raster(region.shape().template slice<M>());
  read_region_to(region.front, raster);
  return raster;
}

template <typename TRaster>
void ImageRaster::read_region_to(FileMemRegions<TRaster::Dim> regions, TRaster& raster) const {
  const auto& mem_region = regions.inMemory();
  if (raster.isContiguous(mem_region)) {
    read_region_to_slice(regions.inFile().front, raster.slice(mem_region));
  } else {
    read_region_to_subraster(regions.inFile().front, raster.subraster(mem_region));
  }
}

template <typename T, long N, typename TContainer>
void ImageRaster::read_region_to(Subraster<T, N, TContainer>& subraster) const {
  read_region_to_subraster(subraster.region().front, subraster);
}

template <typename TRaster, long N>
void ImageRaster::read_region_to_slice(const Position<N>& front_position, TRaster& raster) const {
  m_touch();
  Cfitsio::ImageIo::read_region_to(
      m_fptr,
      Region<N>::fromShape(front_position, raster.shape()), // FIXME use front_position in ImageIo
      raster);
}

template <typename T, long M, long N, typename TContainer>
void ImageRaster::read_region_to_subraster(const Position<N>& front_position, Subraster<T, M, TContainer>& subraster)
    const {
  m_touch();
  Cfitsio::ImageIo::read_region_to(
      m_fptr,
      Region<N>::fromShape(front_position, subraster.shape()), // FIXME use front_position in ImageIo
      subraster);
  // FIXME move algo here, rely solely on read_region_to(fitsfile, Position, Raster)
}

template <typename TRaster>
void ImageRaster::write(const TRaster& raster) const {
  m_edit();
  Cfitsio::ImageIo::write_raster(m_fptr, raster);
}

template <typename TRaster, long N>
void ImageRaster::write_region(FileMemRegions<N> regions, const TRaster& raster) const {
  regions.resolve(read_shape<N>() - 1, raster.shape() - 1);
  if (raster.isContiguous(regions.memory())) {
    write_slice(regions.file().front, raster.slice(regions.memory()));
  } else {
    write_subraster(regions.file().front, raster.subraster(regions.memory()));
  }
}

template <typename T, long N, typename TContainer>
void ImageRaster::write_region(const Subraster<T, N, TContainer>& subraster) const {
  write_region(subraster.region().front, subraster);
}

template <typename TRaster, long N>
void ImageRaster::write_slice(const Position<N>& front_position, const TRaster& raster) const {
  Cfitsio::ImageIo::write_region(m_fptr, raster, front_position);
}

template <typename T, long M, long N, typename TContainer>
void ImageRaster::write_subraster(const Position<N>& front_position, const Subraster<T, M, TContainer>& subraster)
    const {
  m_edit();
  int status = 0;
  auto locus = Region<M>::fromShape(Position<M>::zero(), subraster.shape());
  locus.back[0] = locus.front[0];
  const auto nelem = subraster.shape()[0];
  const auto delta = front_position.template slice<M>();
  Position<N> target;
  for (const auto& source : locus) {
    target = (source + delta).extend(front_position) + 1; // 1-based
    const auto b = &subraster[source];
    const auto e = b + nelem;
    std::vector<std::decay_t<T>> nonconst_data(b, e);
    fits_write_pix(m_fptr, Cfitsio::TypeCode<T>::for_image(), target.data(), nelem, nonconst_data.data(), &status);
    // TODO to ImageWrapper
  }
}

} // namespace Fits
} // namespace Euclid

#endif

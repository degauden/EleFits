// Copyright (C) 2019-2023, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "EleFitsData/DataUtils.h"
#include "EleFitsData/Position.h"

#include <memory>
#include <string>

namespace Euclid {
namespace Fits {

/// @cond
// Forward declaration for friendship in Algo
class MefFile;
/// @endcond

/**
 * @ingroup image_compression
 * @brief FITS-internal, tiled compression of Image HDUs.
 */
namespace Compression {

/**
 * @relates AlgoMixin
 * @brief Create a rowwise tiling.
 * @param rowCount The number of rows per tile
 */
inline Position<-1> rowwiseTiling(long rowCount = 1) {
  return Position<-1> {-1, rowCount};
}

/**
 * @relates AlgoMixin
 * @brief Create a whole-data array tiling.
 */
inline Position<-1> wholeDataTiling() {
  return Position<-1> {-1};
}

/**
 * @ingroup image_compression
 * @brief A factor which can be absolute or relative to the noise level in each tile.
 * 
 * A `relative()` factor yields: `absolute() = RMS_noise / relative()`.
 * A `none()` factor can be used to disable the feature it represents.
 */
// FIXME for q, absolute = RMS / relative
// but for s, absolue = RMS * relative
class Factor {

public:
  /**
   * @brief The type of factor.
   */
  enum Type {
    None, ///< Disable feature
    Absolute, ///< Absolute value
    Relative ///< Tile-relative value
  };

  /**
   * @brief Create a disabled factor.
   */
  static inline Factor none();

  /**
   * @brief Create an absolute factor.
   */
  static inline Factor absolute(float value);

  /**
   * @brief Create a relative factor.
   */
  static inline Factor relative(float value);

  /**
   * @brief Get the factor type.
   */
  inline Factor::Type type() const;

  /**
   * @brief Get the factor value.
   */
  inline float value() const;

  /**
   * @brief Check whether two factors are equal.
   */
  inline bool operator==(const Factor& rhs) const;

private:
  /**
   * @brief Create a factor.
   */
  inline Factor(float value);

  /**
   * @brief The factor value, which encodes the type as its sign.
   */
  float m_value;
};

/**
 * @ingroup image_compression
 * @brief Quantization dithering methods.
 */
enum class Dithering {
  None, ///< Do not dither any pixel
  NonZeroPixel, ///< Dither only non-zero pixels
  EveryPixel ///< Dither all pixels
};

/**
 * @ingroup image_compression
 * @brief Quantization of floating-point pixels.
 */
class Quantization {

public:
  /**
   * @brief Constructor.
   * 
   * Integer data compression is lossless by default.
   */
  inline Quantization(Factor level = Factor::relative(4), Dithering method = Dithering::EveryPixel);

  /**
   * @brief Set the quantization level.
   */
  inline Quantization& level(Factor level);

  /**
   * @brief Set the dithering method.
   */
  inline Quantization& dithering(Dithering method);

  /**
   * @brief Enable lossy compression of integer data.
   * 
   * This is accomplished by considering the values as floating points and then applying quantization.
   */
  inline Quantization& enableLossyInt();

  /**
   * @brief Disable lossy compression of integer data.
   */
  inline Quantization& disableLossyInt();

  /**
   * @brief Get the quantization level
   */
  inline const Factor& level() const;

  /**
   * @brief Get the dithering method for the quantization.
   */
  inline Dithering dithering() const;

  /**
   * @brief Check whether lossy integral compression is enabled.
   */
  inline bool hasLossyInt() const;

  /**
   * @brief Check whether two quatizations are equal.
   */
  inline bool operator==(const Quantization& rhs) const;

private:
  /**
   * @brief The quantization level.
   */
  Factor m_level;

  /**
   * @brief The quantization dithering method.
   */
  Dithering m_dithering;

  /**
   * @brief The lossy integral compression flag.
   */
  bool m_lossyInt;

  // FIXME handle dither offset
};

/**
 * @ingroup image_compression
 * @brief Interface for compression algorithms.
 */
class Algo {

  friend class Euclid::Fits::MefFile; // TODO rm if/when possible

public:
  Algo() = default;
  ELEFITS_VIRTUAL_DTOR(Algo)
  ELEFITS_COPYABLE(Algo)
  ELEFITS_MOVABLE(Algo)

protected:
  virtual void compress(void* fptr) const = 0;
};

/**
 * @ingroup image_compression
 * @brief Intermediate class holding the tiling shape and quantization parameters.
 * 
 * Tiling shape is represented as a `Position<N>`.
 * The maximum dimension possible is equal to 6 (which is an internal CFITSIO limitation).
 * The value of `N` must therefore not exceed it.
 * 
 * @see rowwiseTiling()
 * @see wholeDataTiling()
 */
template <typename TDerived> // FIXME rm N?
class AlgoMixin : public Algo {

public:
  ELEFITS_VIRTUAL_DTOR(AlgoMixin)
  ELEFITS_COPYABLE(AlgoMixin)
  ELEFITS_MOVABLE(AlgoMixin)

  /**
   * @brief Get the tiling.
   */
  const Position<-1>& shape() const;

  /**
   * @brief Get the quantization.
   */
  const Quantization& quantization() const; // FIXME move to children

  /**
   * @brief Set the tiling.
   */
  TDerived& shape(Position<-1> shape);

  /**
   * @brief Set the quantization.
   */
  TDerived& quantization(Quantization quantization);
  // FIXME HCompress does not support NonZeroPixel dithering
  // and therefore this method must be moved to the child classes
  // with specific check in HCompress

protected:
  /**
   * @brief Constructor.
   */
  AlgoMixin(Position<-1> shape);

  /**
   * @brief Dependency inversion to call the wrapper's dispatch based on `TDerived`.
   */
  void compress(void* fptr) const final;
  // FIXME define the function here instead of in the wrapper
  // The function is not used anyway, and this simplifies compilation
  // => Change compress(fitsfile*, TDerived) into compress(void*, TDerived)
  // or forward declare fitsfile

private:
  /**
   * @brief The shape of the tiles.
   */
  Position<-1> m_shape;

  /**
   * @brief The quantization parameters.
   */
  Quantization m_quantization;
};

/**
 * @ingroup image_compression
 * @brief No compression.
 */
class None : public AlgoMixin<None> {

public:
  ELEFITS_VIRTUAL_DTOR(None)
  ELEFITS_COPYABLE(None)
  ELEFITS_MOVABLE(None)

  /**
   * @brief Constructor.
   */
  inline None();
};

/**
 * @ingroup image_compression
 * @brief The Rice algorithm.
 */
class Rice : public AlgoMixin<Rice> {

public:
  ELEFITS_VIRTUAL_DTOR(Rice)
  ELEFITS_COPYABLE(Rice)
  ELEFITS_MOVABLE(Rice)

  /**
   * @brief Constructor.
   */
  inline Rice(Position<-1> shape = rowwiseTiling());
};

/**
 * @ingroup image_compression
 * @brief The HCompress algorithm.
 */
class HCompress : public AlgoMixin<HCompress> {

public:
  ELEFITS_VIRTUAL_DTOR(HCompress)
  ELEFITS_COPYABLE(HCompress)
  ELEFITS_MOVABLE(HCompress)
  /**
   * @brief Constructor.
   */
  inline HCompress(Position<-1> shape = rowwiseTiling(16));

  /**
   * @brief Get the scaling factor.
   */
  inline const Factor& scale() const;

  /**
   * @brief Check whether the image is smoothed at reading.
   */
  inline bool isSmooth() const;

  /**
   * @brief Set the scaling factor.
   */
  inline HCompress& scale(Factor scale);

  /**
   * @brief Enable image smoothing at reading.
   */
  inline HCompress& enableSmoothing();

  /**
   * @brief Disable image smoothing at reading.
   */
  inline HCompress& disableSmoothing();

private:
  /**
   * @brief The scale factor.
   */
  Factor m_scale;

  /**
   * @brief The smoothing flag.
   */
  bool m_smooth;
};

/**
 * @ingroup image_compression
 * @brief The PLIO algorithm.
 * 
 * @warning Only integer values between 0 and 2^24 are supported.
 */
class Plio : public AlgoMixin<Plio> {

public:
  ELEFITS_VIRTUAL_DTOR(Plio)
  ELEFITS_COPYABLE(Plio)
  ELEFITS_MOVABLE(Plio)

  /**
   * @brief Constructor
   */
  inline Plio(Position<-1> shape = rowwiseTiling());
};

/**
 * @ingroup image_compression
 * @brief The GZIP algorithm.
 */
class Gzip : public AlgoMixin<Gzip> {

public:
  ELEFITS_VIRTUAL_DTOR(Gzip)
  ELEFITS_COPYABLE(Gzip)
  ELEFITS_MOVABLE(Gzip)

  /**
   * @brief Constructor
   */
  inline Gzip(Position<-1> shape = rowwiseTiling());
};

/**
 * @ingroup image_compression
 * @brief The GZIP algorithm applied to "shuffled" pixel values.
 * 
 * Suffling means that value bytes are reordered such that most significant bytes of each value appear first.
 * Generally, this algorithm is much more efficient in terms of compression factor than GZIP, although it is a bit slower.
 */
class ShuffledGzip : public AlgoMixin<ShuffledGzip> { // FIXME merge with Gzip with option to shuffle

public:
  ELEFITS_VIRTUAL_DTOR(ShuffledGzip)
  ELEFITS_COPYABLE(ShuffledGzip)
  ELEFITS_MOVABLE(ShuffledGzip)

  /**
   * @brief Constructor.
   */
  inline ShuffledGzip(Position<-1> shape = rowwiseTiling());
};

/**
 * @brief Create a lossless algorithm well suited to the HDU properties.
 * @param bitpix The uncompressed data BITPIX
 * @param dimension The uncompressed data NAXIS
 */
inline std::unique_ptr<Algo> makeLosslessAlgo(long bitpix, long dimension) {
  std::unique_ptr<Algo> out;
  const auto q0 = Quantization(Factor::none());
  if (bitpix > 0 && bitpix <= 24) {
    out.reset(&(new Plio())->quantization(q0));
  } else if (dimension >= 2) {
    out.reset(&(new HCompress())->quantization(q0));
  } else {
    out.reset(&(new Rice())->quantization(q0));
  }
  return out;
}

/**
 * @brief Create a possibly lossy algorithm well suited to the HDU properties.
 * @param bitpix The uncompressed data BITPIX
 * @param dimension The uncompressed data NAXIS
 */
inline std::unique_ptr<Algo> makeAlgo(long bitpix, long dimension) {
  std::unique_ptr<Algo> out;
  if (bitpix > 0 && bitpix <= 24) {
    out.reset(&(new Plio())->quantization(Quantization(Factor::none())));
  } else if (dimension >= 2) {
    out.reset(&(new HCompress())->scale(Factor::relative(2.5)));
  } else {
    out.reset(new Rice());
  }
  return out;
}

} // namespace Compression
} // namespace Fits
} // namespace Euclid

/// @cond INTERNAL
#define COMPRESSION_IMPL
#include "EleFitsData/impl/Compression.hpp"
#undef COMPRESSION_IMPL
/// @endcond

#endif
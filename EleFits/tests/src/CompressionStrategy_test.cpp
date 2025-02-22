// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "EleFits/CompressionStrategy.h"
#include "EleFits/FitsFileFixture.h"
#include "EleFits/MefFile.h"

#include <boost/test/unit_test.hpp>

using namespace Euclid::Fits;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(CompressionStrategy_test)

//-----------------------------------------------------------------------------

/**
 * @brief Whatever the type and shape, check losslessness.
 */
template <typename T>
auto check_basic_lossless(Position<-1> shape) {
  auto strategy = CompressAuto();
  ImageHdu::Initializer<T> init {1, "", {}, shape, nullptr};
  const auto& algo = strategy(init);
  BOOST_TEST(algo.is_lossless());
  bool none = dynamic_cast<const NoCompression*>(&algo);
  auto bytes = shapeSize(shape) * sizeof(T);
  BOOST_TEST(none == (bytes <= 2880));
  if (not none && std::is_floating_point_v<T>) {
    BOOST_CHECK_NO_THROW(dynamic_cast<const ShuffledGzip&>(algo));
  }
}

/**
 * @brief Whatever the type and shape, check losslessness for integers.
 */
template <typename T>
void check_basic_lossless_ints(Position<-1> shape) {
  auto strategy = CompressAuto(CompressionType::LosslessInts);
  ImageHdu::Initializer<T> init {1, "", {}, shape, nullptr};
  const auto& algo = strategy(init);
  if (std::is_integral_v<T>) {
    BOOST_TEST(algo.is_lossless());
  }
  bool none = dynamic_cast<const NoCompression*>(&algo);
  auto bytes = shapeSize(shape) * sizeof(T);
  BOOST_TEST(none == (bytes <= 2880));
}

template <typename T>
void check_basic_lossy(Position<-1> shape) {
  auto strategy = CompressAuto(CompressionType::LosslessInts);
  ImageHdu::Initializer<T> init {1, "", {}, shape, nullptr};
  const auto& algo = strategy(init);
  bool none = dynamic_cast<const NoCompression*>(&algo);
  auto bytes = shapeSize(shape) * sizeof(T);
  BOOST_TEST(none == (bytes <= 2880));
}

template <typename T>
void check_basic(Position<-1> shape) {
  check_basic_lossless<T>(shape);
  check_basic_lossless_ints<T>(shape);
  check_basic_lossy<T>(shape);
}

#define BASIC_LOSSLESSNESS_TEST(type, name) \
  BOOST_AUTO_TEST_CASE(name##_basic_losslessness_test) { \
    check_basic<type>({0}); \
    check_basic<type>({1}); \
    check_basic<type>({2879}); \
    check_basic<type>({2880}); \
    check_basic<type>({2880, 4}); \
  }

// ELEFITS_FOREACH_RASTER_TYPE(BASIC_LOSSLESSNESS_TEST) // FIXME re-enable

template <typename T, typename TAction>
void check_can_compress(TAction action) {
  ImageHdu::Initializer<T> zero {1, "", {}, {2880 / sizeof(T)}, nullptr};
  BOOST_TEST(not action.compression(zero));
  ImageHdu::Initializer<T> one {1, "", {}, {2880 / sizeof(T), 4}, nullptr};
  BOOST_TEST(bool(action.compression(one)));
}

template <typename T, typename TAction>
void check_cannot_compress(TAction action) {
  ImageHdu::Initializer<T> many {1, "", {}, {2880 / sizeof(T), 2880 / sizeof(T)}, nullptr};
  BOOST_TEST(not action.compression(many));
}

BOOST_AUTO_TEST_CASE(lossless_compression_ability_test) {

  check_can_compress<std::uint16_t>(Compress<Gzip>());
  check_cannot_compress<std::int64_t>(Compress<Gzip>());
  check_can_compress<float>(Compress<Gzip>());

  check_can_compress<std::uint16_t>(Compress<ShuffledGzip>());
  check_cannot_compress<std::int64_t>(Compress<ShuffledGzip>());
  check_can_compress<float>(Compress<ShuffledGzip>());

  check_can_compress<std::uint16_t>(Compress<Rice>());
  check_cannot_compress<std::int64_t>(Compress<Rice>());
  check_cannot_compress<float>(Compress<Rice>());

  check_can_compress<std::uint16_t>(Compress<HCompress>());
  check_cannot_compress<std::int64_t>(Compress<HCompress>());
  check_cannot_compress<float>(Compress<HCompress>());

  check_can_compress<std::uint16_t>(Compress<Plio>());
  check_cannot_compress<std::int32_t>(Compress<Plio>());
  check_cannot_compress<std::int64_t>(Compress<Plio>());
  check_cannot_compress<float>(Compress<Plio>());

  check_can_compress<std::uint16_t>(CompressInts<Gzip>());
  check_cannot_compress<std::int64_t>(CompressInts<Gzip>());
  check_cannot_compress<float>(CompressInts<Gzip>());

  check_cannot_compress<std::uint16_t>(CompressFloats<Gzip>());
  check_cannot_compress<std::int64_t>(CompressFloats<Gzip>());
  check_can_compress<float>(CompressFloats<Gzip>());
}

BOOST_AUTO_TEST_CASE(lossy_compression_ability_test) {
  check_can_compress<float>(Compress<Rice>(Tile::rowwise(), Quantization(4)));
  check_can_compress<float>(Compress<HCompress>(Tile::rowwise(16), Quantization(4)));
  check_cannot_compress<float>(Compress<Plio>(Tile::rowwise(), Quantization(4)));
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

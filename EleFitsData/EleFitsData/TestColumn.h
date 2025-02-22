// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _ELEFITSDATA_TESTCOLUMN_H
#define _ELEFITSDATA_TESTCOLUMN_H

#include "EleFitsData/Column.h"
#include "EleFitsData/TestUtils.h"

#include <complex>
#include <string>
#include <tuple>

namespace Euclid {
namespace Fits {
namespace Test {

/**
 * @brief A set of random columns which cover the whole set of supported types.
 */
struct RandomTable {

  /**
   * @brief Generate the table.
   * @param repeat_count The repeat count of each column.
   * @param row_count The row count of each column.
   */
  RandomTable(long repeat_count = 1, long row_count = 3);

  /**
   * @brief Generate a column.
   * @param type_name The value type name.
   * @param repeat_count The repeat count.
   * @param row_count The row count.
   */
  template <typename T>
  static VecColumn<T> generate_column(const std::string& type_name, long repeat_count = 1, long row_count = 3);

  /**
   * @brief Get the column with given value type.
   */
  template <typename T>
  const VecColumn<T>& get_column() const;

  /**
   * @brief Get the column with given value type.
   */
  template <typename T>
  VecColumn<T>& get_column();

  /**
   * @brief The columns.
   */
  std::tuple<
      VecColumn<char>,
      VecColumn<std::int16_t>,
      VecColumn<std::int32_t>,
      VecColumn<std::int64_t>,
      VecColumn<float>,
      VecColumn<double>,
      VecColumn<std::complex<float>>,
      VecColumn<std::complex<double>>,
      VecColumn<std::string>,
      VecColumn<unsigned char>,
      VecColumn<std::uint16_t>,
      VecColumn<std::uint32_t>,
      VecColumn<std::uint64_t>>
      columns;

  /** @brief The number of columns. */
  static constexpr long column_count = 13;
}; // namespace Test

/**
 * @brief A small set of columns with various types.
 */
class SmallTable {

public:
  /**
   * @brief Type of the NUM column.
   */
  using Num = int;

  /**
   * @brief Type of the RADEC column.
   */
  using Radec = std::complex<float>;

  /**
   * @brief Type of the NAME column.
   */
  using Name = std::string;

  /**
   * @brief Type of the DIST_MAG column.
   */
  using DistMag = double;

  /**
   * @brief Generate the columns.
   */
  SmallTable();

  /**
   * @brief HDU name.
   */
  std::string extname;

  /**
   * @brief Values of the NUM column.
   */
  std::vector<Num> nums;

  /**
   * @brief Values of the RADEC column.
   */
  std::vector<Radec> radecs;

  /**
   * @brief Values of the NAME column.
   */
  std::vector<Name> names;

  /**
   * @brief Values of the DIST_MAG column.
   */
  std::vector<DistMag> dists_mags;

  /**
   * @brief NUM column.
   */
  PtrColumn<Num> num_col;

  /**
   * @brief RADEC column.
   */
  PtrColumn<Radec> radec_col;

  /**
   * @brief NAME column.
   */
  PtrColumn<Name> name_col;

  /**
   * @brief DIST_MAG column.
   */
  PtrColumn<DistMag> dist_mag_col;
};

/**
 * @brief A random scalar Column of given type.
 */
template <typename T>
class RandomScalarColumn : public VecColumn<T> {

public:
  /**
   * @brief Generate a Column of given size.
   */
  explicit RandomScalarColumn(long size = 3, T min = almost_min<T>(), T max = almost_max<T>());

  /** @brief Destructor. */
  virtual ~RandomScalarColumn() = default;
};

/**
 * @brief A small vector column of given type.
 */
template <typename T>
class RandomVectorColumn : public VecColumn<T> {

public:
  /**
   * @brief Generate a Column.
   */
  explicit RandomVectorColumn(long repeat_count = 3, long size = 3, T min = almost_min<T>(), T max = almost_max<T>());

  /** @brief Destructor. */
  virtual ~RandomVectorColumn() = default;
};

} // namespace Test
} // namespace Fits
} // namespace Euclid

/// @cond INTERNAL
#define _ELEFITSDATA_TESTCOLUMN_IMPL
#include "EleFitsData/impl/TestColumn.hpp"
#undef _ELEFITSDATA_TESTCOLUMN_IMPL
/// @endcond

#endif

// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _ELEFITS_TESTBINTABLE_H
#define _ELEFITS_TESTBINTABLE_H

#include "EleFits/FitsFileFixture.h" // FIXME rename as TestFitsFile.h
#include "EleFitsData/TestColumn.h"

namespace Euclid {
namespace Fits {
namespace Test {

/**
 * @brief Temporary MEF file with one binary table extension.
 * @details
 * The table is made of a scalar and a vector column of same value type.
 */
template <typename T>
struct TestBintable { // FIXME add multidimensional column

  /**
   * @brief Constructor.
   */
  TestBintable(long rows = 10) :
      scalar_column(rows), vector_column(3, rows), first_column(scalar_column), last_column(vector_column), file(),
      hdu(file.append_bintable_header("BINTABLE", {}, first_column.info(), last_column.info())),
      columns(hdu.columns()) {
    assert(scalar_column.info().name != vector_column.info().name);
  }

  /**
   * @brief The scalar column.
   */
  Test::RandomScalarColumn<T> scalar_column;

  /**
   * @brief The vector column.
   */
  Test::RandomVectorColumn<T> vector_column;

  /**
   * @brief A reference to the first column.
   */
  VecColumn<T>& first_column;

  /**
   * @brief A reference to the last column.
   */
  VecColumn<T>& last_column;

  /**
   * @brief The MEF file.
   */
  Test::TemporaryMefFile file;

  /**
   * @brief The HDU.
   */
  const BintableHdu& hdu;

  /**
   * @brief The data unit.
   */
  const BintableColumns& columns;
};

} // namespace Test
} // namespace Fits
} // namespace Euclid

#endif

// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _ELEFITS_BINTABLEHDU_H
#define _ELEFITS_BINTABLEHDU_H

#include "EleCfitsioWrapper/BintableWrapper.h"
#include "EleFits/BintableColumns.h"
#include "EleFits/Hdu.h"

#include <string>

namespace Euclid {
namespace Fits {

/**
 * @ingroup bintable_handlers
 * @brief Binary table HDU reader-writer.
 */
class BintableHdu : public Hdu {

public:
  /// @cond INTERNAL

  /**
   * @see Hdu
   */
  BintableHdu(Token, fitsfile*& fptr, long index, HduCategory status = HduCategory::Untouched);

  /**
   * @see Hdu
   */
  BintableHdu();

  /// @endcond

  /**
   * @brief Destructor.
   */
  virtual ~BintableHdu() = default;

  /**
   * @brief Access the data unit column-wise.
   * @see BintableColumns
   */
  const BintableColumns& columns() const;

  /**
   * @brief Read the number of columns.
   */
  long read_column_count() const;

  /**
   * @brief Read the number of rows.
   */
  long read_row_count() const;

  /**
   * @copydoc Hdu::category
   */
  HduCategory category() const override;

  /**
   * @brief Read a column with given name or index.
   */
  template <typename T, long N = 1>
  VecColumn<T, N> read_column(ColumnKey key) const;

  /**
   * @brief Write a column.
   */
  template <typename TColumn>
  void write_column(const TColumn& column) const;

  /**
   * @deprecated
   */
  long readColumnCount() const {
    return read_column_count();
  }

  /**
   * @deprecated
   */
  long readRowCount() const {
    return read_row_count();
  }

  /**
   * @deprecated
   */
  template <typename T, long N = 1>
  VecColumn<T, N> readColumn(ColumnKey key) const {
    return read_column<T, N>(key);
  }

  /**
   * @deprecated
   */
  template <typename TColumn>
  void writeColumn(const TColumn& column) const {
    return write_column(column);
  }

private:
  /**
   * @brief The column-wise data unit handler.
   */
  BintableColumns m_columns;
};

} // namespace Fits
} // namespace Euclid

/// @cond INTERNAL
#define _ELEFITS_BINTABLEHDU_IMPL
#include "EleFits/impl/BintableHdu.hpp"
#undef _ELEFITS_BINTABLEHDU_IMPL
/// @endcond

#endif

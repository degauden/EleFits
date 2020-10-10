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

#ifndef _EL_FITSDATA_COLUMN_H
#define _EL_FITSDATA_COLUMN_H

#include <string>
#include <vector>

namespace Euclid {
namespace FitsIO {

/**
 * @brief Column metadata, i.e. `{ name, unit, repeatCount }`
 * and the value type as the template parameter.
 * @details
 * Bintable columns are either scalar (`repeatCount` = 1) or vector (`repeatCount` > 1).
 * In the case of vector columns, each cell of the column contains `repeatCount` values.
 * Here is an example of a 4-row table with a scalar column and a vector column with a repeat count of 3:
 * \code
 * === ======== ========
 * ROW REPEAT_1 REPEAT_3
 * === ======== ========
 *  1     11    11 12 13
 * --- -------- --------
 *  2     21    21 22 23
 * --- -------- --------
 *  3     31    31 32 33
 * --- -------- --------
 *  4     41    41 42 43
 * === ======== ========
 * \endcode
 * For performance, the values are stored sequentially in a 1D array as follows:
 * \code
 * int scalar[] = { 11, 21, 31, 41 };
 * int vector[] = { 11, 12, 13, 21, 22, 33, 41, 42, 43, 44 };
 * \endcode
 *
 * The only exception to this is string columns, which are vector columns
 * -- they should have a repeat count greater than the maximum number of characters in a cell --
 * but each cell contains only one string:
 * \code
 * === ========
 * ROW REPEAT_7
 * === ========
 *  1  "CELL_1"
 * --- --------
 *  2  "CELL_2"
 * --- --------
 *  3  "CELL_3"
 * --- --------
 *  4  "CELL_4"
 * === ========
 * \endcode
 * The data array is a simple array of `std::string`s:
 * \code
 * std::string data[] = { "CELL_1", "CELL_2", "CELL_3", "CELL_4" };
 * \endcode
 * but the repeat count should be at least 7 (beware of the null terminating character).
 *
 * @note
 * Since the values are stored sequentially even for vector columns,
 * a scalar column can be "fold" into a vector column by
 * just setting a repeat count greater than 1, and vice-versa.
 * This trick allows writing scalar columns as vector columns,
 * which is what CFitsIO recommends for performance.
 * Indeed, with CFitsIO, it is much faster to write 1 row with a repeat count of 10.000
 * than 10.000 rows with a repeat count of 1.
 * This is because bintables are written row-wise in the Fits file.
 * CFitsIO uses an internal buffer, which can be exploited to optimize reading and writing.
 * This is generally handled through the "iterator function" provided by CFitsIO.
 * @note
 * Fortunately, this complexity is already embedded in EL_FitsIO internals:
 * the buffer is used optimally when reading and writing several columns.
 * In general, it is nearly as fast to read and write scalar columns as vector columns with EL_FitsIO.
 * Therefore, users are encouraged to consider the repeat count as a meaningful value,
 * rather than as an optimization trick.
 *
 * @see \ref data-classes
 */
template <typename T>
struct ColumnInfo {

  /**
   * @brief Column name.
   */
  std::string name;

  /**
   * @brief Column unit.
   */
  std::string unit;

  /**
   * @brief Repeat count of the column, i.e., number of values per cell.
   * @details
   * Scalar columns have a repeat count of 1.
   * @warning
   * String columns are considered vector columns.
   * Their repeat count must be greater or equal to the longest string of the column
   * including the `\0` character.
   */
  long repeatCount;
};

/**
 * @brief Bintable column data and metadata.
 * @details
 * This is an interface to be implemented with a concrete data container (e.g. `std::vector`).
 * Some implementations are provided with the library,
 * but others could be useful to interface with client code
 * (e.g. with other external libraries with custom containers).
 * @see \ref data-classes
 */
template <typename T>
class Column {

public:
  /**
   * @brief Destructor.
   */
  virtual ~Column() = default;

  /**
   * @brief Create a column with given metadata.
   */
  explicit Column(ColumnInfo<T> columnInfo);

  /**
   * @brief Number of elements in the column, i.e. repeat count * number of rows.
   * @warning
   * For string columns, CFitsIO requires elementCount to be just the number of rows.
   */
  virtual long elementCount() const = 0;

  /**
   * @brief Number of rows in the column.
   */
  long rowCount() const;

  /**
   * @brief Const pointer to the first data element.
   */
  virtual const T *data() const = 0;

  /**
   * @brief Column metadata.
   */
  ColumnInfo<T> info;
};

/**
 * @brief Column which references some external pointer data.
 * @details
 * Use it for temporary columns.
 * @see \ref data-classes
 */
template <typename T>
class PtrColumn : public Column<T> {

public:
  /** @brief Destructor. */
  virtual ~PtrColumn() = default;
  /** @brief Copy constructor. */
  PtrColumn(const PtrColumn &) = default;
  /** @brief Move constructor. */
  PtrColumn(PtrColumn &&) = default;
  /** @brief Copy assignment. */
  PtrColumn &operator=(const PtrColumn &) = default;
  /** @brief Move assignment. */
  PtrColumn &operator=(PtrColumn &&) = default;

  /**
   * @brief Create a new column with given metadata and data.
   * @param info The column metadata.
   * @param elementCount The number of elements in the column,
   * which is the number of rows for scalar and string columns.
   * @param data Pointer to the first element of the data.
   */
  PtrColumn(ColumnInfo<T> info, long elementCount, const T *data);

  /** @copydoc Column::elementCount */
  long elementCount() const override;

  /** @copydoc Column::data */
  const T *data() const override;

private:
  long m_nelements;
  const T *m_data;
};

/**
 * @brief Column which references some external vector data.
 * @details
 * Use it for temporary columns.
 * @see \ref data-classes
 */
template <typename T>
class VecRefColumn : public Column<T> {

public:
  /** @brief Destructor. */
  virtual ~VecRefColumn() = default;
  /** @brief Copy constructor. */
  VecRefColumn(const VecRefColumn &) = default;
  /** @brief Move constructor. */
  VecRefColumn(VecRefColumn &&) = default;
  /** @brief Copy assignment. */
  VecRefColumn &operator=(const VecRefColumn &) = default;
  /** @brief Move assignment. */
  VecRefColumn &operator=(VecRefColumn &&) = default;

  /**
   * @brief Create a VecRefColumn with given metadata and reference to data.
   */
  VecRefColumn(ColumnInfo<T> info, const std::vector<T> &vecRef);

  /** @copydoc Column::elementCount */
  long elementCount() const override;

  /** @copydoc Column::data */
  const T *data() const override;

  /**
   * @brief Const reference to the vector data.
   */
  const std::vector<T> &vector() const;

private:
  const std::vector<T> &m_ref;
};

/**
 * @brief Column which stores internally the data.
 * @details
 * Use it (via move semantics) if you don't need your data after the write operation.
 * @see \ref data-classes
 */
template <typename T>
class VecColumn : public Column<T> {

public:
  /** @brief Destructor. */
  virtual ~VecColumn() = default;
  /** @brief Copy constructor. */
  VecColumn(const VecColumn &) = default;
  /** @brief Move constructor. */
  VecColumn(VecColumn &&) = default;
  /** @brief Copy assignment. */
  VecColumn &operator=(const VecColumn &) = default;
  /** @brief Move assignment. */
  VecColumn &operator=(VecColumn &&) = default;

  /**
   * @brief Create an empty VecColumn.
   */
  VecColumn();

  /**
   * @brief Crate a VecColumn with given data and metadata.
   * @details
   * To transfer ownership of the data instead of copying it, use move semantics:
   * \code
   * VecColumn column(info, std::move(vec));
   * \endcode
   */
  VecColumn(ColumnInfo<T> info, std::vector<T> vec);

  /**
   * @brief Create a VecColumn with given metadata.
   */
  VecColumn(ColumnInfo<T> info, long rowCount);

  /** @copydoc Column::elementCount */
  long elementCount() const override;

  /** @copydoc Column::data */
  const T *data() const override;

  /**
   * @brief Non-const pointer to the first data element.
   */
  T *data();

  /**
   * @brief Const reference to the vector data.
   */
  const std::vector<T> &vector() const;

  /**
   * @brief Non-const reference to the data, useful to take ownership through move semantics.
   * \code
   * std::vector<T> v = std::move(column.vector());
   * \endcode
   */
  std::vector<T> &vector();

private:
  std::vector<T> m_vec;
};

} // namespace FitsIO
} // namespace Euclid

/// @cond INTERNAL
#define _EL_FITSDATA_COLUMN_IMPL
#include "EL_FitsData/impl/Column.hpp"
#undef _EL_FITSDATA_COLUMN_IMPL
/// @endcond

#endif

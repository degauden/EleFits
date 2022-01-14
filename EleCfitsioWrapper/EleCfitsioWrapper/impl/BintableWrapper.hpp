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

#if defined(_ELECFITSIOWRAPPER_BINTABLEWRAPPER_IMPL) || defined(CHECK_QUALITY)

#include "EleCfitsioWrapper/BintableWrapper.h"
#include "EleCfitsioWrapper/ErrorWrapper.h"
#include "EleFitsData/FitsError.h"
#include "ElementsKernel/Unused.h"

#include <algorithm> // transform

namespace Euclid {
namespace Cfitsio {
namespace BintableIo {

/// @cond INTERNAL
namespace Internal {

/**
 * @brief Read metadata and allocate data.
 */
template <typename T>
void readColumnInfoImpl(fitsfile* fptr, long index, Fits::VecColumn<T>& column, long rowCount);

/**
 * @brief String specialization.
 */
template <>
void readColumnInfoImpl<std::string>(fitsfile* fptr, long index, Fits::VecColumn<std::string>& column, long rowCount);

template <typename T>
void readColumnInfoImpl(fitsfile* fptr, long index, Fits::VecColumn<T>& column, long rowCount) {
  column = Fits::VecColumn<T>(
      readColumnInfo<T>(fptr, index),
      std::vector<std::decay_t<T>>(column.info().repeatCount() * rowCount));
}

/**
 * @brief Helper class to loop on a collection of columns.
 * @tparam i The index of the column the methods should be applied to:
 * should be initialized with `sizeof...(Ts) - 1`.
 * @details
 * This is a recursive implementation, decreasing from `sizeof...(Ts) - 1` to 0.
 * Calling a method with index `i` means applying the change to the `i`-th column,
 * and then calling the method with index `i - 1`.
 */
template <std::size_t i, typename... TColumns>
struct ColumnLooperImpl {

  /**
   * @brief Read metadata and allocate data for each column
   */
  static void
  readInfos(fitsfile* fptr, const std::vector<long>& indices, std::tuple<TColumns...>& columns, long rowCount) {
    readColumnInfoImpl(fptr, indices[i], std::get<i>(columns), rowCount);
    ColumnLooperImpl<i - 1, TColumns...>::readInfos(fptr, indices, columns, rowCount);
  }

  /**
   * @brief Read a chunk of each column
   */
  static void readChunks(
      fitsfile* fptr,
      const std::vector<long>& indices,
      std::tuple<TColumns...>& columns,
      long firstRow,
      long rowCount) {
    auto data = &std::get<i>(columns)(firstRow - 1);
    const auto repeatCount = std::get<i>(columns).info().repeatCount();
    readColumnData(fptr, Fits::Segment::fromSize(firstRow, rowCount), indices[i], repeatCount, data);
    ColumnLooperImpl<i - 1, TColumns...>::readChunks(fptr, indices, columns, firstRow, rowCount);
  }

  /**
   * @brief Get the max number of rows of the columns.
   */
  static void maxRowCount(const std::tuple<const TColumns&...>& columns, long& count = 0) {
    count = std::max(std::get<i>(columns).rowCount(), count);
    ColumnLooperImpl<i - 1, TColumns...>::maxRowCount(columns, count);
  }

  /**
   * @brief Write a chunk of each column
   */
  static void writeChunks(
      fitsfile* fptr,
      const std::vector<long>& indices,
      std::tuple<const TColumns&...> columns,
      long firstRow,
      long rowCount) {
    const auto data = &std::get<i>(columns)(firstRow - 1);
    const auto repeatCount = std::get<i>(columns).info().repeatCount();
    writeColumnData(fptr, Fits::Segment::fromSize(firstRow, rowCount), indices[i], repeatCount, data);
    ColumnLooperImpl<i - 1, TColumns...>::writeChunks(fptr, indices, columns, firstRow, rowCount);
  }
};

/**
 * @brief Past-the-terminal case (do nothing).
 */
template <typename... TColumns>
struct ColumnLooperImpl<std::size_t(-1), TColumns...> {

  /** @brief Pass */
  static void readInfos(fitsfile*, const std::vector<long>&, std::tuple<TColumns...>&, long) {}

  /** @brief Pass */
  static void readChunks(fitsfile*, const std::vector<long>&, std::tuple<TColumns...>&, long, long) {}

  /** @brief Pass */
  static void maxRowCount(const std::tuple<const TColumns&...>&, long&) {}

  /** @brief Pass */
  static void writeChunks(fitsfile*, const std::vector<long>&, std::tuple<const TColumns&...>, long, long) {}
};

} // namespace Internal
/// @endcond

template <typename T, long N> // USED
Fits::ColumnInfo<T, N> readColumnInfo(fitsfile* fptr, long index) {
  int status = 0;
  char name[FLEN_VALUE];
  char unit[FLEN_VALUE];
  long repeatCount = 0;
  fits_get_bcolparms(
      fptr,
      index,
      name,
      unit,
      nullptr, // typechar
      &repeatCount,
      nullptr, // scale
      nullptr, // zero
      nullptr, // nulval
      nullptr, // tdisp
      &status);
  CfitsioError::mayThrow(status, fptr, "Cannot read column info: #" + std::to_string(index - 1));
  return {name, unit, repeatCount};
}

template <typename T, long N>
Fits::VecColumn<T, N> readColumn(fitsfile* fptr, long index) {
  const long rows = rowCount(fptr);
  Fits::VecColumn<T, N> column(readColumnInfo<T>(fptr, index), rows);
  readColumnData(fptr, {1, rows}, index, column.info().repeatCount(), column.data());
  return column;
}

template <typename TColumn>
void readColumnSegment(fitsfile* fptr, const Fits::Segment& rows, long index, TColumn& column) {
  readColumnData(fptr, rows, index, column.info().repeatCount(), column.data());
}

template <typename T, long N>
Fits::VecColumn<T, N> readColumn(fitsfile* fptr, const std::string& name) {
  return readColumn<T, N>(fptr, columnIndex(fptr, name));
}

template <typename TColumn>
void writeColumn(fitsfile* fptr, const TColumn& column) {
  writeColumnSegment(fptr, 1, column);
}

template <typename TColumn>
void writeColumnSegment(fitsfile* fptr, long firstRow, const TColumn& column) {
  const auto index = columnIndex(fptr, column.info().name);
  writeColumnData(
      fptr,
      Fits::Segment::fromSize(firstRow, column.rowCount()),
      index,
      column.info().repeatCount(),
      column.data());
}

template <typename... Ts> // FIXME Ns
std::tuple<Fits::VecColumn<Ts, 1>...> readColumns(fitsfile* fptr, const std::vector<long>& indices) {
  /* Read column metadata */
  const long rows = rowCount(fptr);
  std::tuple<Fits::VecColumn<Ts, 1>...> columns;
  Internal::ColumnLooperImpl<sizeof...(Ts) - 1, Fits::VecColumn<Ts, 1>...>::readInfos(fptr, indices, columns, rows);
  /* Get the buffer size */
  int status = 0;
  long chunkRows = 0;
  fits_get_rowsize(fptr, &chunkRows, &status);
  if (chunkRows == 0) {
    throw Fits::FitsError("Cannot compute the optimal number of rows to be read at once");
  }
  /* Read column data */
  for (long first = 1; first <= rows; first += chunkRows) {
    long last = first + chunkRows - 1;
    if (last > rows) {
      chunkRows = rows - first + 1;
    }
    Internal::ColumnLooperImpl<sizeof...(Ts) - 1, Fits::VecColumn<Ts, 1>...>::readChunks(
        fptr,
        indices,
        columns,
        first,
        chunkRows);
  }
  return columns;
}

template <typename... Ts> // FIXME Ns
std::tuple<Fits::VecColumn<Ts, 1>...> readColumns(fitsfile* fptr, const std::vector<std::string>& names) {
  /* List column indices */
  std::vector<long> indices(names.size());
  std::transform(names.cbegin(), names.cend(), indices.begin(), [&](const std::string& n) {
    return columnIndex(fptr, n);
  });
  return readColumns<Ts...>(fptr, indices);
}

template <typename... TColumns>
void writeColumns(fitsfile* fptr, const TColumns&... columns) {
  int status = 0;
  /* Get chunk size */
  const auto table = std::forward_as_tuple(columns...);
  long rows = 0;
  Internal::ColumnLooperImpl<sizeof...(TColumns) - 1, TColumns...>::maxRowCount(table, rows);
  long chunkRows = 0;
  fits_get_rowsize(fptr, &chunkRows, &status); // Tested with other values, e.g. 1 and 10; less efficient
  if (chunkRows == 0) {
    throw Fits::FitsError("Cannot compute the optimal number of rows to be read at once");
  }
  /* Write column data */
  std::vector<long> indices {columnIndex(fptr, columns.info().name)...};
  for (long first = 1; first <= rows; first += chunkRows) {
    long last = first + chunkRows - 1;
    if (last > rows) {
      chunkRows = rows - first + 1;
    }
    Internal::ColumnLooperImpl<sizeof...(TColumns) - 1, TColumns...>::writeChunks(
        fptr,
        indices,
        table,
        first,
        chunkRows);
  }
}

template <typename TColumn>
void insertColumn(fitsfile* fptr, long index, const TColumn& column) {
  auto name = toCharPtr(column.info().name);
  auto tform = toCharPtr(TypeCode<typename TColumn::Value>::tform(column.info().repeatCount()));
  // FIXME write unit
  int status = 0;
  fits_insert_col(fptr, static_cast<int>(index), name.get(), tform.get(), &status);
  writeColumn(fptr, column);
}

template <typename... TColumns>
void insertColumns(fitsfile* fptr, long index, const TColumns&... columns) {
  auto names = CStrArray({columns.info().name...});
  auto tforms = CStrArray({TypeCode<typename TColumns::Value>::tform(columns.info().repeatCount())...});
  // FIXME write unit
  int status = 0;
  fits_insert_cols(fptr, static_cast<int>(index), sizeof...(TColumns), names.data(), tforms.data(), &status);
  writeColumns(fptr, columns...);
}

template <typename TColumn>
void appendColumn(fitsfile* fptr, const TColumn& column) {
  int ncols = 0;
  int status = 0;
  fits_get_num_cols(fptr, &ncols, &status);
  insertColumn(fptr, ncols + 1, column);
}

template <typename... TColumns>
void appendColumns(fitsfile* fptr, const TColumns&... columns) {
  int ncols = 0;
  int status = 0;
  fits_get_num_cols(fptr, &ncols, &status);
  insertColumns(fptr, ncols + 1, columns...);
}

template <typename T>
void readColumnData(fitsfile* fptr, const Fits::Segment& rows, long index, long repeatCount, T* data) {
  int status = 0;
  const auto size = rows.size() * repeatCount;
  fits_read_col(
      fptr,
      TypeCode<T>::forBintable(), // datatype
      static_cast<int>(index),
      rows.front, // 1-based first row index
      1, // 1-based first element index
      size, // number of elements
      nullptr,
      data,
      nullptr,
      &status);
  CfitsioError::mayThrow(status, fptr, "Cannot read column data: #" + std::to_string(index - 1));
}

template <typename T>
void writeColumnData(fitsfile* fptr, const Fits::Segment& rows, long index, long repeatCount, const T* data) {
  int status = 0;
  const auto size = rows.size() * repeatCount;
  std::vector<T> nonconstData(data, data + size); // We need a non-const data for CFitsIO
  fits_write_col(
      fptr,
      TypeCode<T>::forBintable(), // datatype
      static_cast<int>(index), // colnum
      rows.front, // firstrow (1-based)
      1, // firstelem (1-based)
      size, // nelements
      nonconstData.data(),
      &status);
  CfitsioError::mayThrow(status, fptr, "Cannot write column data: #" + std::to_string(index - 1));
}

} // namespace BintableIo
} // namespace Cfitsio
} // namespace Euclid

#endif

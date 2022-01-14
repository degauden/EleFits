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

#if defined(_ELEFITSDATA_COLUMNINFO_IMPL) || defined(CHECK_QUALITY)

#include "EleFitsData/ColumnInfo.h"

namespace Euclid {
namespace Fits {

template <typename T, long N>
bool operator==(const ColumnInfo<T, N>& lhs, const ColumnInfo<T, N>& rhs) {
  return lhs.name == rhs.name && lhs.unit == rhs.unit && lhs.repeatCount() == rhs.repeatCount();
}

template <typename T, long N>
bool operator!=(const ColumnInfo<T, N>& lhs, const ColumnInfo<T, N>& rhs) {
  return not(lhs == rhs);
}

/**
 * @brief String specialization.
 */
template <>
long ColumnInfo<std::string, 1>::elementCountPerEntry() const;

// Generic

template <typename T, long N>
ColumnInfo<T, N>::ColumnInfo(std::string n, std::string u, long r) : ColumnInfo(n, u, Position<N>::one()) {
  shape[0] = r;
}

template <typename T, long N>
ColumnInfo<T, N>::ColumnInfo(std::string n, std::string u, Position<N> s) : name(n), unit(u), shape(s) {}

template <typename T, long N>
long ColumnInfo<T, N>::repeatCount() const {
  return shapeSize(shape);
}

template <typename T, long N>
long ColumnInfo<T, N>::elementCountPerEntry() const {
  return repeatCount();
}

} // namespace Fits
} // namespace Euclid

#endif

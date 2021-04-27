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

#include <boost/test/unit_test.hpp>

#include "EL_FitsData/TestRecord.h"

using namespace Euclid::FitsIO;

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(TestRecord_test, Test::RandomHeader)

//-----------------------------------------------------------------------------

template <typename T>
void checkFieldsAreAllSet(const Test::RandomHeader& header) {
  const auto& record = header.getRecord<T>();
  BOOST_CHECK(not record.keyword.empty());
  BOOST_CHECK(not record.unit.empty());
  BOOST_CHECK(not record.comment.empty());
}

template <typename T>
void checkFieldsAreAllDifferent(const Test::RandomHeader& header) {
  const auto& record = header.getRecord<T>();
  BOOST_CHECK_NE(record.keyword, record.unit);
  BOOST_CHECK_NE(record.unit, record.comment);
  BOOST_CHECK_NE(record.comment, record.keyword);
}

#define FIELDS_ARE_ALL_SET_TEST(type, name) \
  BOOST_AUTO_TEST_CASE(name##_fields_are_all_set_test) { \
    checkFieldsAreAllSet<type>(*this); \
  }

EL_FITSIO_FOREACH_RECORD_TYPE(FIELDS_ARE_ALL_SET_TEST)

#define FIELDS_ARE_ALL_DIFFERENT_TEST(type, name) \
  BOOST_AUTO_TEST_CASE(name##_fields_are_all_different_test) { \
    checkFieldsAreAllDifferent<type>(*this); \
  }

EL_FITSIO_FOREACH_RECORD_TYPE(FIELDS_ARE_ALL_DIFFERENT_TEST)

BOOST_AUTO_TEST_CASE(keywords_are_all_different_test) {
  const auto v = allRecords().vector;
  BOOST_CHECK_EQUAL(v.size(), recordCount);
  for (long lhs = 0; lhs < recordCount; ++lhs) {
    const auto& vlhs = v[lhs];
    for (long rhs = 0; rhs < lhs; ++rhs) {
      const auto& vrhs = v[rhs];
      BOOST_CHECK_NE(vlhs.keyword, vrhs.keyword);
    }
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

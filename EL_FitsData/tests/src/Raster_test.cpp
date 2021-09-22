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

#include "EL_FitsData/Raster.h"
#include "EL_FitsData/TestRaster.h"

#include <boost/test/unit_test.hpp>

using namespace Euclid::FitsIO;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Raster_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(index_test) {

  /* Fixed dimension */
  Position<4> fixedShape;
  for (auto& length : fixedShape) {
    length = std::rand();
  }
  Position<4> fixedPos;
  for (auto& coord : fixedPos) {
    coord = std::rand();
  }
  auto fixedIndex = Internal::IndexRecursionImpl<4>::index(fixedShape, fixedPos);
  BOOST_TEST(
      fixedIndex ==
      fixedPos[0] + fixedShape[0] * (fixedPos[1] + fixedShape[1] * (fixedPos[2] + fixedShape[2] * (fixedPos[3]))));

  /* Variable dimension */
  Position<-1> variableShape(fixedShape.begin(), fixedShape.end());
  Position<-1> variablePos(fixedPos.begin(), fixedPos.end());
  auto variableIndex = Internal::IndexRecursionImpl<-1>::index(variableShape, variablePos);
  BOOST_TEST(variableIndex == fixedIndex);
}

BOOST_AUTO_TEST_CASE(raster_data_test) {
  int data[] = { 0, 1, 2 };
  Raster<int, 1> raster({ 3 }, data);
  BOOST_TEST(raster.data() != nullptr);
  BOOST_TEST(raster[{ 0 }] == 0);
}

BOOST_AUTO_TEST_CASE(const_raster_data_test) {
  const int cData[] = { 3, 4, 5 };
  Raster<const int, 1> cRaster({ 3 }, cData);
  BOOST_TEST(cRaster.data() != nullptr);
  BOOST_TEST(cRaster[{ 0 }] == 3);
}

BOOST_AUTO_TEST_CASE(vec_raster_data_test) {
  VecRaster<int, 1> vecRaster({ 3 });
  BOOST_TEST(vecRaster.data() != nullptr);
  BOOST_TEST(vecRaster[{ 0 }] == 0);
}

BOOST_AUTO_TEST_CASE(const_vec_raster_data_test) {
  const VecRaster<int, 1> cVecRaster({ 3 });
  BOOST_TEST(cVecRaster.data() != nullptr);
  BOOST_TEST(cVecRaster[{ 0 }] == 0);
}

BOOST_FIXTURE_TEST_CASE(small_raster_size_test, Test::SmallRaster) {
  long size(this->width * this->height);
  BOOST_TEST(this->dimension() == 2);
  BOOST_TEST(this->size() == size);
  BOOST_TEST(this->vector().size() == size);
}

BOOST_AUTO_TEST_CASE(variable_dimension_raster_size_test) {
  const long width = 4;
  const long height = 3;
  const long size = width * height;
  Test::RandomRaster<int, -1> raster({ width, height });
  BOOST_TEST(raster.dimension() == 2);
  BOOST_TEST(raster.size() == size);
  BOOST_TEST(raster.vector().size() == size);
}

BOOST_AUTO_TEST_CASE(subscript_bounds_test) {
  const long width = 4;
  const long height = 3;
  Test::RandomRaster<int> raster({ width, height });
  raster.at({ 1, -1 }) = 1;
  BOOST_TEST(raster.at({ 1, -1 }) == 1);
  const auto& vec = raster.vector();
  BOOST_TEST((raster[{ 0, 0 }]) == vec[0]);
  BOOST_TEST(raster.at({ 0, 0 }) == vec[0]);
  BOOST_TEST(raster.at({ -1, 0 }) == vec[width - 1]);
  BOOST_TEST(raster.at({ -width, 0 }) == vec[0]);
  BOOST_TEST(raster.at({ 0, -1 }) == vec[(height - 1) * width]);
  BOOST_TEST(raster.at({ -1, -1 }) == vec[height * width - 1]);
  BOOST_CHECK_THROW(raster.at({ width, 0 }), OutOfBoundsError);
  BOOST_CHECK_THROW(raster.at({ -1 - width, 0 }), OutOfBoundsError);
  BOOST_CHECK_THROW(raster.at({ 0, height }), OutOfBoundsError);
  BOOST_CHECK_THROW(raster.at({ 0, -1 - height }), OutOfBoundsError);
}

BOOST_FIXTURE_TEST_CASE(vecraster_move_test, Test::SmallRaster) {
  const auto copied = this->vector();
  std::vector<Value> moved;
  this->moveTo(moved);
  BOOST_TEST(moved == copied);
  BOOST_TEST(this->vector().size() == 0);
}

BOOST_AUTO_TEST_CASE(make_raster_test) {
  constexpr long width = 16;
  constexpr long height = 9;
  constexpr long depth = 3;
  short data2[width * height] = { 0 };
  const short constData2[width * height] = { 0 };
  float data3[width * height * depth] = { 0 };
  const float constData3[width * height * depth] = { 0 };
  const auto raster2 = makeRaster({ width, height }, data2);
  const auto constRaster2 = makeRaster({ width, height }, constData2);
  const auto raster3 = makeRaster<3>({ width, height, depth }, data3);
  const auto constRaster3 = makeRaster<3>({ width, height, depth }, constData3);
  const auto rasterDyn = makeRaster<-1>({ width, height, depth }, data3);
  const auto constRasterDyn = makeRaster<-1>({ width, height, depth }, constData3);
  BOOST_TEST(raster2.dimension() == 2);
  BOOST_TEST(constRaster2.dimension() == 2);
  BOOST_TEST(raster3.dimension() == 3);
  BOOST_TEST(constRaster3.dimension() == 3);
  BOOST_TEST(rasterDyn.dimension() == 3);
  BOOST_TEST(constRasterDyn.dimension() == 3);
}
//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

// Copyright (C) 2019-2022, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "EleFits/BintableHdu.h"
#include "EleFits/FitsFileFixture.h"
#include "EleFits/MefFile.h"
#include "EleFitsData/TestColumn.h"
#include "ElementsKernel/Temporary.h"

#include <boost/test/unit_test.hpp>

using namespace Euclid::Fits;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(BintableHdu_test)

//-----------------------------------------------------------------------------

template <typename T>
void checkScalar() {
  Test::RandomScalarColumn<T> input;
  Test::TemporaryMefFile file;
  file.append_bintable("BINEXT", {}, input);
  const auto output = file.find<BintableHdu>("BINEXT").readColumn<T>(input.info().name);
  BOOST_TEST(output.container() == input.container());
}

template <typename T>
void checkVector() {
  constexpr long row_count = 10;
  constexpr long repeat_count = 2;
  Test::RandomScalarColumn<T> input(row_count * repeat_count);
  input.reshape(repeat_count);
  Test::TemporaryMefFile file;
  file.append_bintable_header("BINEXT", {}, input.info());
  file.find<BintableHdu>("BINEXT").writeColumn(input);
  const auto output = file.find<BintableHdu>("BINEXT").readColumn<T>(input.info().name);
}

/**
 * We test only one type here to check the flow from the top-level API to CFITSIO.
 * Support for other types is tested in EleCfitsioWrapper.
 */
BOOST_AUTO_TEST_CASE(float_test) {
  checkScalar<float>();
  checkVector<float>();
}

BOOST_AUTO_TEST_CASE(empty_column_test) {
  const std::string filename = Elements::TempFile().path().string();
  VecColumn<float> input({"NAME", "", 1}, std::vector<float>());
  MefFile file(filename, FileMode::Temporary);
  file.append_bintable("BINEXT", {}, input);
}

BOOST_FIXTURE_TEST_CASE(colsize_mismatch_test, Test::TemporaryMefFile) {
  VecColumn<float> input0({"COL0", "", 1}, std::vector<float>());
  VecColumn<float> input1({"COL1", "", 1}, std::vector<float> {0});
  VecColumn<float> input2({"COL1", "", 1}, std::vector<float> {0, 1});
  BOOST_CHECK_THROW(this->append_bintable("0AND1", {}, input0, input1), FitsError);
  BOOST_CHECK_THROW(this->append_bintable("1AND0", {}, input1, input0), FitsError);
  BOOST_CHECK_THROW(this->append_bintable("1AND2", {}, input1, input2), FitsError);
  BOOST_CHECK_THROW(this->append_bintable("2AND1", {}, input2, input1), FitsError);
}

BOOST_FIXTURE_TEST_CASE(counting_test, Test::TemporaryMefFile) {
  const std::string name1 = "COL1";
  Test::RandomScalarColumn<std::string> column1;
  column1.rename(name1);
  const std::string name2 = "COL2";
  Test::RandomScalarColumn<double> column2;
  column2.rename(name2);
  const auto& ext = append_bintable("", {}, column1, column2);
  const auto& du = ext.columns();
  BOOST_TEST(du.readColumnCount() == 2);
  BOOST_TEST(du.readRowCount() == column1.row_count());
  BOOST_TEST(du.has(name1));
  BOOST_TEST(du.has(name2));
  BOOST_TEST(not du.has("NOTHERE"));
}

BOOST_FIXTURE_TEST_CASE(multi_column_test, Test::TemporaryMefFile) {
  const auto intColumn = Test::RandomTable::generate_column<int>("INT");
  const auto floatColumn = Test::RandomTable::generate_column<float>("FLOAT");
  const auto& ext = append_bintable("", {}, intColumn, floatColumn);
  const auto& du = ext.columns();
  const auto byName = du.readSeq(as<int>(intColumn.info().name), as<float>(floatColumn.info().name));
  BOOST_TEST(std::get<0>(byName).container() == intColumn.container());
  BOOST_TEST(std::get<1>(byName).container() == floatColumn.container());
  const auto byIndex = du.readSeq(as<int>(0), as<float>(1));
  BOOST_TEST(std::get<0>(byIndex).container() == intColumn.container());
  BOOST_TEST(std::get<1>(byIndex).container() == floatColumn.container());
}

BOOST_FIXTURE_TEST_CASE(column_renaming_test, Test::TemporaryMefFile) {
  std::vector<ColumnInfo<int>> header {{"A"}, {"B"}, {"C"}};
  const auto& ext = append_bintable_header("TABLE", {}, header[0], header[1], header[2]);
  const auto& du = ext.columns();
  auto names = du.readAllNames();
  for (std::size_t i = 0; i < header.size(); ++i) {
    BOOST_TEST(du.readName(i) == header[i].name);
    BOOST_TEST(names[i] == header[i].name);
  }
  header[0].name = "A2";
  header[2].name = "C2";
  du.rename(0, header[0].name);
  du.rename("C", header[2].name);
  names = du.readAllNames();
  for (std::size_t i = 0; i < header.size(); ++i) {
    BOOST_TEST(du.readName(i) == header[i].name);
    BOOST_TEST(names[i] == header[i].name);
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

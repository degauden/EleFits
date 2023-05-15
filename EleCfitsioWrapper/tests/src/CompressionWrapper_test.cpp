// Copyright (C) 2019-2023, CNES and contributors (for the Euclid Science Ground Segment)
// This file is part of EleFits <github.com/CNES/EleFits>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "EleCfitsioWrapper/CompressionWrapper.h"

#include <boost/test/unit_test.hpp>
#include <string>

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(Compression_test)

//-----------------------------------------------------------------------------

using namespace Euclid::Fits::Compression;

BOOST_AUTO_TEST_CASE(quantification_test) {

  // TODO adapt to new Qunatification class
  // const float positiveLevel = 5.f;
  // const float zeroLevel = 0.f;
  // const float negativeLevel = -5.f;

  // const Quantification absPositive_q = Quantification::absolute(positiveLevel);
  // const Quantification absZero_q = Quantification::absolute(zeroLevel);
  // BOOST_CHECK_THROW(Quantification::absolute(negativeLevel), Euclid::Fits::FitsError);

  // BOOST_TEST(absPositive_q.level() == positiveLevel);
  // BOOST_TEST(absPositive_q.isAbsolute() == true);

  // BOOST_TEST(absZero_q.level() == zeroLevel);
  // BOOST_TEST(absZero_q.isAbsolute() == false); // zero always considered relative

  // const Quantification relPositive_q = Quantification::relativeToNoise(positiveLevel);
  // const Quantification relZero_q = Quantification::relativeToNoise(zeroLevel);
  // BOOST_CHECK_THROW(Quantification::relativeToNoise(negativeLevel), Euclid::Fits::FitsError);

  // BOOST_TEST(relPositive_q.level() == positiveLevel);
  // BOOST_TEST(relPositive_q.isAbsolute() == false);

  // BOOST_TEST(relZero_q.level() == zeroLevel);
  // BOOST_TEST(relZero_q.isAbsolute() == false);
}

BOOST_AUTO_TEST_CASE(scale_test) {

  const float positiveFactor = 5.f;
  const float zeroFactor = 0.f;
  const float negativeFactor = -5.f;

  const Scale absPositive_s = Scale::absolute(positiveFactor);
  const Scale absZero_s = Scale::absolute(zeroFactor);
  BOOST_CHECK_THROW(Scale::absolute(negativeFactor), Euclid::Fits::FitsError);

  BOOST_TEST(absPositive_s.factor() == positiveFactor);
  BOOST_TEST(absPositive_s.isAbsolute() == true);

  BOOST_TEST(absZero_s.factor() == zeroFactor);
  BOOST_TEST(absZero_s.isAbsolute() == false); // zero always considered relative

  const Scale relPositive_s = Scale::relativeToNoise(positiveFactor);
  const Scale relZero_s = Scale::relativeToNoise(zeroFactor);
  BOOST_CHECK_THROW(Scale::relativeToNoise(negativeFactor), Euclid::Fits::FitsError);

  BOOST_TEST(relPositive_s.factor() == positiveFactor);
  BOOST_TEST(relPositive_s.isAbsolute() == false);

  BOOST_TEST(relZero_s.factor() == zeroFactor);
  BOOST_TEST(relZero_s.isAbsolute() == false);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(CompressionWrapper_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(wrapper_test) {

  // TODO
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Compression_learning_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(default_values_learning_test) {

  int status = 0;
  fitsfile* fptr;
  fits_create_file(&fptr, (std::string("!learning_test.fits")).c_str(), &status);
  Euclid::Cfitsio::CfitsioError::mayThrow(status, fptr, "Cannot create file");

  int defaultAlgo;
  fits_get_compression_type(fptr, &defaultAlgo, &status);
  Euclid::Cfitsio::CfitsioError::mayThrow(status, fptr, "Cannot get compression type");
  BOOST_TEST(defaultAlgo == NULL);

  float defaultLevel;
  fits_get_quantize_level(fptr, &defaultLevel, &status);
  Euclid::Cfitsio::CfitsioError::mayThrow(status, fptr, "Cannot get quantize level");
  BOOST_TEST(defaultLevel == 0.0); // default should be 4.0 according to doc

  float defaultScale;
  fits_get_hcomp_scale(fptr, &defaultScale, &status);
  Euclid::Cfitsio::CfitsioError::mayThrow(status, fptr, "Cannot get hcompress scale");
  BOOST_TEST(defaultScale == 0.0);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
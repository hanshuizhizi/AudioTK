/**
 * \ file FlushToZero.cpp
 */

#include <cmath>

#include <ATK/Utility/FlushToZero.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(FTZ_test)
{
  ATK::FlushToZero ftz;
  BOOST_CHECK_THROW(throw std::runtime_error("check"), std::runtime_error); // to make unit test happy
}

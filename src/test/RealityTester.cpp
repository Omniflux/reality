/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

//! This file implements the test suite for Reality

#define BOOST_TEST_MODULE RealityTester

#include <boost/test/included/unit_test.hpp>

#include "ReGlossy.h"

BOOST_AUTO_TEST_CASE(test_Glossy) {
    auto gl = new Reality::Glossy("GlossyTest", 0);
	BOOST_CHECK(!gl->getName().isEmpty());
}


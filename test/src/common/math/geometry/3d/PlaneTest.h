#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/Test.h>
#include <UrchinCommon.h>

class PlaneTest : public CppUnit::TestFixture {
    public:
        static CppUnit::Test* suite();

        void orthogonalProjection();
};

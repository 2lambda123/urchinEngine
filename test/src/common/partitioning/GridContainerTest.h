#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/Test.h>

struct MyGridItem {
    public:
        explicit MyGridItem(const urchin::Point3<int>&);

        const urchin::Point3<int>& getGridPosition() const;

    private:
        urchin::Point3<int> position;
};

class GridContainerTest : public CppUnit::TestFixture {
    public:
        static CppUnit::Test* suite();

        void findXPositiveNeighbor();
        void findXNegativeNeighbor();
        void findYPositiveAllDirectNeighbors();
        void findYNegativeAllDirectNeighbors();

        void removeNeighbor();

        void checkItemExist();
};

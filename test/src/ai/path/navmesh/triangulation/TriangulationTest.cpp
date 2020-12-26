#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include "UrchinCommon.h"

#include "TriangulationTest.h"
#include "AssertHelper.h"
using namespace urchin;

void TriangulationTest::triangleTriangulation() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(0.0f, 0.0f), Point2<float>(2.0f, 0.0f), Point2<float>(1.0f, 1.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 1);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{1, 2, 0});
    AssertHelper::instance()->assertTrue(triangles[0]->getLinks().empty());
}

void TriangulationTest::cubeTriangulation() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 0.0f),
                                                   Point2<float>(1.0f, 1.0f), Point2<float>(0.0f, 1.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 2);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{0, 2, 3});
    assertUniqueLink(triangles[0], 0, triangles[1]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{1, 2, 0});
    assertUniqueLink(triangles[1], 1, triangles[0]);
}

void TriangulationTest::twoNearPoints() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(500.0f, 500.0f), Point2<float>(0.0f, 3.0f), Point2<float>(2.0f, 3.0f), Point2<float>(2.0f, 2.0f),
                                                   Point2<float>(2.0000002f, 2.0f), Point2<float>(3.0f, 1.5f), Point2<float>(5.0f, 1.5f), Point2<float>(0.0f, 0.0f),
                                                   Point2<float>(6.0f, 0.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 7);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{2, 0, 1});
    assertUniqueLink(triangles[0], 0, triangles[1]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{3, 0, 2});
    AssertHelper::instance()->assertUnsignedInt(triangles[1]->getLinks().size(), 2);
    assertLink(triangles[1]->getLinks()[0], 1, triangles[0]);
    assertLink(triangles[1]->getLinks()[1], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{4, 0, 3});
    AssertHelper::instance()->assertUnsignedInt(triangles[2]->getLinks().size(), 2);
    assertLink(triangles[2]->getLinks()[0], 1, triangles[1]);
    assertLink(triangles[2]->getLinks()[1], 0, triangles[3]);

    AssertHelper::instance()->assert3Sizes(triangles[3]->getIndices(), new std::size_t[3]{5, 0, 4});
    AssertHelper::instance()->assertUnsignedInt(triangles[3]->getLinks().size(), 2);
    assertLink(triangles[3]->getLinks()[0], 1, triangles[2]);
    assertLink(triangles[3]->getLinks()[1], 0, triangles[4]);

    AssertHelper::instance()->assert3Sizes(triangles[4]->getIndices(), new std::size_t[3]{6, 0, 5});
    AssertHelper::instance()->assertUnsignedInt(triangles[4]->getLinks().size(), 2);
    assertLink(triangles[4]->getLinks()[0], 1, triangles[3]);
    assertLink(triangles[4]->getLinks()[1], 0, triangles[6]);

    AssertHelper::instance()->assert3Sizes(triangles[5]->getIndices(), new std::size_t[3]{8, 6, 7});
    assertUniqueLink(triangles[5], 0, triangles[6]);

    AssertHelper::instance()->assert3Sizes(triangles[6]->getIndices(), new std::size_t[3]{8, 0, 6});
    AssertHelper::instance()->assertUnsignedInt(triangles[6]->getLinks().size(), 2);
    assertLink(triangles[6]->getLinks()[0], 2, triangles[5]);
    assertLink(triangles[6]->getLinks()[1], 1, triangles[4]);
}

void TriangulationTest::threeAlignedPoints() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(0.0f, 5.0f), Point2<float>(-1.0f, 4.0f), Point2<float>(0.0f, 0.0f),
                                                   Point2<float>(1.0f, 1.0f), Point2<float>(1.0f, 2.0f), Point2<float>(1.0f, 3.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 4);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{1, 5, 0});
    assertUniqueLink(triangles[0], 0, triangles[1]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{1, 4, 5});
    AssertHelper::instance()->assertUnsignedInt(triangles[1]->getLinks().size(), 2);
    assertLink(triangles[1]->getLinks()[0], 2, triangles[0]);
    assertLink(triangles[1]->getLinks()[1], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{1, 3, 4});
    AssertHelper::instance()->assertUnsignedInt(triangles[2]->getLinks().size(), 2);
    assertLink(triangles[2]->getLinks()[0], 2, triangles[1]);
    assertLink(triangles[2]->getLinks()[1], 0, triangles[3]);

    AssertHelper::instance()->assert3Sizes(triangles[3]->getIndices(), new std::size_t[3]{1, 2, 3});
    assertUniqueLink(triangles[3], 2, triangles[2]);
}

void TriangulationTest::alternationPoints() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(0.0f, 6.0f), Point2<float>(-1.0f, 4.0f), Point2<float>(-1.0f, 2.0f),
                                                   Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 1.0f), Point2<float>(1.0f, 3.0f),
                                                   Point2<float>(1.0f, 5.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 5);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{1, 6, 0});
    assertUniqueLink(triangles[0], 0, triangles[1]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{1, 5, 6});
    AssertHelper::instance()->assertUnsignedInt(triangles[1]->getLinks().size(), 2);
    assertLink(triangles[1]->getLinks()[0], 2, triangles[0]);
    assertLink(triangles[1]->getLinks()[1], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{2, 5, 1});
    AssertHelper::instance()->assertUnsignedInt(triangles[2]->getLinks().size(), 2);
    assertLink(triangles[2]->getLinks()[0], 1, triangles[1]);
    assertLink(triangles[2]->getLinks()[1], 0, triangles[3]);

    AssertHelper::instance()->assert3Sizes(triangles[3]->getIndices(), new std::size_t[3]{2, 4, 5});
    AssertHelper::instance()->assertUnsignedInt(triangles[3]->getLinks().size(), 2);
    assertLink(triangles[3]->getLinks()[0], 2, triangles[2]);
    assertLink(triangles[3]->getLinks()[1], 0, triangles[4]);

    AssertHelper::instance()->assert3Sizes(triangles[4]->getIndices(), new std::size_t[3]{2, 3, 4});
    assertUniqueLink(triangles[4], 2, triangles[3]);
}

void TriangulationTest::cavityTriangulation1() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(3.0f, 4.0f), Point2<float>(0.0f, 3.0f), Point2<float>(1.0f, 2.0f),
                                                   Point2<float>(0.0f, 1.75f), Point2<float>(2.0f, 0.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 3);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{2, 0, 1});
    assertUniqueLink(triangles[0], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{4, 2, 3});
    assertUniqueLink(triangles[1], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{4, 0, 2});
    AssertHelper::instance()->assertUnsignedInt(triangles[2]->getLinks().size(), 2);
    assertLink(triangles[2]->getLinks()[0], 2, triangles[1]);
    assertLink(triangles[2]->getLinks()[1], 1, triangles[0]);
}

void TriangulationTest::cavityTriangulation2() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(2.0f, 0.0f), Point2<float>(3.0f, 5.0f), Point2<float>(0.0f, 3.0f),
                                                   Point2<float>(1.0f, 2.5f), Point2<float>(0.0f, 2.0f), Point2<float>(1.0f, 1.5f),
                                                   Point2<float>(0.0f, 1.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 5);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{3, 1, 2});
    assertUniqueLink(triangles[0], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{5, 3, 4});
    assertUniqueLink(triangles[1], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{5, 1, 3});
    AssertHelper::instance()->assertUnsignedInt(triangles[2]->getLinks().size(), 3);
    assertLink(triangles[2]->getLinks()[0], 2, triangles[1]);
    assertLink(triangles[2]->getLinks()[1], 1, triangles[0]);
    assertLink(triangles[2]->getLinks()[2], 0, triangles[4]);

    AssertHelper::instance()->assert3Sizes(triangles[3]->getIndices(), new std::size_t[3]{0, 5, 6});
    assertUniqueLink(triangles[3], 0, triangles[4]);

    AssertHelper::instance()->assert3Sizes(triangles[4]->getIndices(), new std::size_t[3]{0, 1, 5});
    AssertHelper::instance()->assertUnsignedInt(triangles[4]->getLinks().size(), 2);
    assertLink(triangles[4]->getLinks()[0], 2, triangles[3]);
    assertLink(triangles[4]->getLinks()[1], 1, triangles[2]);
}

void TriangulationTest::twoMonotonePolygons() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(0.0f, 0.0f), Point2<float>(4.0f, 0.0f), Point2<float>(3.0f, 2.0f),
                                                   Point2<float>(2.0f, 1.0f), Point2<float>(1.0f, 2.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 3);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{4, 0, 3});
    assertUniqueLink(triangles[0], 1, triangles[1]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{1, 3, 0});
    AssertHelper::instance()->assertUnsignedInt(triangles[1]->getLinks().size(), 2);
    assertLink(triangles[1]->getLinks()[0], 1, triangles[0]);
    assertLink(triangles[1]->getLinks()[1], 0, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{1, 2, 3});
    assertUniqueLink(triangles[2], 2, triangles[1]);
}

void TriangulationTest::threeMonotonePolygons() {
    std::vector<Point2<float>> ccwPolygonPoints = {Point2<float>(0.0f, 0.0f), Point2<float>(4.0f, 0.0f), Point2<float>(4.0f, 3.0f),
                                                   Point2<float>(3.0f, 1.0f), Point2<float>(2.0f, 2.0f), Point2<float>(1.0f, 1.0f),
                                                   Point2<float>(0.0f, 2.0f)};

    TriangulationAlgorithm triangulationAlgorithm(std::move(ccwPolygonPoints), "test");
    std::vector<std::shared_ptr<NavTriangle>> triangles = triangulationAlgorithm.triangulate();

    AssertHelper::instance()->assertUnsignedInt(triangles.size(), 5);
    AssertHelper::instance()->assert3Sizes(triangles[0]->getIndices(), new std::size_t[3]{5, 0, 3});
    AssertHelper::instance()->assertUnsignedInt(triangles[0]->getLinks().size(), 3);
    assertLink(triangles[0]->getLinks()[0], 0, triangles[1]);
    assertLink(triangles[0]->getLinks()[1], 1, triangles[2]);
    assertLink(triangles[0]->getLinks()[2], 2, triangles[4]);

    AssertHelper::instance()->assert3Sizes(triangles[1]->getIndices(), new std::size_t[3]{6, 0, 5});
    assertUniqueLink(triangles[1], 1, triangles[0]);

    AssertHelper::instance()->assert3Sizes(triangles[2]->getIndices(), new std::size_t[3]{1, 3, 0});
    AssertHelper::instance()->assertUnsignedInt(triangles[2]->getLinks().size(), 2);
    assertLink(triangles[2]->getLinks()[0], 1, triangles[0]);
    assertLink(triangles[2]->getLinks()[1], 0, triangles[3]);

    AssertHelper::instance()->assert3Sizes(triangles[3]->getIndices(), new std::size_t[3]{1, 2, 3});
    assertUniqueLink(triangles[3], 2, triangles[2]);

    AssertHelper::instance()->assert3Sizes(triangles[4]->getIndices(), new std::size_t[3]{3, 4, 5});
    assertUniqueLink(triangles[4], 2, triangles[0]);
}

CppUnit::Test* TriangulationTest::suite() {
    auto* suite = new CppUnit::TestSuite("TriangulationTest");

    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("triangleTriangulation", &TriangulationTest::triangleTriangulation));
    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("cubeTriangulation", &TriangulationTest::cubeTriangulation));
    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("twoNearPoints", &TriangulationTest::twoNearPoints));
    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("threeAlignedPoints", &TriangulationTest::threeAlignedPoints));
    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("alternationPoints", &TriangulationTest::alternationPoints));

    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("cavityTriangulation1", &TriangulationTest::cavityTriangulation1));
    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("cavityTriangulation2", &TriangulationTest::cavityTriangulation2));

    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("twoMonotonePolygons", &TriangulationTest::twoMonotonePolygons));
    suite->addTest(new CppUnit::TestCaller<TriangulationTest>("threeMonotonePolygons", &TriangulationTest::threeMonotonePolygons));

    return suite;
}

void TriangulationTest::assertUniqueLink(const std::shared_ptr<NavTriangle>& sourceTriangle, std::size_t sourceEdgeIndex, const std::shared_ptr<NavTriangle>& targetTriangle) {
    AssertHelper::instance()->assertUnsignedInt(sourceTriangle->getLinks().size(), 1);
    assertLink(sourceTriangle->getLinks()[0], sourceEdgeIndex, targetTriangle);
}

void TriangulationTest::assertLink(const std::shared_ptr<NavLink>& link, std::size_t sourceEdgeIndex, const std::shared_ptr<NavTriangle>& targetTriangle) {
    AssertHelper::instance()->assertUnsignedInt(link->getSourceEdgeIndex(), sourceEdgeIndex);
    AssertHelper::instance()->assertTrue(link->getTargetTriangle() == targetTriangle);
}

#include <cppunit/extensions/HelperMacros.h>

#include <common/collision/GJKAlgorithmTest.h>
#include <AssertHelper.h>
using namespace urchin;

void GJKAlgorithmTest::identicalAABBox() {
    AABBox aabbox1(Point3<float>(0.0f, 0.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));
    AABBox aabbox2(Point3<float>(0.0f, 0.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(aabbox1, aabbox2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateAABBox() {
    AABBox aabbox1(Point3<float>(0.0f, 0.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));
    AABBox aabbox2(Point3<float>(-1.1f, 0.0, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(aabbox1, aabbox2);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 0.1f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().X, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().X, -0.1f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().vector(result.getClosestPointA()).length(), 0.1f);
}

void GJKAlgorithmTest::overlapAABBox() {
    AABBox aabbox1(Point3<float>(0.0f, 0.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));
    AABBox aabbox2(Point3<float>(-0.5f, 0.0, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(aabbox1, aabbox2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::identicalOBBox() {
    OBBox obbox1(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));
    OBBox obbox2(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(obbox1, obbox2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateEdgeOBBox() {
    OBBox obbox1(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));
    OBBox obbox2(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(3.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(obbox1, obbox2);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getClosestPointA().Y, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().X, 1.58578643763f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().Y, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().vector(result.getClosestPointA()).length(), 0.17157287526f);
}

void GJKAlgorithmTest::separateCornerOBBox() {
    OBBox obbox1(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::fromAxisAngle(Vector3<float>(0.245f, 0.769f, -0.59f), 0.987859f));
    OBBox obbox2(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(3.0f, 0.0f, 0.0f), Quaternion<float>());

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(obbox1, obbox2);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 0.26794919243f);
    AssertHelper::assertPoint3FloatEquals(result.getClosestPointA(), Point3<float>(1.73205080757f, 0.0f, 0.0f));
    AssertHelper::assertPoint3FloatEquals(result.getClosestPointB(), Point3<float>(2.0f, 0.0f, 0.0f));
}

void GJKAlgorithmTest::overlapOnEdgeOBBox() {
    OBBox obbox1(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));
    OBBox obbox2(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(2.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(obbox1, obbox2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::identicalBox() {
    std::vector<Point3<float>> obbPoints1 = {
            Point3<float>(-0.3f, 1.0f, 0.0f), Point3<float>(0.2f, 0.0f, 0.0f), Point3<float>(-0.3f, -1.0f, 0.0f), Point3<float>(-0.8f, 0.0f, 0.0f),
            Point3<float>(-0.3f, 1.0f, -1.0f), Point3<float>(0.2f, 0.0f, -1.0f), Point3<float>(-0.3f, -1.0f, -1.0f), Point3<float>(-0.8f, 0.0f, -1.0f)
    };
    std::vector<Point3<float>> obbPoints2 = {
            Point3<float>(-0.3f, 1.0f, 0.0f), Point3<float>(0.2f, 0.0f, 0.0f), Point3<float>(-0.3f, -1.0f, 0.0f), Point3<float>(-0.8f, 0.0f, 0.0f),
            Point3<float>(-0.3f, 1.0f, -1.0f), Point3<float>(0.2f, 0.0f, -1.0f), Point3<float>(-0.3f, -1.0f, -1.0f), Point3<float>(-0.8f, 0.0f, -1.0f)
    };
    ConvexHull3D ch1(obbPoints1);
    ConvexHull3D ch2(obbPoints2);

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(ch1, ch2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateBox() {
    std::vector<Point3<float>> aabbPoints = {
            Point3<float>(0.0f, 1.0f, 0.0f), Point3<float>(1.0f, 1.0f, 0.0f), Point3<float>(1.0f, -1.0f, 0.0f), Point3<float>(0.0f, -1.0f, 0.0f),
            Point3<float>(0.0f, 1.0f, -1.0f), Point3<float>(1.0f, 1.0f, -1.0f), Point3<float>(1.0f, -1.0f, -1.0f), Point3<float>(0.0f, -1.0f, -1.0f)
    };
    std::vector<Point3<float>> obbPoints = {
            Point3<float>(-0.6f, 1.0f, 0.0f), Point3<float>(-0.1f, 0.0f, 0.0f), Point3<float>(-0.6f, -1.0f, 0.0f), Point3<float>(-1.1f, 0.0f, 0.0f),
            Point3<float>(-0.6f, 1.0f, -1.0f), Point3<float>(-0.1f, 0.0f, -1.0f), Point3<float>(-0.6f, -1.0f, -1.0f), Point3<float>(-1.1f, 0.0f, -1.0f)
    };
    ConvexHull3D ch1(aabbPoints);
    ConvexHull3D ch2(obbPoints);

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(ch1, ch2);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 0.1f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().X, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().Y, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().X, -0.1f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().Y, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().vector(result.getClosestPointA()).length(), 0.1f);
}

void GJKAlgorithmTest::cornerInsideBox() {
    std::vector<Point3<float>> aabbPoints = {
            Point3<float>(0.0f, 1.0f, 0.0f), Point3<float>(1.0f, 1.0f, 0.0f), Point3<float>(1.0f, -1.0f, 0.0f), Point3<float>(0.0f, -1.0f, 0.0f),
            Point3<float>(0.0f, 1.0f, -1.0f), Point3<float>(1.0f, 1.0f, -1.0f), Point3<float>(1.0f, -1.0f, -1.0f), Point3<float>(0.0f, -1.0f, -1.0f)
    };
    std::vector<Point3<float>> obbPoints = {
            Point3<float>(-0.3f, 1.0f, 0.0f), Point3<float>(0.2f, 0.0f, 0.0f), Point3<float>(-0.3f, -1.0f, 0.0f), Point3<float>(-0.8f, 0.0f, 0.0f),
            Point3<float>(-0.3f, 1.0f, -1.0f), Point3<float>(0.2f, 0.0f, -1.0f), Point3<float>(-0.3f, -1.0f, -1.0f), Point3<float>(-0.8f, 0.0f, -1.0f)
    };
    ConvexHull3D ch1(aabbPoints);
    ConvexHull3D ch2(obbPoints);

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(ch1, ch2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::faceInsideTrapeze() {
    std::vector<Point3<float>> trapezePoints1 = {
            Point3<float>(1.0f, 0.0f, 0.0f), Point3<float>(3.0f, 0.0f, 0.0f), Point3<float>(0.0f, 3.0f, 0.0f), Point3<float>(0.0f, 1.0f, 0.0f),
            Point3<float>(1.0f, 0.0f, -1.0f), Point3<float>(3.0f, 0.0f, -1.0f), Point3<float>(0.0f, 3.0f, -1.0f), Point3<float>(0.0f, 1.0f, -1.0f),
    };
    std::vector<Point3<float>> trapezePoints2 = {
            Point3<float>(0.0f, -1.0f, 0.0f), Point3<float>(1.0f, 0.8f, 0.0f), Point3<float>(0.8f, 1.0f, 0.0f), Point3<float>(-1.0f, 0.0f, 0.0f),
            Point3<float>(0.0f, -1.0f, -1.0f), Point3<float>(1.0f, 0.8f, -1.0f), Point3<float>(0.8f, 1.0f, -1.0f), Point3<float>(-1.0f, 0.0f, -1.0f),
    };
    ConvexHull3D ch1(trapezePoints1);
    ConvexHull3D ch2(trapezePoints2);

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(ch1, ch2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateHexagon() {
    std::vector<Point3<float>> hexagonPoints1 = {
            Point3<float>(0.0f, 0.0f, 0.0f), Point3<float>(1.0f, 0.0f, 0.0f), Point3<float>(1.5f, 1.0f, 0.0f), Point3<float>(1.0f, 2.0f, 0.0f), Point3<float>(0.0f, 2.0f, 0.0f), Point3<float>(-0.5f, 1.0f, 0.0f),
            Point3<float>(0.0f, 0.0f, -1.0f), Point3<float>(1.0f, 0.0f, -1.0f), Point3<float>(1.5f, 1.0f, -1.0f), Point3<float>(1.0f, 2.0f, -1.0f), Point3<float>(0.0f, 2.0f, -1.0f), Point3<float>(-0.5f, 1.0f, -1.0f),
    };
    std::vector<Point3<float>> hexagonPoints2 = {
            Point3<float>(-2.5f, 0.0f, 0.0f), Point3<float>(-1.5f, 0.0f, 0.0f), Point3<float>(-1.0f, 1.0f, 0.0f), Point3<float>(-1.5f, 2.0f, 0.0f), Point3<float>(-2.5f, 2.0f, 0.0f), Point3<float>(-3.0f, 1.0f, 0.0f),
            Point3<float>(-2.5f, 0.0f, -1.0f), Point3<float>(-1.5f, 0.0f, -1.0f), Point3<float>(-1.0f, 1.0f, -1.0f), Point3<float>(-1.5f, 2.0f, -1.0f), Point3<float>(-2.5f, 2.0f, -1.0f), Point3<float>(-3.0f, 1.0f, -1.0f),
    };
    ConvexHull3D ch1(hexagonPoints1);
    ConvexHull3D ch2(hexagonPoints2);

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(ch1, ch2);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 0.5f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().X, -0.5f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().Y, 1.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().X, -1.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().Y, 1.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().vector(result.getClosestPointA()).length(), 0.5f);
}

void GJKAlgorithmTest::cornerInsideHexagon() {
    std::vector<Point3<float>> hexagonPoints1 = {
            Point3<float>(0.0f, 0.0f, 0.0f), Point3<float>(1.0f, 0.0f, 0.0f), Point3<float>(1.5f, 1.0f, 0.0f), Point3<float>(1.0f, 2.0f, 0.0f), Point3<float>(0.0f, 2.0f, 0.0f), Point3<float>(-0.5f, 1.0f, 0.0f),
            Point3<float>(0.0f, 0.0f, -1.0f), Point3<float>(1.0f, 0.0f, -1.0f), Point3<float>(1.5f, 1.0f, -1.0f), Point3<float>(1.0f, 2.0f, -1.0f), Point3<float>(0.0f, 2.0f, -1.0f), Point3<float>(-0.5f, 1.0f, -1.0f),
    };
    std::vector<Point3<float>> hexagonPoints2 = {
            Point3<float>(-1.5f, 0.0f, 0.0f), Point3<float>(-0.5f, 0.0f, 0.0f), Point3<float>(0.0f, 1.0f, 0.0f), Point3<float>(-0.5f, 2.0f, 0.0f), Point3<float>(-1.5f, 2.0f, 0.0f), Point3<float>(-2.0f, 1.0f, 0.0f),
            Point3<float>(-1.5f, 0.0f, -1.0f), Point3<float>(-0.5f, 0.0f, -1.0f), Point3<float>(0.0f, 1.0f, -1.0f), Point3<float>(-0.5f, 2.0f, -1.0f), Point3<float>(-1.5f, 2.0f, -1.0f), Point3<float>(-2.0f, 1.0f, -1.0f),
    };
    ConvexHull3D ch1(hexagonPoints1);
    ConvexHull3D ch2(hexagonPoints2);

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(ch1, ch2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::identicalSphere() {
    Sphere sphere1(10.0f, Point3<float>(1.0f, 0.0f, 0.0f));
    Sphere sphere2(10.0f, Point3<float>(1.0f, 0.0f, 0.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(sphere1, sphere2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateSphere() {
    Sphere sphere1(10.0f, Point3<float>(0.0f, 0.0f, 0.0f));
    Sphere sphere2(10.0f, Point3<float>(7.0710678118f + 8.0f, 7.0710678118f + 8.0f, 0.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(sphere1, sphere2);

    float epsilon = 0.01f; //high epsilon used because curved shapes are bad case for GJK
    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 1.31370849889f, epsilon);
    AssertHelper::assertPoint3FloatEquals(result.getClosestPointA(), Point3<float>(7.0710678118f, 7.0710678118f, 0.0f), epsilon);
    AssertHelper::assertPoint3FloatEquals(result.getClosestPointB(), Point3<float>(8.0f, 8.0f, 0.0f), epsilon);
}

void GJKAlgorithmTest::overlapSphere() {
    Sphere sphere1(10.0f, Point3<float>(0.0f, 0.0f, 0.0f));
    Sphere sphere2(10.0f, Point3<float>(7.0710678118f + 7.0f, 7.0710678118f + 7.0f, 0.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(sphere1, sphere2);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateOBBoxAABBox() {
    OBBox obbox(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));
    AABBox aabbox(Point3<float>(2.0f, -1.0, -1.0f), Vector3<float>(2.0f, 2.0f, 2.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(obbox, aabbox);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 0.58578643763f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().X, 1.41421356237f);
    AssertHelper::assertFloatEquals(result.getClosestPointA().Y, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().X, 2.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().Y, 0.0f);
    AssertHelper::assertFloatEquals(result.getClosestPointB().vector(result.getClosestPointA()).length(), 0.58578643763f);
}

void GJKAlgorithmTest::overlapOBBoxAABBox() {
    OBBox obbox(Vector3<float>(1.0f, 1.0f, 1.0f), Point3<float>(0.0f, 0.0f, 0.0f), Quaternion<float>::rotationZ(2.35619449f));
    AABBox aabbox(Point3<float>(1.0f, -1.0f, -1.0f), Vector3<float>(2.0f, 2.0f, 2.0f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(obbox, aabbox);

    AssertHelper::assertTrue(result.isCollide());
}

void GJKAlgorithmTest::separateSphereAndAABBox() {
    Sphere sphere(1.0f, Point3<float>(0.0f, 0.0f, 0.0f));
    AABBox aabbox(Point3<float>(1.1f, 0.0f, 0.0f), Vector3<float>(0.5f, 0.5f, 0.5f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(sphere, aabbox);

    AssertHelper::assertTrue(!result.isCollide());
    AssertHelper::assertFloatEquals(result.getSeparatingDistance(), 0.1f);
}

void GJKAlgorithmTest::overlapSphereAndAABBox() {
    Sphere sphere(1.0f, Point3<float>(0.0f, 0.0f, 0.0f));
    AABBox aabbox(Point3<float>(0.9f, 0.0f, 0.0f), Vector3<float>(0.5f, 0.5f, 0.5f));

    GJKResult<float> result = GJKAlgorithm<float>().processGJK(sphere, aabbox);

    AssertHelper::assertTrue(result.isCollide());
}

CppUnit::Test* GJKAlgorithmTest::suite() {
    auto* suite = new CppUnit::TestSuite("GJKAlgorithmTest");

    //AABBox
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("identicalAABBox", &GJKAlgorithmTest::identicalAABBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateAABBox", &GJKAlgorithmTest::separateAABBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("overlapAABBox", &GJKAlgorithmTest::overlapAABBox));

    //OBBox
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("identicalOBBox", &GJKAlgorithmTest::identicalOBBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateEdgeOBBox", &GJKAlgorithmTest::separateEdgeOBBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateCornerOBBox", &GJKAlgorithmTest::separateCornerOBBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("overlapOnEdgeOBBox", &GJKAlgorithmTest::overlapOnEdgeOBBox));

    //Convex hull
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("identicalBox", &GJKAlgorithmTest::identicalBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateBox", &GJKAlgorithmTest::separateBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("cornerInsideBox", &GJKAlgorithmTest::cornerInsideBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("faceInsideTrapeze", &GJKAlgorithmTest::faceInsideTrapeze));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateHexagon", &GJKAlgorithmTest::separateHexagon));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("cornerInsideHexagon", &GJKAlgorithmTest::cornerInsideHexagon));

    //Sphere
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("identicalSphere", &GJKAlgorithmTest::identicalSphere));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateSphere", &GJKAlgorithmTest::separateSphere));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("overlapSphere", &GJKAlgorithmTest::overlapSphere));

    //Different shape type
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateOBBoxAABBox", &GJKAlgorithmTest::separateOBBoxAABBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("overlapOBBoxAABBox", &GJKAlgorithmTest::overlapOBBoxAABBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("separateSphereAndAABBox", &GJKAlgorithmTest::separateSphereAndAABBox));
    suite->addTest(new CppUnit::TestCaller<GJKAlgorithmTest>("overlapSphereAndAABBox", &GJKAlgorithmTest::overlapSphereAndAABBox));

    return suite;
}

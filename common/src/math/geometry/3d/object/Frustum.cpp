#include <cmath>
#include <algorithm>

#include <math/geometry/3d/object/Frustum.h>
#include <math/geometry/3d/Line3D.h>
#include <math/algebra/point/Point4.h>
#include <math/trigonometry/AngleConverter.h>

namespace urchin {

    template<class T> Frustum<T>::Frustum() {
        buildFrustum(90.0, 1.0, 0.01f, 1.0);
    }

    /**
    * Frustum constructor from angle, ratio and near/far distances.
    * Default frustum position: x = 0, y = 0, z = 0
    * Default frustum view direction: z axis
    * Default frustum up vector: y axis
    */
    template<class T> Frustum<T>::Frustum(T fovYAngle, T ratio, T nearDistance, T farDistance) {
        buildFrustum(fovYAngle, ratio, nearDistance, farDistance);
    }

    /**
    * @param ntl Near top left point
    * @param ntr Near top right point
    * @param nbl Near bottom left point
    * @param nbr Near bottom right point
    * @param ftl Far top left point
    * @param ftr Far top right point
    * @param fbl Far bottom left point
    * @param fbr Far bottom right point
    */
    template<class T> Frustum<T>::Frustum(const Point3<T>& ntl, const Point3<T>& ntr, const Point3<T>& nbl, const Point3<T>& nbr,
        const Point3<T>& ftl, const Point3<T>& ftr, const Point3<T>& fbl, const Point3<T>& fbr) {
        buildFrustum(ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr);
    }

    template<class T> Frustum<T>::Frustum(Frustum<T>&& frustum) noexcept :
            position(frustum.getPosition()) {
        frustumPoints[0] = frustum.getFrustumPoints()[0];
        frustumPoints[1] = frustum.getFrustumPoints()[1];
        frustumPoints[2] = frustum.getFrustumPoints()[2];
        frustumPoints[3] = frustum.getFrustumPoints()[3];
        frustumPoints[4] = frustum.getFrustumPoints()[4];
        frustumPoints[5] = frustum.getFrustumPoints()[5];
        frustumPoints[6] = frustum.getFrustumPoints()[6];
        frustumPoints[7] = frustum.getFrustumPoints()[7];

        planes[TOP] = frustum.planes[TOP];
        planes[BOTTOM] = frustum.planes[BOTTOM];
        planes[LEFT] = frustum.planes[LEFT];
        planes[RIGHT] = frustum.planes[RIGHT];
        planes[NEARP] = frustum.planes[NEARP];
        planes[FARP] = frustum.planes[FARP];
    }

    template<class T> Frustum<T>& Frustum<T>::operator=(Frustum<T>&& frustum) noexcept {
        frustumPoints[0] = frustum.getFrustumPoints()[0];
        frustumPoints[1] = frustum.getFrustumPoints()[1];
        frustumPoints[2] = frustum.getFrustumPoints()[2];
        frustumPoints[3] = frustum.getFrustumPoints()[3];
        frustumPoints[4] = frustum.getFrustumPoints()[4];
        frustumPoints[5] = frustum.getFrustumPoints()[5];
        frustumPoints[6] = frustum.getFrustumPoints()[6];
        frustumPoints[7] = frustum.getFrustumPoints()[7];

        position = frustum.getPosition();

        planes[TOP] = frustum.planes[TOP];
        planes[BOTTOM] = frustum.planes[BOTTOM];
        planes[LEFT] = frustum.planes[LEFT];
        planes[RIGHT] = frustum.planes[RIGHT];
        planes[NEARP] = frustum.planes[NEARP];
        planes[FARP] = frustum.planes[FARP];

        return *this;
    }

    /**
    * Frustum builder from angle, ratio and near/far distances.
    * Default frustum position: x = 0, y = 0, z = 0
    * Default frustum view direction: z axis
    * Default frustum up vector: y axis
    */
    template<class T> void Frustum<T>::buildFrustum(T verticalFovAngle, T ratio, T nearDistance, T farDistance) {
        //half distance of near and far planes
        T halfVerticalFov = AngleConverter<T>::toRadian(verticalFovAngle) / 2.0f;
        auto tang = (T)std::tan(halfVerticalFov);
        T nearHalfHeight = nearDistance * tang;
        T nearHalfWidth = nearHalfHeight * ratio;
        T farHalfHeight = farDistance * tang;
        T farHalfWidth = farHalfHeight * ratio;

        //building frustum points
        frustumPoints[0] = Point3<T>(-nearHalfWidth, nearHalfHeight, -nearDistance); //ntl
        frustumPoints[1] = Point3<T>(nearHalfWidth, nearHalfHeight, -nearDistance); //ntr
        frustumPoints[2] = Point3<T>(-nearHalfWidth, -nearHalfHeight, -nearDistance); //nbl
        frustumPoints[3] = Point3<T>(nearHalfWidth, -nearHalfHeight, -nearDistance); //nbr
        frustumPoints[4] = Point3<T>(-farHalfWidth, farHalfHeight, -farDistance); //ftl
        frustumPoints[5] = Point3<T>(farHalfWidth, farHalfHeight, -farDistance); //ftr
        frustumPoints[6] = Point3<T>(-farHalfWidth, -farHalfHeight, -farDistance); //fbl
        frustumPoints[7] = Point3<T>(farHalfWidth, -farHalfHeight, -farDistance); //fbr

        //building frustum data
        buildData();
    }

    /**
    * @param ntl Near top left point
    * @param ntr Near top right point
    * @param nbl Near bottom left point
    * @param nbr Near bottom right point
    * @param ftl Far top left point
    * @param ftr Far top right point
    * @param fbl Far bottom left point
    * @param fbr Far bottom right point
    */
    template<class T> void Frustum<T>::buildFrustum(const Point3<T>& ntl, const Point3<T>& ntr, const Point3<T>& nbl, const Point3<T>& nbr,
        const Point3<T>& ftl, const Point3<T>& ftr, const Point3<T>& fbl, const Point3<T>& fbr) {
        //building frustum points
        frustumPoints[0] = ntl;
        frustumPoints[1] = ntr;
        frustumPoints[2] = nbl;
        frustumPoints[3] = nbr;
        frustumPoints[4] = ftl;
        frustumPoints[5] = ftr;
        frustumPoints[6] = fbl;
        frustumPoints[7] = fbr;

        //building frustum data
        buildData();
    }

    template<class T> void Frustum<T>::buildData() {
        //building planes
        planes[TOP].buildFrom3Points(frustumPoints[FTL], frustumPoints[NTL], frustumPoints[NTR]);
        planes[BOTTOM].buildFrom3Points(frustumPoints[FBR], frustumPoints[NBR], frustumPoints[NBL]);
        planes[LEFT].buildFrom3Points(frustumPoints[FBL], frustumPoints[NBL], frustumPoints[NTL]);
        planes[RIGHT].buildFrom3Points(frustumPoints[FBR], frustumPoints[NTR], frustumPoints[NBR]);
        planes[NEARP].buildFrom3Points(frustumPoints[NBR], frustumPoints[NTR], frustumPoints[NTL]);
        planes[FARP].buildFrom3Points(frustumPoints[FBL], frustumPoints[FTL], frustumPoints[FTR]);

        //compute eye/camera position
        Line3D<T> sideLine(frustumPoints[FTR], frustumPoints[NTR]);
        Point3<T> p1((frustumPoints[FTL] + frustumPoints[FBL]) / (T)2.0);
        Point3<T> p2((frustumPoints[FTR] + frustumPoints[FBR]) / (T)2.0);
        Point3<T> p3((frustumPoints[NTR] + frustumPoints[NBR]) / (T)2.0);
        Plane<T> middlePlane(p1, p2, p3);
        bool hasIntersection = false;
        position = middlePlane.intersectPoint(sideLine, hasIntersection);
    }

    template<class T> const std::array<Point3<T>, 8>& Frustum<T>::getFrustumPoints() const {
        return frustumPoints;
    }

    template<class T> const Point3<T> &Frustum<T>::getFrustumPoint(FrustumPoint frustumPoint) const {
        return frustumPoints[frustumPoint];
    }

    template<class T> const Point3<T> &Frustum<T>::getPosition() const {
        return position;
    }

    template<class T> Point3<T> Frustum<T>::getSupportPoint(const Vector3<T>& direction) const {
        T maxPointDotDirection = frustumPoints[0].toVector().dotProduct(direction);
        Point3<T> maxPoint = frustumPoints[0];

        for (unsigned int i = 1; i < 8; ++i) {
            T currentPointDotDirection  = frustumPoints[i].toVector().dotProduct(direction);
            if (currentPointDotDirection > maxPointDotDirection) {
                maxPointDotDirection = currentPointDotDirection;
                maxPoint = frustumPoints[i];
            }
        }

        return maxPoint;
    }

    template<class T> T Frustum<T>::computeNearDistance() const {
        Point3<T> nearCenter((frustumPoints[NTL] + frustumPoints[NBR]) / (T)2.0);
        return position.distance(nearCenter);
    }

    template<class T> T Frustum<T>::computeFarDistance() const {
        Point3<T> farCenter((frustumPoints[FTL] + frustumPoints[FBR]) / (T)2.0);
        return position.distance(farCenter);
    }

    /**
    * @param splitPositionStart Split position start (distance between original near plane and the asked near plane)
    * @param splitPositionEnd Split position end (distance between original far plane and the asked far plane)
    * @return Split frustum according to the given split positions
    */
    template<class T> Frustum<T> Frustum<T>::splitFrustum(T splitPositionStart, T splitPositionEnd) const {
        Point3<T> nearCenter((frustumPoints[NTL] + frustumPoints[NBR]) / (T)2.0);
        Vector3<T> positionToCenter = position.vector(nearCenter).normalize();

        //top left points
        Vector3<T> sideVector = frustumPoints[NTL].vector(frustumPoints[FTL]).normalize();
        T nearDistOnSideVector = splitPositionStart / positionToCenter.dotProduct(sideVector);
        T farDistOnSideVector = splitPositionEnd / positionToCenter.dotProduct(sideVector);
        Point3<T> ntl = frustumPoints[NTL].translate(sideVector * nearDistOnSideVector);
        Point3<T> ftl = frustumPoints[NTL].translate(sideVector * farDistOnSideVector);

        //top right points
        sideVector = frustumPoints[NTR].vector(frustumPoints[FTR]).normalize();
        Point3<T> ntr = frustumPoints[NTR].translate(sideVector * nearDistOnSideVector);
        Point3<T> ftr = frustumPoints[NTR].translate(sideVector * farDistOnSideVector);

        //bottom left points
        sideVector = frustumPoints[NBL].vector(frustumPoints[FBL]).normalize();
        Point3<T> nbl = frustumPoints[NBL].translate(sideVector * nearDistOnSideVector);
        Point3<T> fbl = frustumPoints[NBL].translate(sideVector * farDistOnSideVector);

        //bottom right points
        sideVector = frustumPoints[NBR].vector(frustumPoints[FBR]).normalize();
        Point3<T> nbr = frustumPoints[NBR].translate(sideVector * nearDistOnSideVector);
        Point3<T> fbr = frustumPoints[NBR].translate(sideVector * farDistOnSideVector);

        return Frustum<T>(ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr);
    }

    template<class T> Frustum<T> Frustum<T>::cutFrustum(T newFar) const {
        Point3<T> nearCenter((frustumPoints[NTL] + frustumPoints[NBR]) / (T)2.0);
        Vector3<T> positionToCenter = position.vector(nearCenter).normalize();

        //top left point
        Vector3<T> sideVector = frustumPoints[NTL].vector(frustumPoints[FTL]).normalize();
        T farDistOnSideVector = newFar / positionToCenter.dotProduct(sideVector);
        Point3<T> ftl = frustumPoints[NTL].translate(sideVector * farDistOnSideVector);

        //top right point
        sideVector = frustumPoints[NTR].vector(frustumPoints[FTR]).normalize();
        Point3<T> ftr = frustumPoints[NTR].translate(sideVector * farDistOnSideVector);

        //bottom left point
        sideVector = frustumPoints[NBL].vector(frustumPoints[FBL]).normalize();
        Point3<T> fbl = frustumPoints[NBL].translate(sideVector * farDistOnSideVector);

        //bottom right point
        sideVector = frustumPoints[NBR].vector(frustumPoints[FBR]).normalize();
        Point3<T> fbr = frustumPoints[NBR].translate(sideVector * farDistOnSideVector);

        return Frustum<T>(frustumPoints[NTL], frustumPoints[NTR], frustumPoints[NBL], frustumPoints[NBR], ftl, ftr, fbl, fbr);
    }

    template<class T> bool Frustum<T>::collideWithPoint(const Point3<T>& point) const {
        return std::ranges::all_of(planes, [&point](const auto& plane){return plane.distance(point) <= 0.0;});
    }

    /**
    * @return True if the bounding box collides or is inside this frustum
    */
    template<class T> bool Frustum<T>::collideWithAABBox(const AABBox<T>& bbox) const {
        for (auto& plane : planes) {
            const Vector3<T>& normal = plane.getNormal();

            Point3<T> nVertex(bbox.getMax());
            if (normal.X >= 0.0) {
                nVertex.X = bbox.getMin().X;
            }
            if (normal.Y >= 0.0) {
                nVertex.Y = bbox.getMin().Y;
            }
            if (normal.Z >= 0.0) {
                nVertex.Z = bbox.getMin().Z;
            }

            if (plane.distance(nVertex) > 0.0) {
                return false;
            }
        }

        return true;
    }

    /**
    * @return True if the sphere collides or is inside this frustum
    */
    template<class T> bool Frustum<T>::collideWithSphere(const Sphere<T>& sphere) const {
        return std::ranges::all_of(planes, [&sphere](const auto& plane){return plane.distance(sphere.getCenterOfMass()) <= sphere.getRadius();});
    }

    template<class T> Frustum<T> operator *(const Matrix4<T>& m, const Frustum<T>& frustum) {
        Point4<T> ntl = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::NTL));
        Point4<T> ntr = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::NTR));
        Point4<T> nbl = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::NBL));
        Point4<T> nbr = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::NBR));
        Point4<T> ftl = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::FTL));
        Point4<T> ftr = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::FTR));
        Point4<T> fbl = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::FBL));
        Point4<T> fbr = m * Point4<T>(frustum.getFrustumPoint(Frustum<T>::FBR));

        return Frustum<T>(ntl.toPoint3(), ntr.toPoint3(), nbl.toPoint3(), nbr.toPoint3(),
            ftl.toPoint3(), ftr.toPoint3(), fbl.toPoint3(), fbr.toPoint3());
    }

    template<class T> Frustum<T> operator *(const Frustum<T>& frustum, const Matrix4<T>& m) {
        return m * frustum;
    }

    template<class T> std::ostream& operator <<(std::ostream& stream, const Frustum<T>& frustum) {
        stream.setf(std::ios::left);
        stream << "Frustum point NTL: " << frustum.getFrustumPoint(Frustum<T>::NTL) << std::endl;
        stream << "Frustum point NTR: " << frustum.getFrustumPoint(Frustum<T>::NTR) << std::endl;
        stream << "Frustum point NBL: " << frustum.getFrustumPoint(Frustum<T>::NBL) << std::endl;
        stream << "Frustum point NBR: " << frustum.getFrustumPoint(Frustum<T>::NBR) << std::endl;
        stream << "Frustum point FTL: " << frustum.getFrustumPoint(Frustum<T>::FTL) << std::endl;
        stream << "Frustum point FTR: " << frustum.getFrustumPoint(Frustum<T>::FTR) << std::endl;
        stream << "Frustum point FBL: " << frustum.getFrustumPoint(Frustum<T>::FBL) << std::endl;
        stream << "Frustum point FBR: " << frustum.getFrustumPoint(Frustum<T>::FBR);

        return stream;
    }

    //explicit template
    template class Frustum<float>;
    template Frustum<float> operator *<float>(const Matrix4<float>&, const Frustum<float>&);
    template Frustum<float> operator *<float>(const Frustum<float>&, const Matrix4<float>&);
    template std::ostream& operator <<<float>(std::ostream&, const Frustum<float>&);

    template class Frustum<double>;
    template Frustum<double> operator *<double>(const Matrix4<double>&, const Frustum<double>&);
    template Frustum<double> operator *<double>(const Frustum<double>&, const Matrix4<double>&);
    template std::ostream& operator <<<double>(std::ostream&, const Frustum<double>&);

}

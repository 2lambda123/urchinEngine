#include <limits>
#include <cmath>
#include <cassert>
#include <typeinfo>

#include "math/geometry/2d/object/LineSegment2D.h"
#include "math/algorithm/MathAlgorithm.h"

namespace urchin
{

	template<class T> LineSegment2D<T>::LineSegment2D(const Point2<T> &a, const Point2<T> &b) :
		a(a), b(b)
	{

	}

	template<class T> const Point2<T> &LineSegment2D<T>::getA() const
	{
		return a;
	}

	template<class T> const Point2<T> &LineSegment2D<T>::getB() const
	{
		return b;
	}

	template<class T> Point2<T> LineSegment2D<T>::getSupportPoint(const Vector2<T> &direction) const
	{
		const T pointADotDirection = Point2<T>(0.0, 0.0).vector(a).dotProduct(direction);
		const T pointBDotDirection = Point2<T>(0.0, 0.0).vector(b).dotProduct(direction);

		if(pointADotDirection > pointBDotDirection)
		{
			return a;
		}

		return b;
	}

	/**
	 * @param barycentrics [out] Returns barycentric coordinates for closest point
	 * @return Point on segment AB closest to point p
	 */
	template<class T> Point2<T> LineSegment2D<T>::closestPoint(const Point2<T> &p, T barycentrics[2]) const
	{
		Vector2<T> ab = a.vector(b);

		T abSquareLength = ab.dotProduct(ab);
		if(abSquareLength==(T)0.0)
		{
			barycentrics[0] = 1.0;
			barycentrics[1] = 0.0;
			return a;
		}

		Vector2<T> ap = a.vector(p);
		T t = ap.dotProduct(ab) / abSquareLength;
		t = std::min((T)1.0, std::max((T)0.0, t));

		barycentrics[0] = 1.0-t;
		barycentrics[1] = t;
		return ((b-a)*t) + a;
	}

	/**
	 * @return Minimum square distance between segment AB and point p
	 */
	template<class T> T LineSegment2D<T>::squareDistance(const Point2<T> &p) const
	{
		Vector2<T> ab = a.vector(b);
		Vector2<T> ap = a.vector(p);

		T e = ap.dotProduct(ab);
		if (e <= 0.0f)
		{
			return ap.dotProduct(ap);
		}

		T f = ab.dotProduct(ab);
		if (e >= f)
		{
			Vector2<T> bp = b.vector(p);
			return bp.dotProduct(bp);
		}

        if(typeid(int)==typeid(T) || typeid(long)==typeid(T) || typeid(long long)==typeid(T))
        {
            return ap.dotProduct(ap) - MathAlgorithm::roundDivision<T>(e * e, f);
        }
		return ap.dotProduct(ap) - ((e * e) / f);
	}

    template<class T> bool LineSegment2D<T>::onSegment(const Point2<T> &p) const
    {
        T orientation = ((p.Y - a.Y) * (b.X - p.X) - (p.X - a.X) * (b.Y - p.Y));
        if(orientation==(T)0)
        {
            return p.X <= std::max(a.X, b.X) && p.X >= std::min(a.X, b.X)
                   && p.Y <= std::max(a.Y, b.Y) && p.Y >= std::min(a.Y, b.Y);
        }

        return false;
    }

	template<class T> Vector2<T> LineSegment2D<T>::toVector() const
	{
		return a.vector(b);
	}

	/**
	 * Returns the intersection point of the two lines segment.
	 * When line segments are collinear and intersect: returns the nearest intersection point between this->getA() and this->getB().
	 */
	template<class T> Point2<T> LineSegment2D<T>::intersectPoint(const LineSegment2D<T> &other, bool &hasIntersection) const
	{
        Point2<T> farthestIntersection;
        bool hasFarthestIntersection;
        return intersectPoint(other, hasIntersection, farthestIntersection, hasFarthestIntersection);
	}

    /**
     * Returns the intersection point of the two lines segment.
     * When line segments are collinear and intersect: returns the nearest intersection point between this->getA() and this->getB().
     * The farthest intersection point is returned in 'farthestIntersection' point.
     */
    template<class T> Point2<T> LineSegment2D<T>::intersectPoint(const LineSegment2D<T> &other, bool &hasIntersection,
                                                                 Point2<T> &farthestIntersection, bool &hasFarthestIntersection) const
    { //see http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
        hasFarthestIntersection = false;

        Vector2<T> r(b.X - a.X, b.Y - a.Y); //note: a+1.0*r = b;
        Vector2<T> s(other.getB().X - other.getA().X, other.getB().Y - other.getA().Y);

        T rCrossS = r.crossProduct(s);
        Vector2<T> thisToOther = a.vector(other.getA());
        T startPointsCrossR = thisToOther.crossProduct(r);

        if(rCrossS==0)
        { //line segments are parallel
            double rDotR = static_cast<double>(r.dotProduct(r));
            if(startPointsCrossR==0 && rDotR!=0.0)
            { //line segments are collinear
                double t0 = thisToOther.dotProduct(r) / rDotR;
                double t1 = t0 + s.dotProduct(r) / rDotR;

                if(s.dotProduct(r) < 0.0)
                { //lines in opposite direction
                    std::swap(t0, t1);
                }

                #ifdef _DEBUG
                    assert(t0 <= t1);
                #endif

                if(t0>=0.0 && t0<=1.0 && t1>=1.0)
                { //collinear with intersection
                    hasIntersection = true;
                    hasFarthestIntersection = true;
                    farthestIntersection = b;
                    return a.translate(T(t0)*r);
                }else if(t0<=0.0 && t1>=0.0 && t1<=1.0)
                { //collinear with intersection
                    hasIntersection = true;
                    hasFarthestIntersection = true;
                    farthestIntersection = a.translate(T(t1)*r);
                    return a;
                }else if(t0>=0.0 && t0<=1.0 && t1>=0.0 && t1<=1.0)
                { //collinear intersection (other is totally covered by this)
                    hasIntersection = true;
                    hasFarthestIntersection = true;
                    farthestIntersection = a.translate(T(t1)*r);
                    return a.translate(T(t0)*r);
                }
            }

            hasIntersection = false;
            return Point2<T>(0, 0);
        }

        //line segments not parallel
        T thisToOtherCrossR = thisToOther.crossProduct(s);
        if( //if(t>=0 && t<=1 && u>=0 && u<=1) where t=thisToOtherCrossR/rCrossS and u=startPointsCrossR/rCrossS
                (thisToOtherCrossR==T(0) || MathAlgorithm::sign<T>(thisToOtherCrossR)==MathAlgorithm::sign<T>(rCrossS))
                && std::abs(rCrossS) >= std::abs(thisToOtherCrossR)
                && (startPointsCrossR==T(0) || MathAlgorithm::sign<T>(startPointsCrossR)==MathAlgorithm::sign<T>(rCrossS))
                && std::abs(rCrossS) >= std::abs(startPointsCrossR))
        { //intersection
            hasIntersection = true;
            if(typeid(int)==typeid(T) || typeid(long)==typeid(T) || typeid(long long)==typeid(T))
            {
                Vector2<T> vTranslate(MathAlgorithm::roundDivision<T>(thisToOtherCrossR*r.X, rCrossS),
                                      MathAlgorithm::roundDivision<T>(thisToOtherCrossR*r.Y, rCrossS));
                return a.translate(vTranslate);
            }
            return a.translate((thisToOtherCrossR*r)/rCrossS);
        }

        //no intersection
        hasIntersection = false;
        return Point2<T>(0, 0);
    }

	template<class T> template<class NEW_TYPE> LineSegment2D<NEW_TYPE> LineSegment2D<T>::cast() const
	{
		return LineSegment2D<NEW_TYPE>(a.cast<NEW_TYPE>(), b.cast<NEW_TYPE>());
	}

	template<class T> std::ostream& operator <<(std::ostream &stream, const LineSegment2D<T> &l)
	{
		return stream << l.getA().X << " " << l.getA().Y << " - " << l.getB().X << " " << l.getB().Y;
	}

	//explicit template
	template class LineSegment2D<float>;
	template LineSegment2D<float> LineSegment2D<float>::cast() const;
	template LineSegment2D<double> LineSegment2D<float>::cast() const;
	template LineSegment2D<int> LineSegment2D<float>::cast() const;
	template LineSegment2D<long long> LineSegment2D<float>::cast() const;
	template std::ostream& operator <<<float>(std::ostream & , const LineSegment2D<float> &);

	template class LineSegment2D<double>;
	template LineSegment2D<float> LineSegment2D<double>::cast() const;
	template LineSegment2D<double> LineSegment2D<double>::cast() const;
	template LineSegment2D<int> LineSegment2D<double>::cast() const;
	template LineSegment2D<long long> LineSegment2D<double>::cast() const;
	template std::ostream& operator <<<double>(std::ostream & , const LineSegment2D<double> &);

	template class LineSegment2D<int>;
	template LineSegment2D<float> LineSegment2D<int>::cast() const;
	template LineSegment2D<double> LineSegment2D<int>::cast() const;
	template LineSegment2D<int> LineSegment2D<int>::cast() const;
	template LineSegment2D<long long> LineSegment2D<int>::cast() const;
	template std::ostream& operator <<<int>(std::ostream & , const LineSegment2D<int> &);

	template class LineSegment2D<long long>;
	template LineSegment2D<float> LineSegment2D<long long>::cast() const;
	template LineSegment2D<double> LineSegment2D<long long>::cast() const;
	template LineSegment2D<int> LineSegment2D<long long>::cast() const;
	template LineSegment2D<long long> LineSegment2D<long long>::cast() const;
	template std::ostream& operator <<<long long>(std::ostream & , const LineSegment2D<long long> &);

}

/**
* \class CWall
*
* \brief   This class defines a wall as a set of vertex which has to be declared anticlockwise 
		 as seen from inside the room. Walls are the key component to compute images.
* \date    July 2021
* 
* \authors  Developer's team (University of Malaga), in alphabetical order: F. Arebola-Pérez, M. Cuevas-Rodríguez, D. Gonzalez-Toledo and A. Reyes-Lecuona ||
* Coordinated by A. Reyes-Lecuona (University of Malaga) ||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CWALL_HPP_
#define _CWALL_HPP_


#ifndef NUM_BAND_ABSORTION
	#define NUM_BAND_ABSORTION 9
#endif
#ifndef WALL_THRESHOLD
	#define WALL_THRESHOLD 0.00001f
#endif
#ifndef WALL_THRESHOLD_BORDER
	#define WALL_THRESHOLD_BORDER 0.3f
#endif
#ifndef TWOPI
		#define TWOPI 6.283185307179586476925287
#endif

#include <vector>
#include <Common/Vector3.hpp>

namespace Common {

class CWall {
public:
	
	CWall(): 
		A { 0 }
		, B { 0 }
		, C { 0 }
		, D { 0 }
		, absortionBands { std::vector<float> (NUM_BAND_ABSORTION, 0.0) }
		, active { true } {
		//Wall purely reflective by default
		//Wall active by default
	}

	/** \brief Insert a new corner (vertex) in the wall by a 3D vector
	*	\details Walls are defined as a series of corners (vertices), whcih should be declared in anticlockwise order as seen from inside the
				room. The wall is asumed to be a convex polygon and the last edge is defined between the last and the first declared vertices.
				When more then 3 corners are inserted, it is checked wether the new corner is in the same plane as the others or not. If not,
				it is projected to the plane to ensure that all the corners belong to the same plane.
	*	\param [in] Corner: vector containing the new corner to be inserted (expresed in m).
	*	\param [out] CORRECT: 1 if the new corner is in the same plane of the others. 0 if not and a projection to the plane is carried out.
	*/
	int InsertCorner(Common::CVector3 _corner) {
		return InsertCorner(_corner.x, _corner.y, _corner.z);
	}

	/** \brief Insert a new corner (vertex) in the wall by coordinates
	*	\details Walls are defined as a series of corners (vertices), which should be declared in anticlockwise order as seen from inside the
				room. The wall is asumed to be a convex polygon and the last edge is defined between the last and the first declared vertices.
				When more then 3 corners are inserted, it is checked wether the new corner is in the same plane as the others or not. If not,
				it is projected to the plane to ensure that all the corners belong to the same plane.
	*	\param [in] X coordinate of the corner to be inserted (m).
	*	\param [in] Y coordinate of the corner to be inserted (m).
	*	\param [in] Z coordinate of the corner to be inserted (m).
	*	\param [out] CORRECT: 1 if the new corner is in the same plane of the others. 0 if not and a projection to the plane is carried out.
	*/
	int InsertCorner(float _x, float _y, float _z) { 
		Common::CVector3 tempCorner(_x, _y, _z);
		if (polygon.size() < 3) {
			polygon.push_back(tempCorner);
			if (polygon.size() == 3) {
				Calculate_ABCD();
				return 1;
			} else {
				return 0;
			}
		} else {
			double diff = _x * A + _y * B + _z * C + D;
			diff = std::fabs(diff);
			if (diff < WALL_THRESHOLD) // ¿DBL_EPSILON? ¿THRESHOLD?
			{
				polygon.push_back(tempCorner);
				return 1;
			} else {
				tempCorner = GetPointProjection(_x, _y, _z);
				polygon.push_back(tempCorner);
				return 0;
			}
		}		
	}
	
	/** \brief Returns the corners of the wall
	*	\param [out] Corners: vector containing the set of corners of the wall in teh same order as they are defined.
	*/
	std::vector<Common::CVector3> GetCorners() {
		return polygon;
	}

	/** \brief set the absortion coeficient (frequency independent) of the wall
	*   \param [in] Absortion: absortion coeficient of the wall (expressed as a number between 0 (no absortion) and 1 (total absortion).
	*/
	bool SetAbsortion(float _absortion) {		
		if (_absortion < 0 || _absortion > 1) {
			return false;
		}
		absortionBands = std::vector<float> (NUM_BAND_ABSORTION, _absortion);
		return true;
	}

	/** \brief set the absortion coeficients for each band of the wall
	*	\param [in] Absortion: Vector with absortion coeficients of the wall (expressed as a number between 0 (no absortion) and 1 (total absortion).
	*/
	bool SetAbsortion(std::vector<float> _absortionPerBand) { 
		if (IsValidAbsortionsCoefficientsVector(_absortionPerBand)) {
			absortionBands = _absortionPerBand;
			return true;
		}
		return false;
	}
	
	/** \brief Returns the vector with absortion coeficients of the wall. 
	*	\param [out] Absortion: absortion of the wall. Vector with the absorption coefficients of each band.
	*/
	std::vector<float> GetAbsortionBand() {
		return absortionBands;
	}

	/** \brief Returns the normal vector to the wall. If the wall is properly defined, it points towards inside the room.
	*	\param [out] Normal: normal vector to the wall.
	*/
	Common::CVector3 GetNormal() {
		Common::CVector3 p1, p2, normal;
		float modulus;

		p1 = polygon.at(1) - polygon.at(0);
		p2 = polygon.at(2) - polygon.at(0);

		normal = p1.CrossProduct(p2);

		modulus = normal.GetDistance();

		normal.x = normal.x / modulus;
		normal.y = normal.y / modulus;
		normal.z = normal.z / modulus;

		return normal;
	}

	/** \brief Returns the center of the wall.
	*	\param [out] Center: central point of the wall.
	*/
	Common::CVector3 GetCenter() {
		Common::CVector3 center;
		
		center = Common::CVector3::ZERO();
		for (auto i = 0; i < polygon.size(); i++) {
			center.x += polygon.at(i).x;
			center.y += polygon.at(i).y;
			center.z += polygon.at(i).z;
		}
		center.x /= polygon.size();
		center.y /= polygon.size();
		center.z /= polygon.size();

		return center;
	}
	
	/** \brief Returns the distance of a given point to the wall's plane.
	*	\param [in] Point: point for whhich the distance to the wall's plane will be calculated (m).
	*	\param [out] Distance: shorterst distance to teh wall's plane (m).
	*/
	float GetDistanceFromPoint(Common::CVector3 point) {
		float distance;
		Calculate_ABCD();
		distance = fabs(A * point.x + B * point.y + C * point.z + D);
		distance = distance / sqrtf(A * A + B * B + C * C);
		return distance;
	}

	/** \brief Returns the minimum distance between two walls.
	*	\details Computes the distance between each corner of the given wall and each corner of this wall and returns the minimum of these distances
					This method is used to determine whether an image room can contain an image source closer than a given distance to any possible location
					in the original room. If the minimum distance from the wall in the original room which starts the branch of images and the image wall 
					which produces a new image room is greater than d, it is not possible to find a location in the new image room closar than d to any 
					location in the original room
	*	\param [in] wall: wall to compute the distance to this one.
	*	\param [out] Distance: shorterst distance to teh wall's plane (m).
	*/
	float GetMinimumDistanceFromWall(CWall wall) {
		Common::CVector3 cornerDistance = polygon.at(0) - wall.polygon.at(0);
		float minimumDistance = cornerDistance.GetDistance();
		for (int i = 0; i < polygon.size(); i++) {
			for (int j = 0; j < wall.polygon.size(); j++) {
				cornerDistance = polygon.at(i) - wall.polygon.at(j);
				if (minimumDistance > cornerDistance.GetDistance()) {
					minimumDistance = cornerDistance.GetDistance();
				}
			}
		}
		return minimumDistance;
	}

	/** \brief Returns the location of the image of a given point reflected in the wall's plane.
	*	\param [in] Point: original point for which the image reflected in the wall will be calculated.
	*	\param [out] Image: location of the image point.
	*/
	Common::CVector3 GetImagePoint(Common::CVector3 point) {
		float distance;
		Common::CVector3 imagePoint, normalRay;
		distance = GetDistanceFromPoint(point);

		normalRay = GetNormal();
		normalRay.x *= -(2 * distance);
		normalRay.y *= -(2 * distance);
		normalRay.z *= -(2 * distance);

		imagePoint = point + normalRay;

		return imagePoint;
	}

	/** \brief Returns an image wall of another given wall reflected in this wall's plane
	*	\param [in] Wall: original wall.
	*	\param [out] ImageWall: image wall, result of reflection of the original wall.
	*/
	CWall GetImageWall(CWall _wall) {
		CWall tempWall;
		std::vector<Common::CVector3> corners = _wall.GetCorners();
		for (int i = corners.size() - 1; i >= 0; i--) {
			Common::CVector3 tempImageCorner = GetImagePoint(corners.at(i));
			tempWall.InsertCorner(tempImageCorner);
		}
		tempWall.absortionBands = _wall.absortionBands;
		if (!_wall.IsActive()) tempWall.Disable();
		return tempWall;
	}

	/** \brief Returns the poin projected in the wall's plane of a given point.
	*	\param [in] X coordinate of the point to be projected.
	*	\param [in] Y coordinate of the point to be projected.
	*	\param [in] Z coordinate of the point to be projected.
	*	\param [out] Projection: porjected point in the woall's plane.
	*/
	Common::CVector3 GetPointProjection(float x0, float y0, float z0) { 
		// Vectorial Ec. of straight line --> (X,Y,Z) = (x0, y0, z0) + lambda (normalV.x, normalV.y, normalV.z)
		// Plane of the wall              --> AX+BY+CZ+D = 0
		Common::CVector3 normalV, point(x0, y0, z0);
		double rX1, rY1, rZ1, lambda;
		double rX2, rY2, rZ2;
		double diff1, diff2;
		float rX, rY, rZ;

		Calculate_ABCD();
		normalV = GetNormal();
		lambda = (double)GetDistanceFromPoint(point);

		// lambda could be positive or negative
		//
		rX1 = x0 + lambda * normalV.x;
		rY1 = y0 + lambda * normalV.y;
		rZ1 = z0 + lambda * normalV.z;
		diff1 = rX1 * A + rY1 * B + rZ1 * C + D;
		diff1 = std::fabs(diff1);

		rX2 = x0 - lambda * normalV.x;
		rY2 = y0 - lambda * normalV.y;
		rZ2 = z0 - lambda * normalV.z;
		diff2 = rX2 * A + rY2 * B + rZ2 * C + D;
		diff2 = std::fabs(diff2);

		if (diff1 < diff2) {
			rX = rX1;
			rY = rY1;
			rZ = rZ1;
		} else {
			rX = rX2;
			rY = rY2;
			rZ = rZ2;
		}

		return Common::CVector3(rX, rY, rZ);
	}

	/** \brief Returns the poin projected in the wall's plane of a given point.
	*	\param [in] Point: point to be projected.
	*	\param [out] Projection: porjected point in the woall's plane.
	*/
	Common::CVector3 GetPointProjection(Common::CVector3 point) {
		return GetPointProjection(point.x, point.y, point.z);
	}

	/** \brief Returns the poin where a given line intersects the wall's plane.
	*	\details Given a line defined with two points, this method computes its intersection qithg the wall's plane and returns
					that intersection point.
	*	\param [in] Point1: one of the points to define the line.
	*	\param [in] Point2: the other point to define the line.
	*	\param [out] Intersection: point of intersection of teh given line and the wall's plane.
	*/
	Common::CVector3 GetIntersectionPointWithLine(Common::CVector3 point1, Common::CVector3 point2) {
		Common::CVector3 cutPoint, vecLine;
		float modulus, lambda;

		vecLine = point2 - point1;

		lambda = (-D - (A * point1.x + B * point1.y + C * point1.z));
		lambda = lambda / (A * vecLine.x + B * vecLine.y + C * vecLine.z);

		cutPoint.x = point1.x + lambda * vecLine.x;
		cutPoint.y = point1.y + lambda * vecLine.y;
		cutPoint.z = point1.z + lambda * vecLine.z;

		modulus = GetDistanceFromPoint(cutPoint); // must be = ZERO

		return cutPoint;
	}

	/** \brief Checks wether a given point is inside the wall or not.
	*	\details Given a point, this method returns a positive value -that is the distance from the point to the closest edge of the wall-
				when the point is in the wall's plane and within the limits defined by the wall's corners. If the point is not int wall's plane
				or if the point is outside the polygon defined by the wall's corners, it returns a negative value.
	*	\param [in] Point: point to be checked.
	*	\param [out] Result: 
				0 --> Point is not in the wall's plane or distanceToBorder > THRESHOLD_BORDER
		        1 --> Point is inside the wall 
						visibility (sharpness) = 1.0 if distanceToBorder > THRESHOLD_BORDER
						visibility (sharpness) is between [1.0 ,  0.5) if distanceToBorder < THRESHOLD_BORDER
				2 --> Point is coming out of the wall
						visibility (sharpness) is between (0.5 ,  0] if distanceToBorder <= THRESHOLD_BORDER
	*/
	int CheckPointInsideWall(Common::CVector3 point, float & distanceNearestEdge, float & sharpness) {
		float modulus = GetDistanceFromPoint(point);
		if (modulus > 5 * WALL_THRESHOLD) {
			sharpness = 0.0;
			return 0; // Point is not in the wall's plane
		}

		double m1, m2, anglesum = 0, costheta, anglediff;
		Common::CVector3 p1, p2;
		int n = polygon.size();

		for (auto i = 0; i < n; i++) {
			p1.x = polygon[i].x - point.x;
			p1.y = polygon[i].y - point.y;
			p1.z = polygon[i].z - point.z;
			p2.x = polygon[(i + 1) % n].x - point.x;
			p2.y = polygon[(i + 1) % n].y - point.y;
			p2.z = polygon[(i + 1) % n].z - point.z;
			m1 = p1.GetDistance();
			m2 = p2.GetDistance();
			if (m1 * m2 <= WALL_THRESHOLD) {
				distanceNearestEdge = 0.0f;
				sharpness = 0.5f;
				return 1; // Point is on a corner of the wall,
			} else
				costheta = (p1.x * p2.x + p1.y * p2.y + p1.z * p2.z) / (m1 * m2);

			anglesum += std::acos(costheta);
		}

		anglediff = std::fabs(TWOPI - anglesum);
		if (anglediff < WALL_THRESHOLD) { // Point is inside Wall
			distanceNearestEdge = CalculateDistanceNearestEdge(point);
			if (fabs(distanceNearestEdge) < WALL_THRESHOLD_BORDER)
				sharpness = 0.5 + distanceNearestEdge / (2.0 * WALL_THRESHOLD_BORDER);
			else
				sharpness = 1.0;
			return 1; // Point is inside the wall,
		} else { // Point is outside Wall
			distanceNearestEdge = -CalculateDistanceNearestEdge(point);
			if (fabs(distanceNearestEdge) < WALL_THRESHOLD_BORDER) {
				sharpness = 0.5 + distanceNearestEdge / (2.0 * WALL_THRESHOLD_BORDER);
				return 2; // Point is coming out of the wall
			} else {
				sharpness = 0.0;
				return 0; // Point is outside Wall
			}
		}
	}

	/** \brief Returns the distance to the nearest edge of the wall.
	*	\details
	*	\param [in] Point: point to be checked.
	*	\param [out] Result: distance to the nearest edge.
	*/
	float CalculateDistanceNearestEdge(Common::CVector3 point) {
		float minDistance = 0.0, distance = 0.0;
		int n = polygon.size();
		for (auto i = 0; i < n; i++) {
			distance = DistancePointToLine(point, polygon[i], polygon[(i + 1) % n]);
			if (i == 0)
				minDistance = distance;
			else {
				if (distance < minDistance) minDistance = distance;
			}
		}
		return (minDistance);
	}

	/** \brief Returns the distance between a 3D point and a line in a 3D plane.
	*	\details
	*	\param [in] Point: 3D point, 3D point_1 of line,  3D point_2 of line.
	*	\param [out] Result: distance to the nearest edge.
	*/
	float DistancePointToLine(Common::CVector3 point, Common::CVector3 pointLine1, Common::CVector3 pointLine2) {
		float distance = 0, vectorModulus;
		Common::CVector3 vector1, vector2, vector3;
		vector1 = pointLine2 - pointLine1;
		vector2 = point - pointLine1;
		vector3 = vector1.CrossProduct(vector2);
		//vectorModulus = vector3.GetDistance();
		distance = vector3.GetDistance() / vector1.GetDistance();
		return distance;
	}

	/** \brief Enable the wall.
	*	\details Every wall can be active (it reflects) or not (i does not reflect anything, so it is as it does not exist.
					This methof sets the wall as active.
	*/
	void Enable() { active = true; }

	/** \brief Disable the wall.
	*	\details Every wall can be active (it reflects) or not (i does not reflect anything, so it is as it does not exist.
				This method sets the wall as incative.
	*/
	void Disable() { active = false; }

	/** \brief Returns wether the wall is active or not.
	*	\details Every wall can be active (it reflects) or not (i does not reflect anything, so it is as it does not exist.
				This method returs wether the wall is active or not.
	*/
	bool IsActive() { return active; }

private:
	////////////
	// Methods
	////////////

	/** \brief calculates the general (cartesian) equation of the plane containing the wall. Parameters are stored in private attributes
		*/
	void Calculate_ABCD() {
		Common::CVector3 normal;
		normal = GetNormal();
		A = normal.x;
		B = normal.y;
		C = normal.z;
		D = -(A * polygon.at(2).x + B * polygon.at(2).y + C * polygon.at(2).z);
	}
	
	bool IsValidAbsortionsCoefficientsVector(const std::vector<float>& _absortionPerBand) {		
		if (_absortionPerBand.size() != NUM_BAND_ABSORTION) return false;

		for (int i = 0; i < _absortionPerBand.size(); i++) {
			if (_absortionPerBand.at(i) < 0 || _absortionPerBand.at(i) > 1) {
				return false;
				break;
			}
		}
		return true;
	}

	///////////////
	// Attributes
	///////////////
	std::vector<Common::CVector3> polygon;	// corners of the wall
	std::vector<float> absortionBands;		// absortion coeficients (absorved energy / incident energy) for each octave Band
	bool active;							//sets wether the wall is active or not (if false, the wall is transparent)

	float A, B, C, D; // General Plane Eq.: Ax + By + Cz + D = 0
};
}
#endif
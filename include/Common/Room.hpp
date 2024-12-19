/**
* \class CRoom
*
* \brief   This class allows you to define a room, wall to wall. It is basically a wall container. 
*		   It also allows to define a room, in the shape of a shoebox, in a quick way. 
* \date    July 2021
* 
* \authors  Developer's team (University of Malaga), in alphabetical order: F. Arebola-Pérez, M. Cuevas-Rodríguez, D. Gonzalez-Toledo and A. Reyes-Lecuona ||
* Coordinated by A. Reyes-Lecuona (University of Malaga) ||
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

#ifndef _CROOM_HPP_
#define _CROOM_HPP_

#include <vector>
#include <Common/Vector3.hpp>
#include <Common/Wall.hpp>

namespace Common { 
	
	struct TRoomGeometry {
		
		std::vector<CVector3> corners;
		std::vector<std::vector<int>> walls;
	};

	class CRoom {
	public:
		CRoom()
			: shoeBox { false } { }

		/** \brief Initializes the object with a shoebox room
		*	\details creates six walls conforming a shoebox room with 0,0,0 at the center. It must be used right after
		*			 creating the empty object.
		*	\param [in] length: extension of the room along the X axis.
		*	\param [in] width: extension of the room along the Y axis.
		*	\param [in] height: extension of the room along the Z axis
		*/
		bool SetupShoeBox(float _length, float _width, float _height) {

			if (_length <= 0 || _width <= 0 || _height <= 0) return false;

			//If the room was previously set up as a shoebox, it will keep the wall properties if it is redifined with a new shoeboxSetup
			std::vector<CWall> previousWalls;
			if (shoeBox) {
				previousWalls = walls;
			}

			// Now we can clear the walls in case there was a previous definition to set the room up from scratch
			walls.clear();
			CWall front, back, left, right, ceiling, floor;
			front.InsertCorner(_length / 2, _width / 2, -_height / 2);
			front.InsertCorner(_length / 2, -_width / 2, -_height / 2);
			front.InsertCorner(_length / 2, _width / 2, _height / 2);
			front.InsertCorner(_length / 2, -_width / 2, _height / 2);
			InsertWall(front);
			left.InsertCorner(-_length / 2, _width / 2, _height / 2);
			left.InsertCorner(-_length / 2, _width / 2, -_height / 2);
			left.InsertCorner(_length / 2, _width / 2, -_height / 2);
			left.InsertCorner(_length / 2, _width / 2, _height / 2);
			InsertWall(left);
			right.InsertCorner(_length / 2, -_width / 2, _height / 2);
			right.InsertCorner(_length / 2, -_width / 2, -_height / 2);
			right.InsertCorner(-_length / 2, -_width / 2, -_height / 2);
			right.InsertCorner(-_length / 2, -_width / 2, _height / 2);
			InsertWall(right);
			back.InsertCorner(-_length / 2, -_width / 2, _height / 2);
			back.InsertCorner(-_length / 2, -_width / 2, -_height / 2);
			back.InsertCorner(-_length / 2, _width / 2, -_height / 2);
			back.InsertCorner(-_length / 2, _width / 2, _height / 2);
			InsertWall(back);
			floor.InsertCorner(_length / 2, _width / 2, -_height / 2);
			floor.InsertCorner(-_length / 2, _width / 2, -_height / 2);
			floor.InsertCorner(-_length / 2, -_width / 2, -_height / 2);
			floor.InsertCorner(_length / 2, -_width / 2, -_height / 2);
			InsertWall(floor);
			ceiling.InsertCorner(_length / 2, -_width / 2, _height / 2);
			ceiling.InsertCorner(-_length / 2, -_width / 2, _height / 2);
			ceiling.InsertCorner(-_length / 2, _width / 2, _height / 2);
			ceiling.InsertCorner(_length / 2, _width / 2, _height / 2);
			InsertWall(ceiling);

			if (shoeBox) {
				for (int i = 0; i < previousWalls.size(); i++) {
					if (!previousWalls.at(i).IsActive()) {
						walls.at(i).Disable();
					}
				}
			}
			shoeBox = true;
			shoeBoxLength = _length;
			shoeBoxWidth = _width;
			shoeBoxHeight = _height;
			
			return true;
		}

		/** \brief Initializes the object with a arbitrary geometry
		*	\details creates a room with arbitrary geometry by means of defining all its corners and the walls as polygons with those corners
		*	\param [in] roomGeometry: struct containing all the vertices and walls
		*/
		void SetupRoomGeometry(TRoomGeometry roomGeometry) {
			walls.clear();
			for (int i = 0; i < roomGeometry.walls.size(); i++) {
				CWall tempWall;
				for (int j = 0; j < roomGeometry.walls.at(i).size(); j++) {
					tempWall.InsertCorner(roomGeometry.corners.at(roomGeometry.walls.at(i).at(j)));
				}
				InsertWall(tempWall);
			}
			shoeBox = false;
		}

		/** \brief insert a new wall in the room
		*	\details Instead of using the setup method, this method can be used to create any arbitrary room. It should be
					 called once per wall to be inserted, after creating a new empty room.
		*	\param [in] Wall to be inserted.
		*/
		void InsertWall(CWall _newWall) {
			walls.push_back(_newWall);
		};

		/** \brief Returns a vector of walls containing all the walls of the room.
		*	\param [out] Walls: vector of walls with all the walls of the room.
		*/
		std::vector<CWall> GetWalls() {
			return walls;
		}

		/** \brief Makes one of the room's walls active
		*	\details Sets the i-th wall of the room as active and therefore reflective.
		*	\param [in] index of the wall to be active.
		*/
		void EnableWall(int wallIndex) {
			if (walls.size() > wallIndex) {
				walls.at(wallIndex).Enable();
			}
		}

		/** \brief Makes one of the room's walls transparent
		*	\details Sets the i-th wall of the room as not active and therefore transparent.
		*	\param [in] index of the wall to be active.
		*/
		void DisableWall(int wallIndex) {
			if (walls.size() > wallIndex) {
				walls.at(wallIndex).Disable();
			}
		}

		/** \brief sets the absortion coeficient (frequency independent) of one wall
		*	\details Sets the absortion coeficient (absorved energy / incident energy) of the 
		*            i-th wall of the room.
		*	\param [in] index of the wall.
		*	\param [in] absortion coeficient (frequency independent)
		*/
		bool SetWallAbsortion(int wallIndex, float absortion) {
			if (!IsThereThisWall(wallIndex)) return false;
			return walls.at(wallIndex).SetAbsortion(absortion);
		}

		/** \brief sets the absortion coeficient (frequency independent) of all walls
		*/
		bool SetAllWallsAbsortion(float _absortion) {
			bool control = true;
			int wallsNumber = walls.size();
			for (int i = 0; i < wallsNumber; i++) {
				control = control && walls.at(i).SetAbsortion(_absortion);
			}
			return control;
		}
		
		/** \brief Sets the absortion coeficient (frequency dependent) of one wall
		*	\details Overloads the previous one. Sets the absortion coeficient (absorved energy / incident energy) of 
		*            each of the nine bands for the i-th wall of the room.
		*	\param [in] index of the wall.
		*	\param [in] absortion coeficients for each band (frequency dependent)
		*/
		bool SetWallAbsortion(int wallIndex, std::vector<float> absortionPerBand) {
			if (!IsThereThisWall(wallIndex)) return false;
			return walls.at(wallIndex).SetAbsortion(absortionPerBand);
		};

		/** \brief Sets the absortion coeficient (frequency dependent) of all walls
		*/
		bool SetAllWallsAbsortion(std::vector<float> absortionPerBand) {
			bool control = true;
			int wallsNumber = walls.size();
			for (int i = 0; i < wallsNumber; i++) {
				control = control && walls.at(i).SetAbsortion(absortionPerBand);
			}
			return control;
		}

		

		/** \brief Returns a vector of image rooms
		*	\details creates an image (specular) room for each wall of this room and returns a vector contoining them.
		*	\param [out] ImageRooms: vector containing all image rooms of this room.
		*/
		std::vector<CRoom> GetImageRooms() {
			std::vector<CRoom> roomList;
			for (int i = 0; i < walls.size(); i++) {
				if (walls.at(i).IsActive()) {
					CRoom tempRoom;
					for (int j = 0; j < walls.size(); j++) {
						CWall tempWall = walls.at(i).GetImageWall(walls.at(j));
						tempRoom.InsertWall(tempWall);
					}
					roomList.push_back(tempRoom);
				}
			}
			return roomList;
		};

		/** \brief Checks wether a 3D point is inside the room or not.
		*	\details Returns the result of checking wether a 3D point is inside the room or not and the distance to teh nearest wall 
		*            which is positive if the point is inside the room and negative if it is outside. This method assumes that the room is convex
					 and that all the walls are properly defined declaring their corners clockwise as seen from inside the room.
		*	\param [in] point: 3D point to be checked.
		*	\param [out] distance to nearest wall passed by reference
		*	\param [out] Result: returned boolean indicating if the point is inside the room (true) or not (false)
		*/
		bool CheckPointInsideRoom(Common::CVector3 point, float & distanceNearestWall) {
			float distanceToPlane = FLT_MAX;
			bool inside;

			inside = true;
			for (int i = 0; i < walls.size(); i++) {
				if (walls.at(i).IsActive()) {

					Common::CVector3 normal, farthestCorner, center, p, p1, p2;
					center = walls.at(i).GetCenter();
					farthestCorner = center;
					normal = walls.at(i).GetNormal();

					std::vector<Common::CVector3> corners;
					CWall tWall = walls.at(i);
					corners = tWall.GetCorners();

					float tempDistanceToPlane = walls.at(i).GetDistanceFromPoint(point);
					if (tempDistanceToPlane < distanceToPlane) distanceToPlane = tempDistanceToPlane;

					double distance = 0.0, d;
					for (int j = 0; j < corners.size(); j++) {
						p.x = point.x - corners[j].x;
						p.y = point.y - corners[j].y;
						p.z = point.z - corners[j].z;
						d = p.GetDistance();
						if (d > distance) {
							distance = d;
							farthestCorner = corners[j];
						}
					}
					p = farthestCorner;
					//p = center;

					p1.x = p.x - point.x;
					p1.y = p.y - point.y;
					p1.z = p.z - point.z;

					p2.x = -normal.x;
					p2.y = -normal.y;
					p2.z = -normal.z;

					float dP;
					dP = p2.DotProduct(p1);
					if (dP < 0.0f)
						inside = false;
				}
			}
			distanceNearestWall = distanceToPlane;
			return inside;
		}

		/** \brief Returns the center of the room.
		*	\details The center is calculated as the average of the centers of the walls
		*	\param [out] center: point (CVector3) which is the center of the room.
		*/
		Common::CVector3 GetCenter() {
			Common::CVector3 center = Common::CVector3::ZERO();

			if (walls.size() > 0) {
				for (auto i = 0; i < walls.size(); i++) {
					center = center + walls.at(i).GetCenter();
				}
				center.x /= walls.size();
				center.y /= walls.size();
				center.z /= walls.size();
			}
			return center;
		}

		/** \brief Gets the shoebox room size
			Returns if it is a shoebox room it returns the size of the room, otherwise it returns 0,0,0
		*/
		Common::CVector3 GetShoeBoxRoomSize() {
			if (!shoeBox) return Common::CVector3::ZERO();
			return Common::CVector3(shoeBoxLength, shoeBoxWidth, shoeBoxHeight);
		}

		/** \brief Returns if there is any wall
		*/
		bool IsAnyWallDefined() {
			return walls.size() > 0;
		}

	private:

		bool IsThereThisWall(int _wallIndex) {
			return walls.size() > _wallIndex;
		}

		////////////
		// Attributes
		////////////

		bool shoeBox;			// Flag indicating if the room was set up as a shoebox
		float shoeBoxLength;	// Length of the shoebox room
		float shoeBoxWidth;		// Width of the shoebox room
		float shoeBoxHeight;	// Height of the shoebox room

		std::vector<CWall> walls; //Vector with all the walls of the room
	};
}
#endif
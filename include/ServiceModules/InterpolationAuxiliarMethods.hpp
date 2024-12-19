/**
* \class InterpolationAuxiliarMethods
*
* \brief Declaration of InterpolationAuxiliarMethods classes interface
* \date	July 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
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


#ifndef _CINTERPOLATION_AUXILIARMETHODS_HPP
#define _CINTERPOLATION_AUXILIARMETHODS_HPP


#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/GlobalParameters.hpp>
#include <ServiceModules/ServicesBase.hpp>


namespace BRTServices
{

#ifndef PI 
#define PI 3.14159265
#endif

	/** \brief Type definition for a distance-orientation pair
*/
	typedef std::pair <float, orientation> T_PairDistanceOrientation;

	/**	\brief Type definition for barycentric coordinates
*/
	struct TBarycentricCoordinatesStruct {
		float alpha;	///< Coordinate alpha
		float beta;		///< Coordinate beta
		float gamma;	///< Coordinate gamma
	};

	/** \brief Type definitions to indentify the poles
*/
	enum class TPole { north, south };

	/**
	 * @brief Auxiliar methods to perform the Offline interpolation algorithms
	*/
	class CInterpolationAuxiliarMethods {
	public:

		/**
		 *	@brief Transform azimuth range to [0, 360]
		 *  @param [in] azimuth to be checked and transformed, just in case.
		 *  @return azimuth changed to the new range
		*/
		static double CalculateAzimuthIn0_360Range(double _azimuth) {
			if (_azimuth < 0) {
				_azimuth = std::fmod(_azimuth, (float)360) + 360;
			}
			else if (_azimuth >= 360) {
				_azimuth = std::fmod(_azimuth, (float)360);
			}
			else {
				//DO nothing
			}

			return _azimuth;
		}

		/**
		 * @brief Transform azimuth range to [-180, 180]
		 * @param _azimuth [in] azimuth to be checked and transformed, just in case.
		 * @return azimuth changed to the new range
		*/
		static double CalculateAzimuthIn180Range(double _azimuth) {

			if (_azimuth < -180) {
				_azimuth = std::fmod(_azimuth, (float)180) + 180;
			}
			else if (_azimuth >= 180) {
				_azimuth = std::fmod(_azimuth, (float)180) - 180;
			}

			else {
				//DO nothing
			}

			return _azimuth;
		}


		/**
		 *	@brief ransform elevation range from [-90, 90] to the ([0,90] U [270, 360])
		 *  @param [in] elevation to be checked and transformed, just in case.
		 *	@return azimuth changed to the new range
		*/
		static double CalculateElevationIn0_90_270_360Range(double _elevation) {
			if (_elevation >= -90 && _elevation < 0) {
				_elevation += 360;
			}
			else if (_elevation == 360) {
				_elevation = 0;
			}
			return _elevation;
		}
		static float CalculateElevationIn0_90_270_360Range(float _elevation)
		{
			/*if (elevation < 0) { elevation = elevation + 360; }
			if (elevation >= 360) { elevation = elevation - 360; }
			return elevation;*/
			if (_elevation >= -90 && _elevation < 0) {
				_elevation += 360;
			}
			else if (_elevation == 360) {
				_elevation = 0;
			}
			return _elevation;
		}

		/**
		 *	@brief ransform elevation range from [-90, 90] to the ([0,90] U [270, 360])
		 *  @param [in] elevation to be checked and transformed, just in case.
		 *	@return azimuth changed to the new range
		*/
		static double CalculateElevationIn90Range(double _elevation) {
			if (_elevation >= 270) {
				_elevation -= 360;
			}
			return _elevation;
		}

		/**
		*	@brief Get Pole Elevation
		*	@param [in] Tpole var that indicates of which pole we need elevation
		*	@return azimuth changed to the new range
		*	@eh  On error, an error code is reported to the error handler.
		*/
		static int GetPoleElevation(TPole _pole)
		{
			if (_pole == TPole::north) { return ELEVATION_NORTH_POLE; }
			else if (_pole == TPole::south) { return ELEVATION_SOUTH_POLE; }
			else {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get a non-existent pole");
				return 0;
			}
		}

		/**
 * @brief Calculate the distance between two points [(azimuth1, elevation1) and (azimuth2, elevation2)] using the Haversine formula
 * @param azimuth1
 * @param elevation1
 * @param azimuth2
 * @param elevation2
 * @return float	the distance value
*/
		static float CalculateDistance_HaversineFormula(float azimuth1, float elevation1, float azimuth2, float elevation2)
		{
			float incremento_azimuth = azimuth1 - azimuth2;
			float incremento_elevacion = elevation1 - elevation2;
			float term1 = sin(incremento_elevacion / 2 * PI / 180.0);
			term1 = term1 * term1;
			float term2 = std::cos(elevation1 * PI / 180.0);
			float term3 = std::cos(elevation2 * PI / 180.0);
			float term4 = std::sin(incremento_azimuth / 2 * PI / 180.0);
			term4 = term4 * term4;
			float raiz = term1 + term2 * term3 * term4;

			ASSERT(raiz >= 0, RESULT_ERROR_OUTOFRANGE, "Attempt to compute square root of a negative value using Haversine Formula to compute distance", "");
			float sqrtDistance = std::sqrt(raiz);

			ASSERT(sqrtDistance >= -1.0f && sqrtDistance <= 1.0f, RESULT_ERROR_OUTOFRANGE,
				"Attempt to compute arcsin of a value outside [-1..1] using Harvesine Formula to compute distance",
				"");

			float distance = std::asin(std::sqrt(raiz));

			return distance;
		}

		/**
		 * @brief Get Sort a list of orientations according to the distance to a point.
		 * @param listToSort List of orientations to be ordered
		 * @param _newAzimuth Reference point azimuth
		 * @param _newElevation Reference point elevation
		 * @return List of orientations ordered by distance to the point
		*/
		static std::vector<T_PairDistanceOrientation> GetListOrderedDistancesToPoint(const std::vector<orientation>& listToSort, double _pointAzimuth, double _pointElevation)
		{
			std::vector<T_PairDistanceOrientation> sortedList;
			sortedList.reserve(listToSort.size());

			// Get all the distance to the point
			for (auto it = listToSort.begin(); it != listToSort.end(); ++it)
			{
				float distance = CalculateDistance_HaversineFormula(_pointAzimuth, _pointElevation, it->azimuth, it->elevation);
				orientation _orientation(it->azimuth, it->elevation);
				T_PairDistanceOrientation temp(distance, _orientation);
				/*temp.first = distance;
				temp.second.azimuth = it->azimuth;
				temp.second.elevation = it->elevation;*/
				sortedList.push_back(temp);
			}

			if (sortedList.size() != 0) {
				std::sort(sortedList.begin(), sortedList.end(), [](const T_PairDistanceOrientation& a, const T_PairDistanceOrientation& b) { return a.first < b.first; });
			}
			else {
				SET_RESULT(RESULT_WARNING, "Orientation list sorted by distances is empty");
			}

			return sortedList;
		}
	
		/**
		 * @brief Calculate the barycentric coordinates of three vertex [(x1,y1), (x2,y2), (x3,y3)] and the orientation of interest (x,y)
		 * @param x
		 * @param y
		 * @param x1
		 * @param y1
		 * @param x2
		 * @param y2
		 * @param x3
		 * @param y3
		 * @return
		*/
		static const TBarycentricCoordinatesStruct GetBarycentricCoordinates(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3)
		{
			// Obtain Barycentric coordinates:
			TBarycentricCoordinatesStruct barycentricCoordinates;

			float denominator = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);

			//if (round(denominator) == 0) {	//if denominator=0 -> no triangle -> barycentric coordinates NO VALID 
			if (Common::AreSame(denominator, 0.0f, EPSILON_SEWING)){
				//SET_RESULT(RESULT_WARNING, "Barycentric coordinates can be computed only on triangles");
				barycentricCoordinates.alpha = -1;
				barycentricCoordinates.beta = -1;
				barycentricCoordinates.gamma = -1;
			}
			else {
				barycentricCoordinates.alpha = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
				barycentricCoordinates.alpha = trunc(1000 * barycentricCoordinates.alpha) / 1000;
				barycentricCoordinates.beta = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
				barycentricCoordinates.beta = trunc(1000 * barycentricCoordinates.beta) / 1000;
				barycentricCoordinates.gamma = 1.0f - barycentricCoordinates.alpha - barycentricCoordinates.beta;
				barycentricCoordinates.gamma = trunc(1000 * barycentricCoordinates.gamma) / 1000;
				//SET_RESULT(RESULT_OK, "Barycentric coordinates computed succesfully");
			}
			return barycentricCoordinates;
		}

		static void CalculateAzimuth_BackandFront(float& aziBack, float& aziFront, float aziStep, float _azimuth)
		{
			int idxAzi = ceil(_azimuth / aziStep);

			aziFront = idxAzi * aziStep;
			aziBack = (idxAzi - 1) * aziStep;


			aziBack = CheckLimitsAzimuth_and_Transform(aziBack);
		}

		static float CheckLimitsAzimuth_and_Transform(float azimuth)
		{
			if (azimuth < 0) { azimuth = azimuth + 360; }
			else if (azimuth >= 360) { azimuth = azimuth - 360; }
			return azimuth;
		}

	};
}
#endif
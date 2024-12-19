/**
* \class COfflineInterpolatorAuxiliarMethods, CDistanceBasedInterpolator, CQuadrantBasedInterpolator
*
* \brief Declaration of COfflineInterpolatorInterface and CDistanceBasedInterpolator classes interface
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


#ifndef _COFFLINE_INTERPOLATION_AUXILIARMETHODS_HPP
#define _COFFLINE_INTERPOLATION_AUXILIARMETHODS_HPP

#include <unordered_map>
#include <vector>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>

namespace BRTServices
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// COfflineInterpolationAuxiliarMethods Class
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief Auxiliar methods to perform the Offline interpolation algorithms
	*/
	class COfflineInterpolationAuxiliarMethods {
	public:
		/**
		 * @brief Transform the orientation in order to move the orientation of interest to 180 degrees
		 * @param azimuthOrientationOfInterest
		 * @param originalAzimuth
		 * @return transformed azimuth
		*/
		static float TransformAzimuthToAvoidSewing(double azimuthOrientationOfInterest, double originalAzimuth)
		{
			float azimuth;
			azimuth = originalAzimuth + 180 - azimuthOrientationOfInterest;
			// Check limits 
			if (azimuth > DEFAULT_MAX_AZIMUTH)
				azimuth = std::fmod(azimuth, (float)360);

			if (azimuth < DEFAULT_MIN_AZIMUTH)
				azimuth = azimuth + 360;

			return azimuth;
		}

		/**
		 * @brief Transform the orientation in order to express the elevation in the interval [-90,90]
		 * @param elevationOrientationOfInterest
		 * @param originalElevation
		 * @return transformed elevation
		*/
		static float TransformElevationToAvoidSewing(double elevationOrientationOfInterest, double originalElevation)
		{
			if (originalElevation >= ELEVATION_SOUTH_POLE) {
				originalElevation = originalElevation - 360;
			}
			return originalElevation;
		}
 	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CDistanceBased_OfflineInterpolator Class
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief Offline interpolation based on the search for the 3 closest points for each point to be interpolated.
	*/
	class CDistanceBased_OfflineInterpolator {
	public:

		/**
		 * @brief Offline calculation of the interpolated TF using the Distance-based method
		 * @tparam T Type of table where the TF (or IR) will be included
		 * @tparam U Type of data to be included in the table
		 * @tparam Functor Function that performs the interpolation
		 * @param _table Table where the newTF will be emplaced
		 * @param f_CalculateTF_Offline Function that performs the interpolation
		 * @param listToSort List will all orientation ordered
		 * @param _newAzimuth Azimuth of the point to be interpolated
		 * @param _newElevation Elevation of the point to be interpolated
		 * @param _TFLength Lenght of the TF (or IR) to be emplaced in the _table
		 * @param pole Indicates if the oritation is a pole or not
		 * @return 
		*/
		template <typename T, typename U, typename Functor>
		U CalculateHRIR_offlineMethod(const T& _table, Functor f_CalculateTF_Offline, std::vector<orientation>& listToSort, double _newAzimuth, double _newElevation, int _TFLength, int pole = 0)
		{
			U newTF;
			// Get a list sorted by distances to the orientation of interest
			std::vector<T_PairDistanceOrientation> sortedList = GetSortedDistancesList(listToSort, _newAzimuth, _newElevation);
			if (sortedList.size() == 0) {
				SET_RESULT(RESULT_ERROR_NOTSET, "Orientation List sorted by distances in GetHRIR_InterpolationMethod is empty");
				return newTF;
			}

			// Obtain  the valid Barycentric coordinates:
			TBarycentricCoordinatesStruct barycentricCoordinates;
			std::vector<orientation> mygroup(sortedList.size());
			auto it = sortedList.begin();
			for (int copy = 0; copy < sortedList.size(); copy++) {
				mygroup[copy] = it->second;
				it++;
			}
			//Algorithm to get a triangle around the orientation of interest
			for (int groupSize = 3; groupSize < sortedList.size(); groupSize++)
			{
				for (int i = 0; i < groupSize - 2; i++)
				{
					for (int j = i + 1; j < groupSize - 1; j++)
					{
						for (int k = j + 1; k < groupSize; k++)
						{
							if (pole == ELEVATION_SOUTH_POLE || pole == ELEVATION_NORTH_POLE)
							{
								mygroup[i].azimuth = _newAzimuth;
								mygroup[i].elevation = pole;
							}
							//Azimuth and elevation transformation in order to get the barientric coordinates (due to we are working with a spehere not a plane)
							float newAzimuthTransformed = COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth, _newAzimuth);
							float iAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth, mygroup[i].azimuth);
							float jAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth, mygroup[j].azimuth);
							float kAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth, mygroup[k].azimuth);
							float newElevationTransformed = COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation, _newElevation);
							float iElevationTransformed = COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation, mygroup[i].elevation);
							float jElevationTransformed = COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation, mygroup[j].elevation);
							float kElevationTransformed = COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation, mygroup[k].elevation);

							barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, iAzimuthTransformed, iElevationTransformed, jAzimuthTransformed, jElevationTransformed, kAzimuthTransformed, kElevationTransformed);
							//FIXME: It's not checking of the barycentric coordinates are bigger than zero, and sometimes are not.
							newTF = f_CalculateTF_Offline(_table, orientation(mygroup[i].azimuth, mygroup[i].elevation), orientation(mygroup[j].azimuth, mygroup[j].elevation), orientation(mygroup[k].azimuth, mygroup[k].elevation), _TFLength, barycentricCoordinates);
							
							return newTF;
						}
					}
				}
			}
			return newTF;
			//SET_RESULT(RESULT_OK, "");			
		}
		friend class CHRTFTester;


	private:
		std::vector<T_PairDistanceOrientation> GetSortedDistancesList(const std::vector<orientation>& listToSort, float newAzimuth, float newElevation)
		{

			T_PairDistanceOrientation temp;
			float distance;
			std::vector<T_PairDistanceOrientation> sortedList;
			sortedList.reserve(listToSort.size());

			// Algorithm to calculate the three shortest distances between the point (newAzimuth, newelevation) and all the points in the given list
			for (auto it = listToSort.begin(); it != listToSort.end(); ++it)
			{
				distance = CInterpolationAuxiliarMethods::CalculateDistance_HaversineFormula(newAzimuth, newElevation, it->azimuth, it->elevation);

				temp.first = distance;
				temp.second.azimuth = it->azimuth;
				temp.second.elevation = it->elevation;

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
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CQuadrantBased_OfflineInterpolator Class
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @brief Offline interpolation based on quadrant method
	*/
	class CQuadrantBased_OfflineInterpolator {
	public:

		/**
		 * @brief Offline calculation of the interpolated TF using the Quadrant-based method
		 * @tparam T Type of table where the TF (or IR) will be included
		 * @tparam W_TFStruct Type of table where the TF (or IR) will be included
		 * @tparam Functor Function that performs the interpolation
		 * @param _table Table where the newTF will be emplaced
		 * @param f_CalculateTF_Offline Function that performs the interpolation
		 * @param listToSort List will all orientation ordered
		 * @param _TFLength Lenght of the TF (or IR) to be emplaced in the _table
		 * @param _newAzimuth Lenght of the TF (or IR) to be emplaced in the _table
		 * @param _newElevation Elevation of the point to be interpolated
		 * @param pole Indicates if the oritation is a pole or not
		 * @return 
		*/
		template <typename T, typename W_TFStruct, typename Functor>
		W_TFStruct CalculateHRIR_offlineMethod(const T& _table, Functor f_CalculateTF_Offline, std::vector<orientation>& listToSort, int _TFLength, double _newAzimuth, double _newElevation, int pole = 0)
		{
			std::vector<orientation> azimuthBackList, azimuthFrontList, backCeilList, backFloorList, frontCeilList, frontFloorList;
			TBarycentricCoordinatesStruct barycentricCoordinates;

			//THRIRStruct emptyHRIR;
			W_TFStruct newTF;

			// Get 2 list sorted by azimuth to the orientation of interest, Back and Front
			SortListByAzimuthAndSplit(_newAzimuth, listToSort, azimuthBackList, azimuthFrontList);

			// Get 2 lists sorted by elevation to the orientation of interest, Back Ceil and Back Floor
			SortListByElevationAndSplit(_newElevation, azimuthBackList, backCeilList, backFloorList);

			// Get 2 lists sorted by elevation to the orientation of interest, Front Ceil and Front Floor
			SortListByElevationAndSplit(_newElevation, azimuthFrontList, frontCeilList, frontFloorList);


			// Now each list will be sort by distance to the orientation of interest
			std::vector<T_PairDistanceOrientation> backCeilListSortedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(backCeilList, _newAzimuth, _newElevation);
			std::vector<T_PairDistanceOrientation> backFloorListSortedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(backFloorList, _newAzimuth, _newElevation);
			std::vector<T_PairDistanceOrientation> frontCeilListSortedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(frontCeilList, _newAzimuth, _newElevation);
			std::vector<T_PairDistanceOrientation> frontFloorlListSortedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(frontFloorList, _newAzimuth, _newElevation);

			// Transform Azimuth and Elevation to avoid the sewing ////// First column is Distance, so Azimuth is 2nd and Elevation 3rd
			// Azimuth
			float newAzimuthTransformed			= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth, _newAzimuth);
			float backCeilAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth,	backCeilListSortedByDistance[0].second.azimuth);
			float backFloorAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth,	backFloorListSortedByDistance[0].second.azimuth);
			float frontCeilAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth,	frontCeilListSortedByDistance[0].second.azimuth);
			float frontFloorAzimuthTransformed	= COfflineInterpolationAuxiliarMethods::TransformAzimuthToAvoidSewing(_newAzimuth, frontFloorlListSortedByDistance[0].second.azimuth);
			// Elevation
			float newElevationTransformed		= COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation, _newElevation);
			float backCeilElevationTransformed	= COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation,		backCeilListSortedByDistance[0].second.elevation);
			float backFloorElevationTransformed	= COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation,	backFloorListSortedByDistance[0].second.elevation);
			float frontCeilElevationTransformed	= COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation,	frontCeilListSortedByDistance[0].second.elevation);
			float frontFloorElevationTransformed= COfflineInterpolationAuxiliarMethods::TransformElevationToAvoidSewing(_newElevation,	frontFloorlListSortedByDistance[0].second.elevation);

			// Calculate slopes to make the triangulation
			float slopeDiagonalTrapezoid = std::abs((frontFloorElevationTransformed - backCeilElevationTransformed) / (frontFloorAzimuthTransformed - backCeilAzimuthTransformed));
			float slopeOrientationOfInterest = std::abs((newElevationTransformed - backCeilElevationTransformed) / (newAzimuthTransformed - backCeilAzimuthTransformed));

			if (slopeOrientationOfInterest >= slopeDiagonalTrapezoid)
			{
				// Uses A,C,D
				barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, backCeilAzimuthTransformed,
					backCeilElevationTransformed, backFloorAzimuthTransformed, backFloorElevationTransformed, frontFloorAzimuthTransformed, frontFloorElevationTransformed);

				if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
				{
					newTF = f_CalculateTF_Offline(_table, orientation(backCeilListSortedByDistance[0].second.azimuth, backCeilListSortedByDistance[0].second.elevation), orientation(backFloorListSortedByDistance[0].second.azimuth, backFloorListSortedByDistance[0].second.elevation), orientation(frontFloorlListSortedByDistance[0].second.azimuth, frontFloorlListSortedByDistance[0].second.elevation), _TFLength, barycentricCoordinates);
					return newTF;
				}
				else
				{
					SET_RESULT(RESULT_ERROR_NOTSET, "Calculate HRIR OfflineMethod (QuadrantBased) return empty TF in position [" + std::to_string(_newAzimuth) + ", " + std::to_string(_newElevation) + "]");
					return newTF;
				}
			}
			else
			{
				//Uses A,B,D
				barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, backCeilAzimuthTransformed,
					backCeilElevationTransformed, frontCeilAzimuthTransformed, frontCeilElevationTransformed, frontFloorAzimuthTransformed, frontFloorElevationTransformed);

				if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
				{
					newTF = f_CalculateTF_Offline(_table, orientation(backCeilListSortedByDistance[0].second.azimuth, backCeilListSortedByDistance[0].second.elevation), orientation(frontCeilListSortedByDistance[0].second.azimuth, frontCeilListSortedByDistance[0].second.elevation), orientation(frontFloorlListSortedByDistance[0].second.azimuth, frontFloorlListSortedByDistance[0].second.elevation), _TFLength, barycentricCoordinates);
					return newTF;
				}
				else
				{
					SET_RESULT(RESULT_ERROR_NOTSET, "Calculate HRIR OfflineMethod (QuadrantBased) return empty TF in position [" + std::to_string(_newAzimuth) + ", " + std::to_string(_newElevation) + "]");
					return newTF;
				}
			}		
		}

	private:

		void SortListByAzimuthAndSplit(double _newAzimuth, std::vector<orientation>& listToSort, std::vector<orientation>& _azimuthBackList, std::vector<orientation>& _azimuthFrontList)
		{
			// Sort List By Azimuth
			if (listToSort.size() != 0) {
				std::sort(listToSort.begin(), listToSort.end(), [](const orientation& a, const orientation& b) { return a.azimuth < b.azimuth; });
			}
			else {
				SET_RESULT(RESULT_WARNING, "Orientation list sorted is empty");
			}

			// NEW SPLIT
			double azimuthDifference;

			for (auto& it : listToSort)
			{
				if (it.azimuth == 360)
				{
					//Do not clasify Az360 except in the following case:
					//If we are in Az180 we have to include in the clasification the Az360, in order to get a triangle with the barycentric coordinates
					if (_newAzimuth == 180) {
						_azimuthFrontList.push_back(it);
					}
				}
				else
				{
					azimuthDifference = it.azimuth - _newAzimuth;
					 if (azimuthDifference > 0 && azimuthDifference <= 180)
					{
						_azimuthFrontList.push_back(it);
					}
					else if (azimuthDifference < 0 && azimuthDifference >= -180)
					{
						_azimuthBackList.push_back(it);
					}
					else if (azimuthDifference > 0 && azimuthDifference > 180)
					{
						_azimuthBackList.push_back(it);
					}
					else
					{
						_azimuthFrontList.push_back(it);
					}
				}

			}
		}

		void SortListByElevationAndSplit(double _newElevation, std::vector<orientation>& listToSort, std::vector<orientation>& ceilList, std::vector<orientation>& floorList)
		{
			// Sort List By Elevation
			if (listToSort.size() != 0) {
				std::sort(listToSort.begin(), listToSort.end(), [](const orientation& a, const orientation& b) { return a.elevation < b.elevation; });
			}
			else {
				SET_RESULT(RESULT_WARNING, "Orientation list sorted by distances is empty");
			}

			double elevationFromListTransformed;
			for (auto& it : listToSort)
			{
				// Transform elevation to then range [-90, 90] in order to make the comparison
				elevationFromListTransformed = it.elevation;
				if (elevationFromListTransformed >= 270) { elevationFromListTransformed = it.elevation - 360; }
				if (_newElevation >= 270) { _newElevation = _newElevation - 360; }
				

				if (elevationFromListTransformed < _newElevation)
				{
					floorList.push_back(it);
				}
				else
				{
					ceilList.push_back(it);
				}
	
			}

		}
	};

}
#endif
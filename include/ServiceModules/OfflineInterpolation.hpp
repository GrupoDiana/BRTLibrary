/**
* \class COfflineInterpolatorInterface, CDistanceBasedInterpolator
*
* \brief Declaration of COfflineInterpolatorInterface and CDistanceBasedInterpolator classes interface
* \date	July 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga 2023. Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _COFFLINE_INTERPOLATION_HPP
#define _COFFLINE_INTERPOLATION_HPP

#include <unordered_map>
#include <vector>
#include <ServiceModules/HRTFDefinitions.hpp>


namespace BRTServices
{
	class COfflineInterpolatorInterface {
	public:
		//virtual std::vector<T_PairDistanceOrientation> GetSortedDistancesList(std::vector<orientation> listToSort, float newAzimuth, float newElevation) = 0;
		virtual THRIRStruct CalculateHRIR_offlineMethod(const T_HRTFTable& table, std::vector<orientation>& listToSort, float newAzimuth, float newElevation, int HRIRLength, int pole) = 0;
	};

	class CDistanceBasedInterpolator : COfflineInterpolatorInterface {
	public:

		THRIRStruct CalculateHRIR_offlineMethod(const T_HRTFTable& table, std::vector<orientation>& listToSort, float newAzimuth, float newElevation, int HRIRLength, int pole = 0)
		{
			THRIRStruct newHRIR;
			//// Get a list sorted by distances to the orientation of interest

			//std::list<T_PairDistanceOrientation> sortedList = GetSortedDistancesList(newAzimuth, newElevation);
			std::vector<T_PairDistanceOrientation> sortedList = GetSortedDistancesList(listToSort, newAzimuth, newElevation);

			// PASS THE SORTED LIST TO THE FUNCTION
			if (sortedList.size() == 0) {
				SET_RESULT(RESULT_ERROR_NOTSET, "Orientation List sorted by distances in GetHRIR_InterpolationMethod is empty");
				return newHRIR;
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
								mygroup[i].azimuth = newAzimuth;
								mygroup[i].elevation = pole;
							}

							//Azimuth and elevation transformation in order to get the barientric coordinates (due to we are working with a spehere not a plane)
							float newAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth, newAzimuth);
							float iAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth, mygroup[i].azimuth);
							float jAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth, mygroup[j].azimuth);
							float kAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth, mygroup[k].azimuth);
							float newElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation, newElevation);
							float iElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation, mygroup[i].elevation);
							float jElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation, mygroup[j].elevation);
							float kElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation, mygroup[k].elevation);

							barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, iAzimuthTransformed, iElevationTransformed, jAzimuthTransformed, jElevationTransformed, kAzimuthTransformed, kElevationTransformed);

							if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f) {
								// Calculate the new HRIR with the barycentric coorfinates
								auto it0 = table.find(orientation(mygroup[i].azimuth, mygroup[i].elevation));
								auto it1 = table.find(orientation(mygroup[j].azimuth, mygroup[j].elevation));
								auto it2 = table.find(orientation(mygroup[k].azimuth, mygroup[k].elevation));

								if (it0 != table.end() && it1 != table.end() && it2 != table.end()) {

									//FIXME!!! another way to initialize?
									newHRIR = it0->second;
									//END FIXME

									for (int i = 0; i < HRIRLength; i++) {
										newHRIR.leftHRIR[i] = barycentricCoordinates.alpha * it0->second.leftHRIR[i] + barycentricCoordinates.beta * it1->second.leftHRIR[i] + barycentricCoordinates.gamma * it2->second.leftHRIR[i];
										newHRIR.rightHRIR[i] = barycentricCoordinates.alpha * it0->second.rightHRIR[i] + barycentricCoordinates.beta * it1->second.rightHRIR[i] + barycentricCoordinates.gamma * it2->second.rightHRIR[i];
									}

									// Calculate delay
									newHRIR.leftDelay = barycentricCoordinates.alpha * it0->second.leftDelay + barycentricCoordinates.beta * it1->second.leftDelay + barycentricCoordinates.gamma * it2->second.leftDelay;
									newHRIR.rightDelay = barycentricCoordinates.alpha * it0->second.rightDelay + barycentricCoordinates.beta * it1->second.rightDelay + barycentricCoordinates.gamma * it2->second.rightDelay;
									//SET_RESULT(RESULT_OK, "HRIR calculated with interpolation method succesfully");
									return newHRIR;
								}
								else {
									SET_RESULT(RESULT_WARNING, "GetHRIR_InterpolationMethod return empty because HRIR with a specific orientation was not found");
									return newHRIR;
								}
							}
						}
					}
				}
			}
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
				distance = CHRTFAuxiliarMethods::CalculateDistance_HaversineFormula(newAzimuth, newElevation, it->azimuth, it->elevation);

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

	class CQuadrantBasedInterpolator : COfflineInterpolatorInterface {
	public:

		THRIRStruct CalculateHRIR_offlineMethod(const T_HRTFTable& table, std::vector<orientation>& listToSort, float newAzimuth, float newElevation, int HRIRLength, int pole = 0)
		{
			std::vector<orientation> azimuthBackList, azimuthFrontList, backCeilList, backFloorList, frontCeilList, frontFloorList;
			TBarycentricCoordinatesStruct barycentricCoordinates;

			THRIRStruct emptyHRIR;

			// Get 2 list sorted by azimuth to the orientation of interest, Back and Front
			SortListByAzimuthAndSplit(newAzimuth, listToSort, azimuthBackList, azimuthFrontList);

			// Get 2 lists sorted by elevation to the orientation of interest, Back Ceil and Back Floor
			SortListByElevationAndSplit(newElevation, azimuthBackList, backCeilList, backFloorList);

			// Get 2 lists sorted by elevation to the orientation of interest, Front Ceil and Front Floor
			SortListByElevationAndSplit(newElevation, azimuthFrontList, frontCeilList, frontFloorList);


			// Now each list will be sort by distance to the orientation of interest
			std::vector<T_PairDistanceOrientation> backCeilListSortedByDistance = GetSortedDistancesList(backCeilList, newAzimuth, newElevation);
			std::vector<T_PairDistanceOrientation> backFloorListSortedByDistance = GetSortedDistancesList(backFloorList, newAzimuth, newElevation);
			std::vector<T_PairDistanceOrientation> frontCeilListSortedByDistance = GetSortedDistancesList(frontCeilList, newAzimuth, newElevation);
			std::vector<T_PairDistanceOrientation> frontFloorlListSortedByDistance = GetSortedDistancesList(frontFloorList, newAzimuth, newElevation);

			// Transform Azimuth and Elevation to avoid the sewing ////// First column is Distance, so Azimuth is 2nd and Elevation 3rd

			// Azimuth

			float newAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth, newAzimuth);
			float backCeilAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth,	backCeilListSortedByDistance[0].second.azimuth);
			float backFloorAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth,	backFloorListSortedByDistance[0].second.azimuth);
			float frontCeilAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth,	frontCeilListSortedByDistance[0].second.azimuth);
			float frontFloorAzimuthTransformed = CHRTFAuxiliarMethods::TransformAzimuth(newAzimuth, frontFloorlListSortedByDistance[0].second.azimuth);

			// Elevation

			float newElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation, newElevation);
			float backCeilElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation,		backCeilListSortedByDistance[0].second.elevation);
			float backFloorElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation,	backFloorListSortedByDistance[0].second.elevation);
			float frontCeilElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation,	frontCeilListSortedByDistance[0].second.elevation);
			float frontFloorElevationTransformed = CHRTFAuxiliarMethods::TransformElevation(newElevation,	frontFloorlListSortedByDistance[0].second.elevation);


			float slopeDiagonalTrapezoid = std::abs(frontFloorElevationTransformed - backCeilElevationTransformed) / (frontFloorAzimuthTransformed - backCeilAzimuthTransformed);
			float slopeOrientationOfInterest = std::abs(newElevationTransformed - backCeilElevationTransformed) / (newAzimuthTransformed - backCeilAzimuthTransformed);

			if (slopeOrientationOfInterest >= slopeDiagonalTrapezoid)
			{
				// Uses A,C,D
				barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, backCeilAzimuthTransformed,
					backCeilElevationTransformed, backFloorAzimuthTransformed, backFloorElevationTransformed, frontFloorAzimuthTransformed, frontFloorElevationTransformed);

				if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
				{
					return DataInterpolation(table, barycentricCoordinates, HRIRLength, backCeilListSortedByDistance[0].second.azimuth, backCeilListSortedByDistance[0].second.elevation, 
						backFloorListSortedByDistance[0].second.azimuth, backFloorListSortedByDistance[0].second.elevation, frontFloorlListSortedByDistance[0].second.azimuth, frontFloorlListSortedByDistance[0].second.elevation);
				}
				else
				{
					return emptyHRIR;
				}
			}
			else
			{
				//Uses A,B,D
				barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(newAzimuthTransformed, newElevationTransformed, backCeilAzimuthTransformed,
					backCeilElevationTransformed, frontCeilAzimuthTransformed, frontCeilElevationTransformed, frontFloorAzimuthTransformed, frontFloorElevationTransformed);

				if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
				{
					return DataInterpolation(table, barycentricCoordinates, HRIRLength, backCeilListSortedByDistance[0].second.azimuth, backCeilListSortedByDistance[0].second.elevation,
						frontCeilListSortedByDistance[0].second.azimuth, frontCeilListSortedByDistance[0].second.elevation, frontFloorlListSortedByDistance[0].second.azimuth, frontFloorlListSortedByDistance[0].second.elevation);
				}
				else
				{
					return emptyHRIR;
				}
			}		
		}

	private:

		void SortListByAzimuthAndSplit(float _newAzimuth, std::vector<orientation>& listToSort, std::vector<orientation>& _azimuthBackList, std::vector<orientation>& _azimuthFrontList)
		{
			// Sort List By Azimuth
			if (listToSort.size() != 0) {
				std::sort(listToSort.begin(), listToSort.end(), [](const orientation& a, const orientation& b) { return a.azimuth < b.azimuth; });
			}
			else {
				SET_RESULT(RESULT_WARNING, "Orientation list sorted is empty");
			}

			// NEW SPLIT

			float resta;

			for (auto& it : listToSort)
			{
				resta = it.azimuth - _newAzimuth;
				if (resta > 0 && resta <= 180) 
				{
					_azimuthFrontList.push_back(it);
				}
				else if(resta < 0 && resta > -180) 
				{
					_azimuthBackList.push_back(it);
				}
				else if (resta > 0 && resta > 180)
				{
					_azimuthBackList.push_back(it);
				}
				else
				{
					_azimuthFrontList.push_back(it);
				}
			}


			//Split
			//std::copy_if(listToSort.begin(), listToSort.end(), back_inserter(_azimuthBackList), [_newAzimuth](orientation n) {return n.azimuth <= _newAzimuth; });

			//std::copy_if(listToSort.begin(), listToSort.end(), back_inserter(_azimuthFrontList), [_newAzimuth](orientation n) {return n.azimuth > _newAzimuth; });
		}

		void SortListByElevationAndSplit(float _newElevation, std::vector<orientation>& listToSort, std::vector<orientation>& ceilList, std::vector<orientation>& floorList)
		{
			// Sort List By Elevation
			if (listToSort.size() != 0) {
				std::sort(listToSort.begin(), listToSort.end(), [](const orientation& a, const orientation& b) { return a.elevation < b.elevation; });
			}
			else {
				SET_RESULT(RESULT_WARNING, "Orientation list sorted by distances is empty");
			}

			float elevationFromListTransformed;
			for (auto& it : listToSort)
			{
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

			//std::copy_if(listToSort.begin(), listToSort.end(), back_inserter(floorList), [_newElevation](orientation n) {return n.elevation <= _newElevation; });

			//std::copy_if(listToSort.begin(), listToSort.end(), back_inserter(ceilList), [_newElevation](orientation n) {return n.elevation > _newElevation; });
		}

		std::vector<T_PairDistanceOrientation> GetSortedDistancesList(const std::vector<orientation>& listToSort, float _newAzimuth, float _newElevation)
		{

			T_PairDistanceOrientation temp;
			float distance;
			std::vector<T_PairDistanceOrientation> sortedList;
			sortedList.reserve(listToSort.size());

			// Algorithm to calculate the three shortest distances between the point (newAzimuth, newelevation) and all the points in the given list
			for (auto it = listToSort.begin(); it != listToSort.end(); ++it)
			{
				distance = CHRTFAuxiliarMethods::CalculateDistance_HaversineFormula(_newAzimuth, _newElevation, it->azimuth, it->elevation);

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

		THRIRStruct DataInterpolation(const T_HRTFTable& table, TBarycentricCoordinatesStruct _barycentricCoordinates, int HRIRLength, float azimuthPnt1, float elevationPnt1, float azimuthPnt2, float elevationPnt2, float azimuthPnt3, float elevationPnt3)
		{
			// Calculate the new HRIR with the barycentric coorfinates
			auto it1 = table.find(orientation(azimuthPnt1, elevationPnt1));
			auto it2 = table.find(orientation(azimuthPnt2, elevationPnt2));
			auto it3 = table.find(orientation(azimuthPnt3, elevationPnt3));

			THRIRStruct newHRIR;

			if (it1 != table.end() && it2 != table.end() && it3 != table.end()) {

				//FIXME!!! another way to initialize?
				newHRIR = it1->second;
				//END FIXME

				for (int i = 0; i < HRIRLength; i++) {
					newHRIR.leftHRIR[i] = _barycentricCoordinates.alpha * it1->second.leftHRIR[i] + _barycentricCoordinates.beta * it2->second.leftHRIR[i] + _barycentricCoordinates.gamma * it3->second.leftHRIR[i];
					newHRIR.rightHRIR[i] = _barycentricCoordinates.alpha * it1->second.rightHRIR[i] + _barycentricCoordinates.beta * it2->second.rightHRIR[i] + _barycentricCoordinates.gamma * it3->second.rightHRIR[i];
				}

				// Calculate delay
				newHRIR.leftDelay = _barycentricCoordinates.alpha * it1->second.leftDelay + _barycentricCoordinates.beta * it2->second.leftDelay + _barycentricCoordinates.gamma * it3->second.leftDelay;
				newHRIR.rightDelay = _barycentricCoordinates.alpha * it1->second.rightDelay + _barycentricCoordinates.beta * it2->second.rightDelay + _barycentricCoordinates.gamma * it3->second.rightDelay;
				//SET_RESULT(RESULT_OK, "HRIR calculated with interpolation method succesfully");
				return newHRIR;
			}
			else {
				SET_RESULT(RESULT_WARNING, "GetHRIR_InterpolationMethod return empty because HRIR with a specific orientation was not found");
				return newHRIR;
			}
		}


	};

}
#endif
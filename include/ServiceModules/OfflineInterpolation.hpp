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
		virtual THRIRStruct CalculateHRIR_offlineMethod(const T_HRTFTable& table, const std::vector<orientation>& listToSort, float newAzimuth, float newElevation, int HRIRLength, int pole) = 0;
	};

	class CDistanceBasedInterpolator : COfflineInterpolatorInterface {
	public:

		THRIRStruct CalculateHRIR_offlineMethod(const T_HRTFTable& table, const std::vector<orientation>& listToSort, float newAzimuth, float newElevation, int HRIRLength, int pole = 0)
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


}
#endif
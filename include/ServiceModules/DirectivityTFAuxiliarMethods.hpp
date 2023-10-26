/**
* \class CDirectivityTFAuxiliarMethods
*
* \brief Declaration of CDirectivityTFAuxiliarMethods class 
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


#ifndef _CDIRECTIVITYTF_AUXILIARMETHODS_HPP
#define _CDIRECTIVITYTF_AUXILIARMETHODS_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/GlobalParameters.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/DirectivityTFDefinitions.hpp>
#include <ServiceModules/OfflineInterpolationAuxiliarMethods.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>


namespace BRTServices
{

	/** \brief Auxiliary methods used in different classes working with HRTFs
*/
	class CDirectivityTFAuxiliarMethods {
	public:

		struct CalculateDirectivityTFFromHemisphereParts {
			//static THRIRStruct CalculateHRIRFromHemisphereParts(T_HRTFTable& _t_HRTF_DataBase, int _HRIRLength, std::vector < std::vector <orientation>> _hemisphereParts) {
			BRTServices::TDirectivityTFStruct operator () (T_DirectivityTFTable& _t_DirectivityTF_DataBase, int _DirectivityTFLength, std::vector < std::vector <orientation>> _hemisphereParts) {

				BRTServices::TDirectivityTFStruct calculatedDirectivityTF;

				//Calculate the DirectivityTF of each hemisphere part
				std::vector< BRTServices::TDirectivityTFStruct> newDirectivityTF;
				newDirectivityTF.resize(_hemisphereParts.size());

				for (int q = 0; q < _hemisphereParts.size(); q++)
				{
					newDirectivityTF[q].realPart.resize(_DirectivityTFLength, 0.0f);
					newDirectivityTF[q].imagPart.resize(_DirectivityTFLength, 0.0f);

					float scaleFactor;
					if (_hemisphereParts[q].size() != 0) {
						scaleFactor = 1.0f / _hemisphereParts[q].size();
					}
					else { scaleFactor = 0.0f; }

					for (auto it = _hemisphereParts[q].begin(); it != _hemisphereParts[q].end(); it++)
					{
						auto itDirectivityTF = _t_DirectivityTF_DataBase.find(orientation(it->azimuth, it->elevation));

						//Get the DirectivityTF
						for (int i = 0; i < _DirectivityTFLength; i++) {
							newDirectivityTF[q].realPart[i] = (newDirectivityTF[q].realPart[i] + itDirectivityTF->second.realPart[i] * scaleFactor);
							newDirectivityTF[q].imagPart[i] = (newDirectivityTF[q].imagPart[i] + itDirectivityTF->second.imagPart[i] * scaleFactor);
						}
					}
				}

				//Get the FINAL values
				float scaleFactor_final = 1.0f / _hemisphereParts.size();

				//calculate Final HRIR
				calculatedDirectivityTF.realPart.resize(_DirectivityTFLength, 0.0f);
				calculatedDirectivityTF.imagPart.resize(_DirectivityTFLength, 0.0f);

				for (int i = 0; i < _DirectivityTFLength; i++)
				{
					for (int q = 0; q < _hemisphereParts.size(); q++)
					{
						calculatedDirectivityTF.realPart[i] = calculatedDirectivityTF.realPart[i] + newDirectivityTF[q].realPart[i] * scaleFactor_final;
						calculatedDirectivityTF.imagPart[i] = calculatedDirectivityTF.imagPart[i] + newDirectivityTF[q].imagPart[i] * scaleFactor_final;
					}
				}
				return calculatedDirectivityTF;
			}
		};
	
		/**
		 * @brief Returns an DirectivityTF filled with zeros in all cases.
		*/
		struct GetZerosDirectivityTF {

			/**
			 * @brief Returns an Directivity TF filled with zeros in all cases.
			 * @param table data table
			 * @param orientations List Orientations of the data table. This data is not used
			 * @param _azimuth This data is not used
			 * @param _elevation This data is not used
			 * @return TDirectivityTFStruct filled with zeros
			*/
			TDirectivityTFStruct operator() (const T_DirectivityTFTable& table, const std::vector<orientation>& orientationsList, int _DirectivityTFLength, double _azimuth, double _elevation) {
				
				TDirectivityTFStruct directivityTFZeros;								
				directivityTFZeros.realPart.resize(_DirectivityTFLength, 0.0f);
				directivityTFZeros.imagPart.resize(_DirectivityTFLength, 0.0f);
				return directivityTFZeros;
			}
		};


		/**
		 * @brief 
		 * @param 
		*/
		struct CalculateDirectivityTF_FromBarycentrics_OfflineInterpolation {

			BRTServices::TDirectivityTFStruct operator () (const T_DirectivityTFTable& _table, orientation _orientation1, orientation _orientation2, orientation _orientation3, int _HRIRLength, BRTServices::TBarycentricCoordinatesStruct barycentricCoordinates) {

				BRTServices::TDirectivityTFStruct calculatedDirectivityTF;
				calculatedDirectivityTF.realPart.resize(_HRIRLength, 0.0f);
				calculatedDirectivityTF.imagPart.resize(_HRIRLength, 0.0f);

				// Calculate the new DirectivityTF with the barycentric coordinates
				auto it1 = _table.find(_orientation1);
				auto it2 = _table.find(_orientation2);
				auto it3 = _table.find(_orientation3);

				if (it1 != _table.end() && it2 != _table.end() && it3 != _table.end()) 
				{
					for (int i = 0; i < _HRIRLength; i++) {
						calculatedDirectivityTF.realPart[i] = barycentricCoordinates.alpha * it1->second.realPart[i] + barycentricCoordinates.beta * it2->second.realPart[i] + barycentricCoordinates.gamma * it3->second.realPart[i];
						calculatedDirectivityTF.imagPart[i] = barycentricCoordinates.alpha * it1->second.imagPart[i] + barycentricCoordinates.beta * it2->second.imagPart[i] + barycentricCoordinates.gamma * it3->second.imagPart[i];
					}
					return calculatedDirectivityTF;
				}

				else {
					SET_RESULT(RESULT_WARNING, "GetDirectivityTF_OffInterpolationMethod return empty because DirectivityTF with a specific orientation was not found");
					return calculatedDirectivityTF;
				}

			}
		};
	
		/**
		 * @brief Given any point returns the DirectivityTF of the closest point to that point.
		*/
		struct GetNearestPointDirectivityTF {
			/**
			 * @brief Given any point returns the DirectivityTF of the closest point to that point.
			 * @param table data table
			 * @param orientationsList List Orientations of the data table
			 * @param _azimuth point of interest azimuth
			 * @param _elevation point of interest elevation
			 * @return TDirectivityTFStruct filled with the nearest point data
			*/
			TDirectivityTFStruct operator() (const T_DirectivityTFTable& table, const std::vector<orientation>& orientationsList, int _DirectivityTFLength, double _azimuth, double _elevation) {
				// Order list of orientation
				std::vector<T_PairDistanceOrientation> pointsOrderedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(orientationsList, _azimuth, _elevation);
				// Get nearest
				double nearestAzimuth = pointsOrderedByDistance.begin()->second.azimuth;
				double nearestElevation = pointsOrderedByDistance.begin()->second.elevation;
				// Find nearest HRIR and copy
				TDirectivityTFStruct nearestDirectivityTF;

				auto it = table.find(orientation(nearestAzimuth, nearestElevation));
				if (it != table.end()) {
					nearestDirectivityTF = it->second;
				}
				else {
					SET_RESULT(RESULT_WARNING, "No point close enough to make the extrapolation has been found, this must not happen.");
						
					nearestDirectivityTF.realPart.resize(_DirectivityTFLength, 0.0f);
					nearestDirectivityTF.imagPart.resize(_DirectivityTFLength, 0.0f);
				}

				return nearestDirectivityTF;
			}
		};
	};

}
#endif
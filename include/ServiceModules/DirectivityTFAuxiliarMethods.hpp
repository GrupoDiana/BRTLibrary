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
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
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
#include <ServiceModules/ServicesBase.hpp>
#include <ServiceModules/DirectivityTFDefinitions.hpp>
#include <ServiceModules/OfflineInterpolationAuxiliarMethods.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>


namespace BRTServices
{

	/** \brief Auxiliary methods used in different classes working with HRTFs
*/
	class CDirectivityTFAuxiliarMethods {
	public:
		/**
		 * @brief Struct that contain a function to Calculate the Directivity TF using the Quadrant-based method
		*/
		struct CalculateDirectivityTFFromHemisphereParts {
			/**
			 * @brief Calculate the Directivity TF using the Quadrant-based method
			 * @param _t_DirectivityTF_DataBase Table with the loaded darectivities (from the SOFA file)
			 * @param _DirectivityTFLength Size of the directivity TF (real part or imaginary part) that came from the DataBase table
			 * @param _hemisphereParts Vector that contains all the oritations divided in the four quadrants
			 * @return 
			*/
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
		 * @brief Struct that contains the function to do the off-line calculation of the interpolated directivity TF using the three nearest points and the barycentric coordinates
		*/
		struct CalculateDirectivityTF_FromBarycentrics_OfflineInterpolation {

			/**
			 * @brief Off-line calculation of the interpolated directivity TF using the three nearest points and the barycentric coordinates
			 * @param _table T_DirectivityTF Table
			 * @param _orientation1 Orientation of one of the three points used for the interpolation
			 * @param _orientation2 Orientation of one of the three points used for the interpolation
			 * @param _orientation3 Orientation of one of the three points used for the interpolation
			 * @param _DirectivityTFLength Lenght of the real part (or imag part, because it should be the same) of the directivity TF
			 * @param _barycentricCoordinates Value of the three barycentric coordinates used to get the new interpolated DitectivityTF
			 * @return 
			*/
			BRTServices::TDirectivityTFStruct operator () (const T_DirectivityTFTable& _table, orientation _orientation1, orientation _orientation2, orientation _orientation3, int _DirectivityTFLength, BRTServices::TBarycentricCoordinatesStruct _barycentricCoordinates) {

				BRTServices::TDirectivityTFStruct calculatedDirectivityTF;
				calculatedDirectivityTF.realPart.resize(_DirectivityTFLength, 0.0f);
				calculatedDirectivityTF.imagPart.resize(_DirectivityTFLength, 0.0f);

				// Calculate the new DirectivityTF with the barycentric coordinates
				auto it1 = _table.find(_orientation1);
				auto it2 = _table.find(_orientation2);
				auto it3 = _table.find(_orientation3);

				if (it1 != _table.end() && it2 != _table.end() && it3 != _table.end()) 
				{
					for (int i = 0; i < _DirectivityTFLength; i++) {
						calculatedDirectivityTF.realPart[i] = _barycentricCoordinates.alpha * it1->second.realPart[i] + _barycentricCoordinates.beta * it2->second.realPart[i] + _barycentricCoordinates.gamma * it3->second.realPart[i];
						calculatedDirectivityTF.imagPart[i] = _barycentricCoordinates.alpha * it1->second.imagPart[i] + _barycentricCoordinates.beta * it2->second.imagPart[i] + _barycentricCoordinates.gamma * it3->second.imagPart[i];
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

		/**
		 * @brief Struct that contains the function to do the on-line calculation of the interpolated directivity TF using the three nearest points and the barycentric coordinates
		*/
		struct CalculateDirectivityTF_FromBarycentric_OnlineInterpolation {
			/**
			 * @brief On-line Calculation of the interpolated directivity TF using the three nearest points and the barycentric coordinates
			 * @param _resampledTable DirectivityTF Interlaced Data Table
			 * @param _numberOfSubfilters 
			 * @param _subfilterLength 
			 * @param _barycentricCoordinates Value of the three barycentric coordinates used to get the new interpolated DitectivityTF
			 * @param orientation_pto1 Orientation of one of the three points used for the interpolation
			 * @param orientation_pto2 Orientation of one of the three points used for the interpolation
			 * @param orientation_pto3 Orientation of one of the three points used for the interpolation
			 * @return 
			*/
			const TDirectivityInterlacedTFStruct operator()(const T_DirectivityTFInterlacedDataTable& _resampledTable, int32_t _numberOfSubfilters, int32_t _subfilterLength, TBarycentricCoordinatesStruct _barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)
			{
				TDirectivityInterlacedTFStruct newDirectivityTF;

				// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
				auto it1 = _resampledTable.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = _resampledTable.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = _resampledTable.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != _resampledTable.end() && it2 != _resampledTable.end() && it3 != _resampledTable.end())
				{
					newDirectivityTF.data.resize(_numberOfSubfilters);

					for (int subfilterID = 0; subfilterID < _numberOfSubfilters; subfilterID++)
					{
						newDirectivityTF.data[subfilterID].resize(_subfilterLength);
						for (int i = 0; i < _subfilterLength; i++)
						{
							newDirectivityTF.data[subfilterID][i] = _barycentricCoordinates.alpha * it1->second.data[subfilterID][i] + _barycentricCoordinates.beta * it2->second.data[subfilterID][i] + _barycentricCoordinates.gamma * it3->second.data[subfilterID][i];
						}
					}
				}
				else {
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateDirectivityTF_FromBarycentricCoordinates() not found");
				}

				return newDirectivityTF;
			}
		};


	};

}
#endif
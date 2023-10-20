/**
* \class CHRTFAuxiliarMethods
*
* \brief Declaration of CHRTFAuxiliarMethods class interface
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


#ifndef _CHRTF_DEFINITIONS_HPP
#define _CHRTF_DEFINITIONS_HPP

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

namespace BRTServices {


#ifndef PI 
#define PI 3.14159265
#endif
#ifndef DEFAULT_RESAMPLING_STEP
#define DEFAULT_RESAMPLING_STEP 5
#endif

#ifndef DEFAULT_HRTF_MEASURED_DISTANCE
#define DEFAULT_HRTF_MEASURED_DISTANCE 1.95f
#endif

#ifndef DEFAULT_EXTRAPOLATION_STEP
#define DEFAULT_EXTRAPOLATION_STEP 10
#endif

	/** \brief Type definition for a left-right pair of impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	struct THRIRPartitionedStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		std::vector<CMonoBuffer<float>> leftHRIR_Partitioned;	///< Left partitioned impulse response data
		std::vector<CMonoBuffer<float>> rightHRIR_Partitioned;	///< Right partitioned impulse response data

		THRIRPartitionedStruct() : leftDelay{ 0 }, rightDelay{ 0 } {}
	};

	/** \brief Type definition for an impulse response with the ITD removed and stored in a specific struct field
	*/
	struct oneEarHRIR_struct {
		uint64_t delay;				///< Delay, in number of samples
		CMonoBuffer<float> HRIR;	///< Impulse response data
	};

	/** \brief Type definition for an impulse response subfilter set with the ITD removed and stored in a specific struct field
	*/
	struct TOneEarHRIRPartitionedStruct {
		std::vector<CMonoBuffer<float>> HRIR_Partitioned;	///< Partitioned impulse response data
		uint64_t delay;				///< Delay, in number of samples
	};

	/**	\brief Type definition for barycentric coordinates
	*/
	struct TBarycentricCoordinatesStruct {
		float alpha;	///< Coordinate alpha
		float beta;		///< Coordinate beta
		float gamma;	///< Coordinate gamma
	};


	/** \brief Type definition for the HRTF table
	*/
	typedef std::unordered_map<orientation, BRTServices::THRIRStruct> T_HRTFTable;

	/** \brief Type definition for the HRTF partitioned table used when UPConvolution is activated
	*/
	typedef std::unordered_map<orientation, THRIRPartitionedStruct> T_HRTFPartitionedTable;

	/** \brief Type definition for a distance-orientation pair
	*/
	typedef std::pair <float, orientation> T_PairDistanceOrientation;


	/** \brief Type definitions to indentify the poles
	*/
	enum class TPole { north, south };


	/** \brief Auxiliary methods used in different classes working with HRTFs
	*/
	class CHRTFAuxiliarMethods {
	public:

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
		 *	@brief Transform azimuth range to [0, 360]
		 *  @param [in] azimuth to be checked and transformed, just in case.
		 *  @return azimuth changed to the new range
		*/
		static double CalculateAzimuthIn0_360Range(double _azimuth) {						
			if (_azimuth < 0) {
				_azimuth = std::fmod(_azimuth, (float)360) + 360;
			} else if ( _azimuth >= 360) {
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
				T_PairDistanceOrientation temp (distance, _orientation);
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

			if (round(denominator) == 0) {	//if denominator=0 -> no triangle -> barycentric coordinates NO VALID 
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

		/**
		 * @brief Calculate HRIR subfilters using a barycentric coordinates of the three nearest orientation.
		 * @param ear
		 * @param barycentricCoordinates
		 * @param orientation_pto1
		 * @param orientation_pto2
		 * @param orientation_pto3
		 * @return
		*/
		static const THRIRPartitionedStruct CalculateHRIR_partitioned_FromBarycentricCoordinates(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)
		{
			THRIRPartitionedStruct data;
			//std::vector<CMonoBuffer<float>> newHRIR;

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (orientation_pto1.azimuth == DEFAULT_MAX_AZIMUTH) { orientation_pto1.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (orientation_pto2.azimuth == DEFAULT_MAX_AZIMUTH) { orientation_pto2.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (orientation_pto3.azimuth == DEFAULT_MAX_AZIMUTH) { orientation_pto3.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (orientation_pto1.elevation == DEFAULT_MAX_ELEVATION) { orientation_pto1.elevation = DEFAULT_MIN_ELEVATION; }
				if (orientation_pto2.elevation == DEFAULT_MAX_ELEVATION) { orientation_pto2.elevation = DEFAULT_MIN_ELEVATION; }
				if (orientation_pto3.elevation == DEFAULT_MAX_ELEVATION) { orientation_pto3.elevation = DEFAULT_MIN_ELEVATION; }

				// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
				auto it1 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{
					int subfilterLength = HRIR_partitioned_SubfilterLength;
					//newHRIR.resize(HRIR_partitioned_NumberOfSubfilters);

					if (ear == Common::T_ear::LEFT)
					{
						data.leftHRIR_Partitioned.resize(HRIR_partitioned_NumberOfSubfilters);

						for (int subfilterID = 0; subfilterID < HRIR_partitioned_NumberOfSubfilters; subfilterID++)
						{
							data.leftHRIR_Partitioned[subfilterID].resize(subfilterLength);
							for (int i = 0; i < subfilterLength; i++)
							{
								data.leftHRIR_Partitioned[subfilterID][i] = barycentricCoordinates.alpha * it1->second.leftHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.beta * it2->second.leftHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.gamma * it3->second.leftHRIR_Partitioned[subfilterID][i];
							}
						}
					}

					else if (ear == Common::T_ear::RIGHT)
					{
						data.rightHRIR_Partitioned.resize(HRIR_partitioned_NumberOfSubfilters);
						for (int subfilterID = 0; subfilterID < HRIR_partitioned_NumberOfSubfilters; subfilterID++)
						{
							data.rightHRIR_Partitioned[subfilterID].resize(subfilterLength, 0.0f);
							for (int i = 0; i < subfilterLength; i++)
							{
								data.rightHRIR_Partitioned[subfilterID][i] = barycentricCoordinates.alpha * it1->second.rightHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.beta * it2->second.rightHRIR_Partitioned[subfilterID][i] + barycentricCoordinates.gamma * it3->second.rightHRIR_Partitioned[subfilterID][i];
							}
						}
					}

					else {
						SET_RESULT(RESULT_WARNING, "Ear Type for calculating HRIR from Barycentric Coordinates is not valid");
					}

					//SET_RESULT(RESULT_OK, "CalculateHRIRFromBarycentricCoordinates completed succesfully");
				}
				else {
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateHRIR_partitioned_FromBarycentricCoordinates not found");
				}
			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateHRIR_partitioned_FromBarycentricCoordinates");
			}

			return data;
		}

		/**
		 * @brief Calculate HRIR DELAY using a barycentric coordinates of the three nearest orientation, in number of samples
		 * @param ear
		 * @param barycentricCoordinates
		 * @param orientation_pto1
		 * @param orientation_pto2
		 * @param orientation_pto3
		 * @return
		*/
		static const THRIRPartitionedStruct CalculateHRIRDelayFromBarycentricCoordinates(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)
		{
			THRIRPartitionedStruct data;

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (orientation_pto1.azimuth == DEFAULT_MAX_AZIMUTH) { orientation_pto1.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (orientation_pto2.azimuth == DEFAULT_MAX_AZIMUTH) { orientation_pto2.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (orientation_pto3.azimuth == DEFAULT_MAX_AZIMUTH) { orientation_pto3.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (orientation_pto1.elevation == DEFAULT_MAX_ELEVATION) { orientation_pto1.elevation = DEFAULT_MIN_ELEVATION; }
				if (orientation_pto2.elevation == DEFAULT_MAX_ELEVATION) { orientation_pto2.elevation = DEFAULT_MIN_ELEVATION; }
				if (orientation_pto3.elevation == DEFAULT_MAX_ELEVATION) { orientation_pto3.elevation = DEFAULT_MIN_ELEVATION; }

				// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
				auto it1 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{

					data.leftDelay = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.leftDelay + barycentricCoordinates.beta * it2->second.leftDelay + barycentricCoordinates.gamma * it3->second.leftDelay));
					data.rightDelay = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.rightDelay + barycentricCoordinates.beta * it2->second.rightDelay + barycentricCoordinates.gamma * it3->second.rightDelay));
					//SET_RESULT(RESULT_OK, "CalculateHRIRFromBarycentricCoordinates completed succesfully");
				}
				else {
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateHRIRDelayFromBarycentricCoordinates not found");
				}
			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateHRIRDelayFromBarycentricCoordinates");
			}

			return data;
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

		/// <summary>
		/// Calculate HRIR and delay from a given set of orientations		
		/// </summary>
		/// <param name="_t_HRTF_DataBase"></param>
		/// <param name="_HRIRLength"></param>
		/// <param name="_hemisphereParts"></param>
		/// <returns></returns>
		struct CalculateHRIRFromHemisphereParts{
			//static THRIRStruct CalculateHRIRFromHemisphereParts(T_HRTFTable& _t_HRTF_DataBase, int _HRIRLength, std::vector < std::vector <orientation>> _hemisphereParts) {
			BRTServices::THRIRStruct operator () (T_HRTFTable& _t_HRTF_DataBase, int _HRIRLength, std::vector < std::vector <orientation>> _hemisphereParts) {

				BRTServices::THRIRStruct calculatedHRIR;

				//Calculate the delay and the HRIR of each hemisphere part
				float totalDelay_left = 0.0f;
				float totalDelay_right = 0.0f;

				std::vector< BRTServices::THRIRStruct> newHRIR;
				newHRIR.resize(_hemisphereParts.size());

				for (int q = 0; q < _hemisphereParts.size(); q++)
				{
					newHRIR[q].leftHRIR.resize(_HRIRLength, 0.0f);
					newHRIR[q].rightHRIR.resize(_HRIRLength, 0.0f);

					float scaleFactor;
					if (_hemisphereParts[q].size() != 0)
					{
						scaleFactor = 1.0f / _hemisphereParts[q].size();
					}
					else
					{
						scaleFactor = 0.0f;
					}

					for (auto it = _hemisphereParts[q].begin(); it != _hemisphereParts[q].end(); it++)
					{
						auto itHRIR = _t_HRTF_DataBase.find(orientation(it->azimuth, it->elevation));

						//Get the delay
						newHRIR[q].leftDelay = (newHRIR[q].leftDelay + itHRIR->second.leftDelay);
						newHRIR[q].rightDelay = (newHRIR[q].rightDelay + itHRIR->second.rightDelay);

						//Get the HRIR
						for (int i = 0; i < _HRIRLength; i++) {
							newHRIR[q].leftHRIR[i] = (newHRIR[q].leftHRIR[i] + itHRIR->second.leftHRIR[i]);
							newHRIR[q].rightHRIR[i] = (newHRIR[q].rightHRIR[i] + itHRIR->second.rightHRIR[i]);
						}
					}//END loop hemisphere part

					 //Multiply by the factor (weighted sum)
					 // TODO: Use the previous loop to multiply by the factor
					 //Delay 
					totalDelay_left = totalDelay_left + (scaleFactor * newHRIR[q].leftDelay);
					totalDelay_right = totalDelay_right + (scaleFactor * newHRIR[q].rightDelay);
					//HRIR
					for (int i = 0; i < _HRIRLength; i++)
					{
						newHRIR[q].leftHRIR[i] = newHRIR[q].leftHRIR[i] * scaleFactor;
						newHRIR[q].rightHRIR[i] = newHRIR[q].rightHRIR[i] * scaleFactor;
					}
				}

				//Get the FINAL values
				float scaleFactor_final = 1.0f / _hemisphereParts.size();

				//Calculate Final delay
				calculatedHRIR.leftDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_left));
				calculatedHRIR.rightDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_right));

				//calculate Final HRIR
				calculatedHRIR.leftHRIR.resize(_HRIRLength, 0.0f);
				calculatedHRIR.rightHRIR.resize(_HRIRLength, 0.0f);

				for (int i = 0; i < _HRIRLength; i++)
				{
					for (int q = 0; q < _hemisphereParts.size(); q++)
					{
						calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] + newHRIR[q].leftHRIR[i];
						calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] + newHRIR[q].rightHRIR[i];
					}
				}
				for (int i = 0; i < _HRIRLength; i++)
				{
					calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] * scaleFactor_final;
					calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] * scaleFactor_final;
				}

				return calculatedHRIR;
		}
		};

		/**
		 * @brief Returns an HRIR filled with zeros in all cases.
		*/
		struct GetZerosHRIR {

			/**
			 * @brief Returns an HRIR filled with zeros in all cases.
			 * @param table data table
			 * @param orientations List Orientations of the data table. This data is not used
			 * @param _azimuth This data is not used
			 * @param _elevation This data is not used
			 * @return HRIR struct filled with zeros
			*/
			THRIRStruct operator() (const T_HRTFTable& table, const std::vector<orientation>& orientationsList, double _azimuth, double _elevation) {
				// Initialization
				int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one
				THRIRStruct HRIRZeros;
				HRIRZeros.leftHRIR.resize(HRIRSize, 0);
				HRIRZeros.rightHRIR.resize(HRIRSize, 0);
				return HRIRZeros;
			}
		};

		/**
		 * @brief Given any point returns the HRIR of the closest point to that point.
		*/
		struct GetNearestPointHRIR {
			/**
			 * @brief Given any point returns the HRIR of the closest point to that point.
			 * @param table data table
			 * @param orientationsList List Orientations of the data table
			 * @param _azimuth point of interest azimuth
			 * @param _elevation point of interest elevation
			 * @return HRIR struct filled with the nearest point data
			*/
			THRIRStruct operator() (const T_HRTFTable& table, const std::vector<orientation>& orientationsList, double _azimuth, double _elevation) {
				// Order list of orientation
				std::vector<T_PairDistanceOrientation> pointsOrderedByDistance = CHRTFAuxiliarMethods::GetListOrderedDistancesToPoint(orientationsList, _azimuth, _elevation);
				// Get nearest
				double nearestAzimuth = pointsOrderedByDistance.begin()->second.azimuth;
				double nearestElevation = pointsOrderedByDistance.begin()->second.elevation;
				// Find nearest HRIR and copy
				THRIRStruct nearestHRIR;

				auto it = table.find(orientation(nearestAzimuth, nearestElevation));
				if (it != table.end()) {
					nearestHRIR = it->second;
				}
				else {
					SET_RESULT(RESULT_WARNING, "No point close enough to make the extrapolation has been found, this must not happen.");

					int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one					
					nearestHRIR.leftHRIR.resize(HRIRSize, 0);
					nearestHRIR.rightHRIR.resize(HRIRSize, 0);
				}

				return nearestHRIR;
			}
		};
	};
}
#endif
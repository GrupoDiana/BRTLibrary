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




	enum class TPole { north, south };



	class CHRTFAuxiliarMethods {
	public:

		/** \brief Get Pole Elevation
		*	\param [in] Tpole var that indicates of which pole we need elevation
		*   \eh  On error, an error code is reported to the error handler.
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


		static float CheckLimitsElevation_and_Transform(float elevation)
		{
			if (elevation < 0) { elevation = elevation + 360; }
			if (elevation >= 360) { elevation = elevation - 360; }
			return elevation;

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

			ASSERT(raiz > 0, RESULT_ERROR_OUTOFRANGE, "Attempt to compute square root of a negative value using Haversine Formula to compute distance", "");
			float sqrtDistance = std::sqrt(raiz);

			ASSERT(sqrtDistance >= -1.0f && sqrtDistance <= 1.0f, RESULT_ERROR_OUTOFRANGE,
				"Attempt to compute arcsin of a value outside [-1..1] using Harvesine Formula to compute distnace",
				"");

			float distance = std::asin(std::sqrt(raiz));

			return distance;
		}

		//		Transform the orientation in order to move the orientation of interest to 180 degrees
		//returnval	float	transformed azimuth		
		static float TransformAzimuth(float azimuthOrientationOfInterest, float originalAzimuth)
		{
			float azimuth;
			azimuth = originalAzimuth + 180 - azimuthOrientationOfInterest;

			// Check limits (always return 0 instead of 360)
			if (azimuth >= DEFAULT_MAX_AZIMUTH)
				azimuth = std::fmod(azimuth, (float)360);

			if (azimuth < DEFAULT_MIN_AZIMUTH)
				azimuth = azimuth + 360;

			return azimuth;
		}

		//		Transform the orientation in order to express the elevation in the interval [-90,90]
		//returnval float transformed elevation		
		static float TransformElevation(float elevationOrientationOfInterest, float originalElevation)
		{
			if (originalElevation >= ELEVATION_SOUTH_POLE) {
				originalElevation = originalElevation - 360;
			}
			return originalElevation;
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
	};
}
#endif
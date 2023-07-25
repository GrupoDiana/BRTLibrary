/**
* \class COnlineInterpolatorInterface, CMidPointOnlineInterpolator
*
* \brief Declaration of COnlineInterpolatorInterface and CMidPointOnlineInterpolator classes interface
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


#ifndef _CONLINE_INTERPOLATION_HPP
#define _CONLINE_INTERPOLATION_HPP


#include <unordered_map>
#include <vector>
#include <ServiceModules/HRTFDefinitions.hpp>

namespace BRTServices
{

	class COnlineInterpolatorInterface {
	public:
		/**
		 * @brief Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @return 
		*/
		virtual	THRIRPartitionedStruct CalculateHRIRPartitioned_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap) const = 0;
		
		/**
		 * @brief  Calculate from resample table DELAY using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @return 
		*/
		virtual	THRIRPartitionedStruct CalculateDelay_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap) const = 0;

	};

	class CMidPointOnlineInterpolator :COnlineInterpolatorInterface {
	public:

		enum TParameterToBeCalculated { HRIR, Delay };

		/**
		 * @brief Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateHRIRPartitioned_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap)const {
			return CalculateHRIRPartitionedDelay_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear,
				_azimuth, _elevation, stepMap, TParameterToBeCalculated::HRIR);

		}

		/**
		 * @brief  Calculate from resample table DELAY using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateDelay_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap) const {
			return CalculateHRIRPartitionedDelay_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear,
				_azimuth, _elevation, stepMap, TParameterToBeCalculated::Delay);
		}
	private:

		/**
		 * @brief  Calculate from resample table DELAY using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @param _parameterToBeCalculated 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateHRIRPartitionedDelay_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, TParameterToBeCalculated _parameterToBeCalculated) const {
			//std::vector<CMonoBuffer<float>> newHRIR;
			THRIRPartitionedStruct data;

			TBarycentricCoordinatesStruct barycentricCoordinates;
			orientation orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP;
			std::pair<float, float>nearestElevations;

			find_4Nearest_Points(_azimuth, _elevation, stepMap, orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP, nearestElevations);
			float eleCeil = nearestElevations.first;
			float eleFloor = nearestElevations.second;

			// Depend on the quadrant where the point of interest is situated obtain the Barycentric coordinates and the HRIR of the orientation of interest (azimuth, elevation)
			if (_azimuth >= orientation_ptoP.azimuth)
			{
				if (_elevation >= orientation_ptoP.elevation)
				{
					//Second quadrant
					data = CalculateBaricentricHRIRInterpolation(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoA, orientation_ptoB, orientation_ptoD, orientation_ptoC, _parameterToBeCalculated);
				}
				else if (_elevation < orientation_ptoP.elevation)
				{
					//Forth quadrant
					data = CalculateBaricentricHRIRInterpolation(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoA, _parameterToBeCalculated);
				}
			}
			else if (_azimuth < orientation_ptoP.azimuth)
			{
				if (_elevation >= orientation_ptoP.elevation)
				{
					//First quadrant
					data = CalculateBaricentricHRIRInterpolation(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, _parameterToBeCalculated);
				}
				else if (_elevation < orientation_ptoP.elevation) {
					//Third quadrant
					data = CalculateBaricentricHRIRInterpolation(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoA, orientation_ptoC, orientation_ptoD, orientation_ptoB, _parameterToBeCalculated);
				}
			}
			//SET_RESULT(RESULT_OK, "GetHRIR_partitioned_InterpolationMethod completed succesfully");
			return data;

		}

		/**
		 * @brief  Calculate from resample table HRIR using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param elevationCeil 
		 * @param elevationFloor 
		 * @param point1 
		 * @param point2 
		 * @param point3 
		 * @param point4 
		 * @param parameterToBeCalculated 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateBaricentricHRIRInterpolation(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength,
			Common::T_ear ear, float _azimuth, float _elevation, float elevationCeil, float elevationFloor, orientation point1, orientation point2, orientation point3, orientation point4, TParameterToBeCalculated parameterToBeCalculated) const
		{
			//std::vector<CMonoBuffer<float>> newHRIR;
			THRIRPartitionedStruct data;
			TBarycentricCoordinatesStruct barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, point1.azimuth, point1.elevation, point2.azimuth, point2.elevation, point3.azimuth, point3.elevation);

			if (barycentricCoordinates.alpha < 0 || barycentricCoordinates.beta < 0 || barycentricCoordinates.gamma < 0) {
				barycentricCoordinates = Check_Triangles_Left(_azimuth, _elevation, point1, point2, point3, point4);
			}

			if (elevationCeil == ELEVATION_NORTH_POLE) { point2.azimuth = DEFAULT_MIN_AZIMUTH; }
			else if (elevationFloor == ELEVATION_SOUTH_POLE) { point3.azimuth = DEFAULT_MIN_AZIMUTH; }

			if (parameterToBeCalculated == TParameterToBeCalculated::HRIR) {
				data = CHRTFAuxiliarMethods::CalculateHRIR_partitioned_FromBarycentricCoordinates(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, barycentricCoordinates, point1, point2, point3);
			}
			else {
				data = CHRTFAuxiliarMethods::CalculateHRIRDelayFromBarycentricCoordinates(t_HRTF_Resampled_partitioned, barycentricCoordinates, point1, point2, point3);
			}

			return data;
		}

		/**
		 * @brief Find 4 nearest points (trapezoid) in the quasiUniform sphere distribution
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @param orientation_ptoA 
		 * @param orientation_ptoB 
		 * @param orientation_ptoC 
		 * @param orientation_ptoD 
		 * @param orientation_ptoP 
		 * @param nearestElevations 
		*/
		void find_4Nearest_Points(float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, orientation& orientation_ptoA, orientation& orientation_ptoB, orientation& orientation_ptoC, orientation& orientation_ptoD, orientation& orientation_ptoP, std::pair<float, float>& nearestElevations)const
		{
			float aziCeilBack, aziCeilFront, aziFloorBack, aziFloorFront;

			float eleStep = stepMap.find(orientation(-1, -1))->second; // Elevation Step -- Same always
			int idxEle = ceil(_elevation / eleStep);
			float eleCeil = eleStep * idxEle;
			float eleFloor = eleStep * (idxEle - 1);

			eleCeil = CHRTFAuxiliarMethods::CheckLimitsElevation_and_Transform(eleCeil);				//			   Back	  Front
			eleFloor = CHRTFAuxiliarMethods::CheckLimitsElevation_and_Transform(eleFloor);				//	Ceil		A		B

			auto stepItr = stepMap.find(orientation(0, eleCeil));										//	Floor		C		D
			float aziStepCeil = stepItr->second;

			CHRTFAuxiliarMethods::CalculateAzimuth_BackandFront(aziCeilBack, aziCeilFront, aziStepCeil, _azimuth);
			// azimuth values passed by reference

			auto stepIt = stepMap.find(orientation(0, eleFloor));
			float aziStepFloor = stepIt->second;

			CHRTFAuxiliarMethods::CalculateAzimuth_BackandFront(aziFloorBack, aziFloorFront, aziStepFloor, _azimuth);

			eleCeil = eleStep * idxEle;

			// Mid Point of a trapezoid can be compute by averaging all azimuths
			float azimuth_ptoP = (aziCeilBack + aziCeilFront + aziFloorBack + aziFloorFront) * 0.25;
			// to avoid take points above under 0 like 345,350,355 and compare with them
			float elevation_ptoP = (eleCeil - eleStep * 0.5f);

			orientation_ptoP = orientation(azimuth_ptoP, elevation_ptoP);

			// Particular case of points near poles
			if (eleCeil == ELEVATION_NORTH_POLE) { aziCeilFront = aziFloorFront; }
			else if (eleFloor == ELEVATION_SOUTH_POLE) { aziFloorFront = aziCeilFront; }

			//Calculate the quadrant points A, B, C and D and the middle quadrant point P
			orientation_ptoC.azimuth = aziFloorBack;
			orientation_ptoC.elevation = eleFloor;
			orientation_ptoA.azimuth = aziCeilBack;
			orientation_ptoA.elevation = eleCeil;
			orientation_ptoB.azimuth = aziCeilFront;
			orientation_ptoB.elevation = eleCeil;
			orientation_ptoD.azimuth = aziFloorFront;
			orientation_ptoD.elevation = eleFloor;

			nearestElevations = std::pair(eleCeil, eleFloor);
		}

		/**
		 * @brief Check Triangles Left Cause Mid Point Method may fail in some cases and choose the wrong triangle
		 * @param _azimuth 
		 * @param _elevation 
		 * @param pnt1 
		 * @param pnt2 
		 * @param pnt3 
		 * @param pnt4 
		 * @return 
		*/
		TBarycentricCoordinatesStruct Check_Triangles_Left(float _azimuth, float _elevation, orientation pnt1, orientation pnt2, orientation pnt3, orientation pnt4)const
		{
			// The triangle with points 1, 2 and 3 is the one just check, so we are going to check the others

			TBarycentricCoordinatesStruct barycentricCoordinates;
			barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, pnt1.azimuth, pnt1.elevation, pnt2.azimuth, pnt2.elevation, pnt4.azimuth, pnt4.elevation);
			if (barycentricCoordinates.alpha < 0 || barycentricCoordinates.beta < 0 || barycentricCoordinates.gamma < 0)
			{
				barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, pnt1.azimuth, pnt1.elevation, pnt3.azimuth, pnt3.elevation, pnt4.azimuth, pnt4.elevation);
				if (barycentricCoordinates.alpha < 0 || barycentricCoordinates.beta < 0 || barycentricCoordinates.gamma < 0)
				{
					barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, pnt2.azimuth, pnt2.elevation, pnt3.azimuth, pnt3.elevation, pnt4.azimuth, pnt4.elevation);
				}
			}
			return barycentricCoordinates;

		}

	};

	class CSlopesMethodOnlineInterpolator : COnlineInterpolatorInterface
	{
	public:
		enum TParameterToBeCalculated { HRIR, Delay };

		/**
		 * @brief Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateHRIRPartitioned_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap)const {
			return CalculateHRIRPartitionedDelay_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear,
				_azimuth, _elevation, stepMap, TParameterToBeCalculated::HRIR);

		}

		/**
		 * @brief  Calculate from resample table DELAY using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateDelay_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap) const {
			return CalculateHRIRPartitionedDelay_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear,
				_azimuth, _elevation, stepMap, TParameterToBeCalculated::Delay);
		}
	
	private:
		/**
		 * @brief Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned
		 * @param HRIR_partitioned_NumberOfSubfilters
		 * @param HRIR_partitioned_SubfilterLength
		 * @param ear
		 * @param _azimuth
		 * @param _elevation
		 * @param stepMap
		 * @param _parameterToBeCalculated
		 * @return
		*/
		THRIRPartitionedStruct CalculateHRIRPartitionedDelay_onlineMethod(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, Common::T_ear ear, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, TParameterToBeCalculated _parameterToBeCalculated) const {
			//std::vector<CMonoBuffer<float>> newHRIR;
			THRIRPartitionedStruct data;

			TBarycentricCoordinatesStruct barycentricCoordinates;
			orientation orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP;
			std::pair<float, float>nearestElevations;

			find_4Nearest_Points(_azimuth, _elevation, stepMap, orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP, nearestElevations);
			float eleCeil = nearestElevations.first;
			float eleFloor = nearestElevations.second;


			// SLOPE METHOD
			// First make the slope of 2 points, always the same 2 points, A->D			


			float slopeDiagonalTrapezoid		= std::abs(orientation_ptoD.elevation - orientation_ptoA.elevation) / (orientation_ptoD.azimuth - orientation_ptoA.azimuth);
			float slopeOrientationOfInterest	= std::abs(_elevation - orientation_ptoA.elevation) / (_azimuth - orientation_ptoA.azimuth);

			if (slopeOrientationOfInterest >= slopeDiagonalTrapezoid)
			{
				// Uses A,C,D
				data = CalculateBaricentricHRIRInterpolation(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation,
								eleCeil, eleFloor, orientation_ptoA, orientation_ptoC, orientation_ptoD, orientation_ptoB, _parameterToBeCalculated);
			}
			else
			{
				//Uses A,B,D
				data = CalculateBaricentricHRIRInterpolation(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation,
								eleCeil, eleFloor, orientation_ptoA, orientation_ptoB, orientation_ptoD, orientation_ptoC, _parameterToBeCalculated);
				
			}

			////SET_RESULT(RESULT_OK, "GetHRIR_partitioned_InterpolationMethod completed succesfully");
			return data;

		}

		/**
		 * @brief  Calculate from resample table HRIR using a barycentric interpolation of the three nearest orientation.
		 * @param t_HRTF_Resampled_partitioned 
		 * @param HRIR_partitioned_NumberOfSubfilters 
		 * @param HRIR_partitioned_SubfilterLength 
		 * @param ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param elevationCeil 
		 * @param elevationFloor 
		 * @param point1 
		 * @param point2 
		 * @param point3 
		 * @param point4 
		 * @param parameterToBeCalculated 
		 * @return 
		*/
		THRIRPartitionedStruct CalculateBaricentricHRIRInterpolation(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength,
			Common::T_ear ear, float _azimuth, float _elevation, float elevationCeil, float elevationFloor, orientation point1, orientation point2, orientation point3, orientation point4, TParameterToBeCalculated parameterToBeCalculated) const
		{
			//std::vector<CMonoBuffer<float>> newHRIR;
			THRIRPartitionedStruct data;
			TBarycentricCoordinatesStruct barycentricCoordinates = CHRTFAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, point1.azimuth, point1.elevation, point2.azimuth, point2.elevation, point3.azimuth, point3.elevation);

			if (elevationCeil == ELEVATION_NORTH_POLE) { point2.azimuth = DEFAULT_MIN_AZIMUTH; }
			else if (elevationFloor == ELEVATION_SOUTH_POLE) { point3.azimuth = DEFAULT_MIN_AZIMUTH; }

			if (parameterToBeCalculated == TParameterToBeCalculated::HRIR) {
				data = CHRTFAuxiliarMethods::CalculateHRIR_partitioned_FromBarycentricCoordinates(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, barycentricCoordinates, point1, point2, point3);
			}
			else {
				data = CHRTFAuxiliarMethods::CalculateHRIRDelayFromBarycentricCoordinates(t_HRTF_Resampled_partitioned, barycentricCoordinates, point1, point2, point3);
			}

			return data;
		}

		/**
		 * @brief Find 4 nearest points (trapezoid) in the quasiUniform sphere distribution
		 * @param _azimuth 
		 * @param _elevation 
		 * @param stepMap 
		 * @param orientation_ptoA 
		 * @param orientation_ptoB 
		 * @param orientation_ptoC 
		 * @param orientation_ptoD 
		 * @param orientation_ptoP 
		 * @param nearestElevations 
		*/
		void find_4Nearest_Points(float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, orientation& orientation_ptoA, orientation& orientation_ptoB, orientation& orientation_ptoC, orientation& orientation_ptoD, orientation& orientation_ptoP, std::pair<float, float>& nearestElevations)const
		{
			float aziCeilBack, aziCeilFront, aziFloorBack, aziFloorFront;

			float eleStep = stepMap.find(orientation(-1, -1))->second; // Elevation Step -- Same always
			int idxEle = ceil(_elevation / eleStep);
			float eleCeil = eleStep * idxEle;
			float eleFloor = eleStep * (idxEle - 1);

			eleCeil = CHRTFAuxiliarMethods::CheckLimitsElevation_and_Transform(eleCeil);				//			   Back	  Front
			eleFloor = CHRTFAuxiliarMethods::CheckLimitsElevation_and_Transform(eleFloor);				//	Ceil		A		B

			auto stepItr = stepMap.find(orientation(0, eleCeil));										//	Floor		C		D
			float aziStepCeil = stepItr->second;

			CHRTFAuxiliarMethods::CalculateAzimuth_BackandFront(aziCeilBack, aziCeilFront, aziStepCeil, _azimuth);
			// azimuth values passed by reference

			auto stepIt = stepMap.find(orientation(0, eleFloor));
			float aziStepFloor = stepIt->second;

			CHRTFAuxiliarMethods::CalculateAzimuth_BackandFront(aziFloorBack, aziFloorFront, aziStepFloor, _azimuth);

			eleCeil = eleStep * idxEle;

			// Mid Point of a trapezoid can be compute by averaging all azimuths
			float azimuth_ptoP = (aziCeilBack + aziCeilFront + aziFloorBack + aziFloorFront) * 0.25;
			// to avoid take points above under 0 like 345,350,355 and compare with them
			float elevation_ptoP = (eleCeil - eleStep * 0.5f);

			orientation_ptoP = orientation(azimuth_ptoP, elevation_ptoP);

			// Particular case of points near poles
			if (eleCeil == ELEVATION_NORTH_POLE) { aziCeilFront = aziFloorFront; }
			else if (eleFloor == ELEVATION_SOUTH_POLE) { aziFloorFront = aziCeilFront; }

			//Calculate the quadrant points A, B, C and D and the middle quadrant point P
			orientation_ptoC.azimuth = aziFloorBack;
			orientation_ptoC.elevation = eleFloor;
			orientation_ptoA.azimuth = aziCeilBack;
			orientation_ptoA.elevation = eleCeil;
			orientation_ptoB.azimuth = aziCeilFront;
			orientation_ptoB.elevation = eleCeil;
			orientation_ptoD.azimuth = aziFloorFront;
			orientation_ptoD.elevation = eleFloor;

			nearestElevations = std::pair(eleCeil, eleFloor);
		}

	};
}
#endif
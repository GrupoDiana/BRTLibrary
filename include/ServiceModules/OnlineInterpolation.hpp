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


#ifndef _CONLINE_INTERPOLATION_HPP
#define _CONLINE_INTERPOLATION_HPP


#include <unordered_map>
#include <vector>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>

namespace BRTServices
{
	class CMidPointOnlineInterpolator {
	
	public:

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
		template <typename T, typename U, typename Functor>
		U CalculateTF_OnlineMethod(const T& resampledTable, int32_t numberOfSubfilters, int32_t subfilterLength, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, Functor f) const
		{
			U data;

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
					data = CalculateTF_BarycentricInterpolation<T, U>(resampledTable, numberOfSubfilters, subfilterLength, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoA, orientation_ptoB, orientation_ptoD, orientation_ptoC, f);
				}
				else if (_elevation < orientation_ptoP.elevation)
				{
					//Forth quadrant
					data = CalculateTF_BarycentricInterpolation<T, U>(resampledTable, numberOfSubfilters, subfilterLength, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoA, f);
				}
			}
			else if (_azimuth < orientation_ptoP.azimuth)
			{
				if (_elevation >= orientation_ptoP.elevation)
				{
					//First quadrant
					data = CalculateTF_BarycentricInterpolation<T, U>(resampledTable, numberOfSubfilters, subfilterLength, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, f);
				}
				else if (_elevation < orientation_ptoP.elevation) {
					//Third quadrant
					data = CalculateTF_BarycentricInterpolation<T, U>(resampledTable, numberOfSubfilters, subfilterLength, _azimuth, _elevation,
						eleCeil, eleFloor, orientation_ptoA, orientation_ptoC, orientation_ptoD, orientation_ptoB, f);
				}
			}
			//SET_RESULT(RESULT_OK, "GetHRIR_partitioned_InterpolationMethod completed succesfully");
			return data;

		}

	private:

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
		template <typename T, typename U, typename Functor>
		U CalculateTF_BarycentricInterpolation(const T& resampledTable, int32_t numberOfSubfilters, int32_t subfilterLength,
			float _azimuth, float _elevation, float elevationCeil, float elevationFloor, orientation point1, orientation point2, orientation point3, orientation point4, Functor f) const
		{
			U data;
			TBarycentricCoordinatesStruct barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, point1.azimuth, point1.elevation, point2.azimuth, point2.elevation, point3.azimuth, point3.elevation);
			
			if (barycentricCoordinates.alpha < 0 || barycentricCoordinates.beta < 0 || barycentricCoordinates.gamma < 0) {
				barycentricCoordinates = Check_Triangles_Left(_azimuth, _elevation, point1, point2, point3, point4);
			}

			if (elevationCeil == ELEVATION_NORTH_POLE) { point2.azimuth = DEFAULT_MIN_AZIMUTH; }
			else if (elevationFloor == ELEVATION_SOUTH_POLE) { point3.azimuth = DEFAULT_MIN_AZIMUTH; }

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (point1.azimuth == DEFAULT_MAX_AZIMUTH) { point1.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (point2.azimuth == DEFAULT_MAX_AZIMUTH) { point2.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (point3.azimuth == DEFAULT_MAX_AZIMUTH) { point3.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (point1.elevation == DEFAULT_MAX_ELEVATION) { point1.elevation = DEFAULT_MIN_ELEVATION; }
				if (point2.elevation == DEFAULT_MAX_ELEVATION) { point2.elevation = DEFAULT_MIN_ELEVATION; }
				if (point3.elevation == DEFAULT_MAX_ELEVATION) { point3.elevation = DEFAULT_MIN_ELEVATION; }

				data = f(resampledTable, numberOfSubfilters, subfilterLength, barycentricCoordinates, point1, point2, point3);

			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateTF_BarycentricInterpolation()");
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

			eleCeil = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(eleCeil);				//			   Back	  Front
			eleFloor = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(eleFloor);				//	Ceil		A		B

			auto stepItr = stepMap.find(orientation(0, eleCeil));													//	Floor		C		D
			float aziStepCeil = stepItr->second;

			CInterpolationAuxiliarMethods::CalculateAzimuth_BackandFront(aziCeilBack, aziCeilFront, aziStepCeil, _azimuth);
			// azimuth values passed by reference

			auto stepIt = stepMap.find(orientation(0, eleFloor));
			float aziStepFloor = stepIt->second;

			CInterpolationAuxiliarMethods::CalculateAzimuth_BackandFront(aziFloorBack, aziFloorFront, aziStepFloor, _azimuth);

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
			barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, pnt1.azimuth, pnt1.elevation, pnt2.azimuth, pnt2.elevation, pnt4.azimuth, pnt4.elevation);
			if (barycentricCoordinates.alpha < 0 || barycentricCoordinates.beta < 0 || barycentricCoordinates.gamma < 0)
			{
				barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, pnt1.azimuth, pnt1.elevation, pnt3.azimuth, pnt3.elevation, pnt4.azimuth, pnt4.elevation);
				if (barycentricCoordinates.alpha < 0 || barycentricCoordinates.beta < 0 || barycentricCoordinates.gamma < 0)
				{
					barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, pnt2.azimuth, pnt2.elevation, pnt3.azimuth, pnt3.elevation, pnt4.azimuth, pnt4.elevation);
				}
			}
			return barycentricCoordinates;

		}

	};

	class CSlopesMethodOnlineInterpolator
	{
	public:

		///**
		// * @brief Calculate from resample table HRIR subfilters using a barycentric interpolation of the three nearest orientation.
		template <typename T, typename U, typename Functor>
		static U CalculateTF_OnlineMethod(const T& resampledTable, int32_t numberOfSubfilters, int32_t subfilterLength, float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, Functor f)
		{
			U data;
			TBarycentricCoordinatesStruct barycentricCoordinates;

			// Find four nearest points					
			orientation orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP;
			std::pair<float, float>nearestElevations;

			Find_4Nearest_Points(_azimuth, _elevation, stepMap, orientation_ptoA, orientation_ptoB, orientation_ptoC, orientation_ptoD, orientation_ptoP, nearestElevations);
			float eleCeil = nearestElevations.first;
			float eleFloor = nearestElevations.second;


			// SLOPE METHOD
			// First make the slope of 2 points, always the same 2 points, A->D			
			float slopeDiagonalTrapezoid = std::abs((orientation_ptoD.elevation - orientation_ptoA.elevation) / (orientation_ptoD.azimuth - orientation_ptoA.azimuth));
			float slopeOrientationOfInterest = std::abs((_elevation - orientation_ptoA.elevation) / (_azimuth - orientation_ptoA.azimuth));

			if (slopeOrientationOfInterest >= slopeDiagonalTrapezoid)
			{
				// Uses A,C,D
				data = CalculateTF_BarycentricInterpolation<T,U>(resampledTable, numberOfSubfilters, subfilterLength, _azimuth, _elevation,
					eleCeil, eleFloor, orientation_ptoA, orientation_ptoC, orientation_ptoD, orientation_ptoB, f);
			}
			else
			{
				//Uses A,B,D
				data = CalculateTF_BarycentricInterpolation<T,U>(resampledTable, numberOfSubfilters, subfilterLength, _azimuth, _elevation,
					eleCeil, eleFloor, orientation_ptoA, orientation_ptoB, orientation_ptoD, orientation_ptoC, f);

			}

			////SET_RESULT(RESULT_OK, "GetHRIR_partitioned_InterpolationMethod completed succesfully");
			return data;

		}

	
	private:
			

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
		template <typename T, typename U, typename Functor>
		static U CalculateTF_BarycentricInterpolation(const T& resampledTable, int32_t numberOfSubfilters, int32_t subfilterLength,
			float _azimuth, float _elevation, float elevationCeil, float elevationFloor, orientation point1, orientation point2, orientation point3, orientation point4, Functor f)
		{
			U data;
			TBarycentricCoordinatesStruct barycentricCoordinates = CInterpolationAuxiliarMethods::GetBarycentricCoordinates(_azimuth, _elevation, point1.azimuth, point1.elevation, point2.azimuth, point2.elevation, point3.azimuth, point3.elevation);

			if (elevationCeil == ELEVATION_NORTH_POLE) { point2.azimuth = DEFAULT_MIN_AZIMUTH; }
			else if (elevationFloor == ELEVATION_SOUTH_POLE) { point3.azimuth = DEFAULT_MIN_AZIMUTH; }

			if (barycentricCoordinates.alpha >= 0.0f && barycentricCoordinates.beta >= 0.0f && barycentricCoordinates.gamma >= 0.0f)
			{
				// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
				if (Common::AreSame(point1.azimuth, DEFAULT_MAX_AZIMUTH, EPSILON_SEWING)) { point1.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (Common::AreSame(point2.azimuth, DEFAULT_MAX_AZIMUTH, EPSILON_SEWING)) { point2.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (Common::AreSame(point3.azimuth, DEFAULT_MAX_AZIMUTH, EPSILON_SEWING)) { point3.azimuth = DEFAULT_MIN_AZIMUTH; }
				if (Common::AreSame(point1.elevation, DEFAULT_MAX_ELEVATION, EPSILON_SEWING)) { point1.elevation = DEFAULT_MIN_ELEVATION; }
				if (Common::AreSame(point2.elevation, DEFAULT_MAX_ELEVATION, EPSILON_SEWING)) { point2.elevation = DEFAULT_MIN_ELEVATION; }
				if (Common::AreSame(point3.elevation, DEFAULT_MAX_ELEVATION, EPSILON_SEWING)) { point3.elevation = DEFAULT_MIN_ELEVATION; }

				data = f(resampledTable, numberOfSubfilters, subfilterLength,  barycentricCoordinates, point1, point2, point3);

			}
			else {
				SET_RESULT(RESULT_WARNING, "No Barycentric coordinates Triangle in CalculateTF_BarycentricInterpolation()");
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
		static void Find_4Nearest_Points(float _azimuth, float _elevation, std::unordered_map<orientation, float> stepMap, orientation& orientation_ptoA, orientation& orientation_ptoB, orientation& orientation_ptoC, orientation& orientation_ptoD, orientation& orientation_ptoP, std::pair<float, float>& nearestElevations)
		{
			float azimuthCeilBack, azimuthCeilFront, azimuthFloorBack, azimuthFloorFront;
			float azimuthStepCeil, azimuthStepFloor;

			float elevationStep = stepMap.find(orientation(-1, -1))->second; // Elevation Step -- Same always
			int indexElevation = ceil(_elevation / elevationStep);
			float elevationCeil = elevationStep * indexElevation;
			float elevationFloor = elevationStep * (indexElevation - 1);

			elevationCeil = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(elevationCeil);				//			   Back	  Front
			elevationFloor = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(elevationFloor);				//	Ceil		A		B

			auto stepItr = stepMap.find(orientation(0, elevationCeil));															//	Floor		C		D
			if(stepItr!= stepMap.end()){ azimuthStepCeil = stepItr->second; }
			else { SET_RESULT(RESULT_ERROR_NOTSET, "OrientationCeil not found in the ONline interpolation (Find4Nearest algorithm)"); }
			
			CInterpolationAuxiliarMethods::CalculateAzimuth_BackandFront(azimuthCeilBack, azimuthCeilFront, azimuthStepCeil, _azimuth);

			auto stepIt = stepMap.find(orientation(0, elevationFloor));
			if (stepItr != stepMap.end()) { azimuthStepFloor = stepIt->second;	}
			else { SET_RESULT(RESULT_ERROR_NOTSET, "OrientationFloor not found in the ONline interpolation (Find4Nearest algorithm)"); }
			
			CInterpolationAuxiliarMethods::CalculateAzimuth_BackandFront(azimuthFloorBack, azimuthFloorFront, azimuthStepFloor, _azimuth);

			elevationCeil = elevationStep * indexElevation;

			// Mid Point of a trapezoid can be compute by averaging all azimuths
			float azimuth_ptoP = (azimuthCeilBack + azimuthCeilFront + azimuthFloorBack + azimuthFloorFront) * 0.25;
			// to avoid take points above under 0 like 345,350,355 and compare with them
			float elevation_ptoP = (elevationCeil - elevationStep * 0.5f);

			orientation_ptoP = orientation(azimuth_ptoP, elevation_ptoP);

			// Particular case of points near poles
			if (elevationCeil == ELEVATION_NORTH_POLE) { azimuthCeilFront = azimuthFloorFront; }
			else if (elevationFloor == ELEVATION_SOUTH_POLE) { azimuthFloorFront = azimuthCeilFront; }

			//Calculate the quadrant points A, B, C and D and the middle quadrant point P
			orientation_ptoC.azimuth = azimuthFloorBack;
			orientation_ptoC.elevation = elevationFloor;
			orientation_ptoA.azimuth = azimuthCeilBack;
			orientation_ptoA.elevation = elevationCeil;
			orientation_ptoB.azimuth = azimuthCeilFront;
			orientation_ptoB.elevation = elevationCeil;
			orientation_ptoD.azimuth = azimuthFloorFront;
			orientation_ptoD.elevation = elevationFloor;

			nearestElevations = std::pair(elevationCeil, elevationFloor);
		}

	};
}
#endif
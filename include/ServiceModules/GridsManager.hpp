/**
* \class CGridManagerInterface, CAngularBasedDistribution, CQuasiUniformSphereDistribution
*
* \brief Declaration of CGridManagerInterface, CAngularBasedDistribution, CQuasiUniformSphereDistribution classes interface
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


#ifndef _CGRIDS_MANAGER_HPP
#define _CGRIDS_MANAGER_HPP

#include <unordered_map>
#include <vector>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>

namespace BRTServices
{

	/*class CGridManagerInterface {
	public:
		virtual void CreateGrid(T_HRTFPartitionedTable& table, std::unordered_map<orientation, float>& stepVector, int _resamplingStep) = 0;
		virtual void FindNearestHRIR(const T_HRTFPartitionedTable& table, std::vector<CMonoBuffer<float>>& newHRIR, const std::unordered_map<orientation, float>& stepMap, Common::T_ear ear, float _azimuth, float _elevation, int resamplingStep)const = 0;
		virtual void FindNearestDelay(const T_HRTFPartitionedTable& table, float& HRIR_delay, const std::unordered_map<orientation, float >& stepMap, Common::T_ear ear, float _azimuthCenter, float _elevationCenter, int resamplingStep)const = 0;
		friend class CHRTFTester;
	};*/

	//class CAngularBasedDistribution :public CGridManagerInterface {
	
	template <typename T>
	class CAngularBasedDistribution  {
	public:
		void CreateGrid(T_HRTFPartitionedTable& table, std::unordered_map<orientation, float>& stepVector, int _resamplingStep) {}

		void FindNearestHRIR(const T_HRTFPartitionedTable& table, std::vector<CMonoBuffer<float>>& newHRIR, const std::unordered_map<orientation, float>& stepMap, Common::T_ear ear, float _azimuth, float _elevation, int resamplingStep)const
		{
			int nearestAzimuth = static_cast<int>(round(_azimuth / resamplingStep) * resamplingStep);
			int nearestElevation = static_cast<int>(round(_elevation / resamplingStep) * resamplingStep);
			// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
			if (nearestAzimuth == DEFAULT_MAX_AZIMUTH) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }
			if (nearestElevation == DEFAULT_MAX_ELEVATION) { nearestElevation = DEFAULT_MIN_ELEVATION; }
			// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
			if ((nearestElevation == CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north)) || (nearestElevation == CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south))) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }
			auto it = table.find(orientation(nearestAzimuth, nearestElevation));
			if (it != table.end())
			{
				if (ear == Common::T_ear::LEFT)
				{
					newHRIR = it->second.leftHRIR_Partitioned;
				}
				else
				{
					newHRIR = it->second.rightHRIR_Partitioned;
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRIR not found");
			}
		}

		void FindNearestDelay(const T_HRTFPartitionedTable& table, float& HRIR_delay, const std::unordered_map<orientation, float >& stepMap, Common::T_ear ear, float _azimuthCenter, float _elevationCenter, int resamplingStep)const
		{

			int nearestAzimuth = static_cast<int>(round(_azimuthCenter / resamplingStep) * resamplingStep);
			int nearestElevation = static_cast<int>(round(_elevationCenter / resamplingStep) * resamplingStep);

			// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
			if (nearestAzimuth == DEFAULT_MAX_AZIMUTH) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }
			if (nearestElevation == DEFAULT_MAX_ELEVATION) { nearestElevation = DEFAULT_MIN_ELEVATION; }
			// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
			if ((nearestElevation == CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north)) || (nearestElevation == CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south))) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }

			auto it = table.find(orientation(nearestAzimuth, nearestElevation));
			if (it != table.end())
			{
				if (ear == Common::T_ear::LEFT)
				{
					HRIR_delay = it->second.leftDelay;
				}
				else
				{
					HRIR_delay = it->second.rightDelay;
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: HRIR not found");
			}
		}
	
		friend class CHRTFTester;
	};

	//class CQuasiUniformSphereDistribution : public CGridManagerInterface {
	
	class CQuasiUniformSphereDistribution {
	public:
		template <typename T, typename U>
		static void CreateGrid(T& table, std::unordered_map<orientation, float>& stepVector, int _resamplingStep) {
			int n_divisions_by_elev;

			double elevationInRange;
			double actual_Azi_Step;
			
			U emptyData;

			int n_divisions = std::ceil(360.0f / _resamplingStep);
			int n_rings_hemisphere = std::ceil(90.0f / _resamplingStep);
			float actual_Ele_Step = 90.0f / n_rings_hemisphere;
			
			// Saving in -1,-1 the Elevation Step, same for all grid
			stepVector.emplace(orientation(-1, -1), actual_Ele_Step);

			// Round newElevation to avoid not saving elevation 90 due to float adding problems
			for (double newElevation = -90.0f; round(newElevation) <= 90.0f; newElevation = newElevation + actual_Ele_Step)
			{
				double temp_elevation = newElevation;
				if (Common::AreSameDouble(newElevation,0.0, EPSILON_SEWING)) {
					temp_elevation = 0.0;
				}
				float a = d2r(temp_elevation);
				float b = cos(a);
				float c = (n_divisions * b);

				n_divisions_by_elev = std::ceil(n_divisions * std::cos(d2r(temp_elevation)));
				if (Common::AreSame(n_divisions_by_elev, 0.0f, EPSILON_SEWING)) { 
					n_divisions_by_elev = 1.0f;
				}
				actual_Azi_Step = 360.0f / n_divisions_by_elev;
			
				// Calculate new Elevation to be in range [270,360] and use it to create the vector and to emplace data
				elevationInRange = AdjustElevationRange(temp_elevation);

				// Create the vector
				stepVector.emplace(orientation(0, elevationInRange), actual_Azi_Step);

				// Ceil to avoid error with the sum of decimal digits and not emplace 360 azimuth
				for (double newAzimuth = DEFAULT_MIN_AZIMUTH; std::ceil(newAzimuth) < DEFAULT_MAX_AZIMUTH; newAzimuth = newAzimuth + actual_Azi_Step)
				{
					table.emplace(orientation(newAzimuth, elevationInRange), emptyData);
				}
			}					
		}

		template <typename T, typename U>
		static U FindNearest(const T& table, const std::unordered_map<orientation, float>& stepMap, /*Common::T_ear ear,*/ float _azimuth, float _elevation)
		{
			U emptyData;
			float eleStep = stepMap.find(orientation(-1, -1))->second;

			float nearestElevation = (round(_elevation / eleStep) * eleStep);

			nearestElevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(nearestElevation);

			auto nearestElevationStep = stepMap.find(orientation(0, nearestElevation));
			if (nearestElevationStep == stepMap.end()) {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Error rounding the elevation looking in the GRID, this should not happen, it is a coding error.");
				return emptyData;
			}			
			float aziStep = nearestElevationStep->second;

			float nearestAzimuth = (round(_azimuth / aziStep) * aziStep);

			// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
			if (nearestAzimuth == DEFAULT_MAX_AZIMUTH) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }
			if (nearestElevation == DEFAULT_MAX_ELEVATION) { nearestElevation = DEFAULT_MIN_ELEVATION; }
			// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
			if ((nearestElevation == CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north)) || (nearestElevation == CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south))) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }

			auto it = table.find(orientation(nearestAzimuth, nearestElevation));
			if (it != table.end())
			{				
				return it->second;
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "Not found a TF close to the azimuth and elevation given in the GRID, this should not happen, it is a coding error.");				
				return emptyData;
			}

		}

		//template <typename T>
		//void FindNearestDelay(const T& table, float& HRIR_delay, const std::unordered_map<orientation, float>& stepMap, Common::T_ear ear, float _azimuthCenter, float _elevationCenter, int resamplingStep = 0) const
		//{
		//	float eleStep = stepMap.find(orientation(-1, -1))->second;

		//	float nearestElevation = (round(_elevationCenter / eleStep) * eleStep);

		//	nearestElevation = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(nearestElevation);

		//	float aziStep = stepMap.find(orientation(0, nearestElevation))->second;

		//	float nearestAzimuth = (round(_azimuthCenter / aziStep) * aziStep);

		//	// HRTF table does not contain data for azimuth = 360, which has the same values as azimuth = 0, for every elevation
		//	if (nearestAzimuth == DEFAULT_MAX_AZIMUTH) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }
		//	if (nearestElevation == DEFAULT_MAX_ELEVATION) { nearestElevation = DEFAULT_MIN_ELEVATION; }
		//	// When elevation is 90 or 270 degrees, the HRIR value is the same one for every azimuth
		//	if ((nearestElevation == CHRTFAuxiliarMethods::GetPoleElevation(TPole::north)) || (nearestElevation == CHRTFAuxiliarMethods::GetPoleElevation(TPole::south))) { nearestAzimuth = DEFAULT_MIN_AZIMUTH; }

		//	auto it = table.find(orientation(nearestAzimuth, nearestElevation));
		//	if (it != table.end())
		//	{
		//		if (ear == Common::T_ear::LEFT)
		//		{
		//			HRIR_delay = it->second.leftDelay;
		//		}
		//		else
		//		{
		//			HRIR_delay = it->second.rightDelay;
		//		}

		//	}
		//	else
		//	{
		//		SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRIR not found");
		//	}

		//};

		friend class CHRTFTester;
	private:
		static double AdjustElevationRange(double elev) {

			if (elev < 0) { 
				elev = elev + 360; 
			}
			return elev;
		}

		static double d2r(double d) {
			return (d / 180.0) * ((double)M_PI);
		}
	};

}
#endif

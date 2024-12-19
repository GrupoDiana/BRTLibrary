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


#ifndef _CHRTF_AUXILIARMETHODS_HPP
#define _CHRTF_AUXILIARMETHODS_HPP

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
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>
#include <ServiceModules/GridsManager.hpp>
#include <ServiceModules/OnlineInterpolation.hpp>


namespace BRTServices {

	/** \brief Auxiliary methods used in different classes working with HRTFs
	*/
	class CHRTFAuxiliarMethods {
	public:		

		/**
		 * @brief Get interpolated and partitioned HRIR buffer for one ear, without delay
		 * @param table Table with the HRIR data
		 * @param ear ear for which ear we want to get the HRIR
		 * @param _azimuth azimuth angle in degrees
		 * @param _elevation elevation angle in degrees
		 * @param runTimeInterpolation switch run-time interpolation
		 * @param _numberOfSubfilters number of subfilters in which the HRIR is divided
		 * @param _subfilterLength subfilter length
		 * @param stepVector steps of the oofline interpolation grid
		 * @return HRIR interpolated buffer for specified ear  without delay
		 *   \eh On error, an error code is reported to the error handler.
		 *       Warnings may be reported to the error handler.
		 */
		static const std::vector<CMonoBuffer<float>> GetHRIRFromPartitionedTable(const T_HRTFPartitionedTable& table, Common::T_ear ear, float _azimuth, float _elevation,
			bool runTimeInterpolation, int32_t _numberOfSubfilters, int32_t _subfilterLength, std::unordered_map<orientation, float> stepVector)
		{

			float sphereBorder = SPHERE_BORDER;
			float epsilon_sewing = EPSILON_SEWING;
			float azimuthMin = DEFAULT_MIN_AZIMUTH;
			float elevationMin = DEFAULT_MIN_ELEVATION;
			float elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			float elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

			std::vector<CMonoBuffer<float>> newHRIR;
						

			if (!runTimeInterpolation) {
				THRIRPartitionedStruct temp = CQuasiUniformSphereDistribution::FindNearest<T_HRTFPartitionedTable, THRIRPartitionedStruct>(table, stepVector, _azimuth, _elevation);

				if (ear == Common::T_ear::LEFT) { newHRIR = temp.leftHRIR_Partitioned; }
				else if (ear == Common::T_ear::RIGHT) { newHRIR = temp.rightHRIR_Partitioned; }
				else { SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get HRIR for a wrong ear (BOTH or NONE)"); }

				return newHRIR;
			}

			//  We have to do the run time interpolation -- (runTimeInterpolation = true)

			// Check if we are close to 360 azimuth or elevation and change to 0
			if (Common::AreSame(_azimuth, sphereBorder, epsilon_sewing)) { _azimuth = azimuthMin; }
			if (Common::AreSame(_elevation, sphereBorder, epsilon_sewing)) { _elevation = elevationMin; }

			// Check if we are at a pole
			int ielevation = static_cast<int>(round(_elevation));
			if ((ielevation == elevationNorth) || (ielevation == elevationSouth)) {
				GetPoleHRIRFromPartitionedTable(table, newHRIR, ear, ielevation, azimuthMin);
				return newHRIR;
			}

			// We search if the point already exists
			auto it = table.find(orientation(_azimuth, _elevation));
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
				return newHRIR;
			}

			// ONLINE Interpolation 	
			if (ear == Common::T_ear::LEFT) {
				const THRIRPartitionedStruct data = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<T_HRTFPartitionedTable, THRIRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CHRTFAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_LeftEar());
				return data.leftHRIR_Partitioned;
			}
			else
			{
				const THRIRPartitionedStruct data = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<T_HRTFPartitionedTable, THRIRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CHRTFAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_RightEar());
				return data.rightHRIR_Partitioned;
			}
		}

		/**
		 * @brief Get HRIR from a pole
		 * @param table Table with the HRIR data
		 * @param newHRIR
		 * @param ear
		 * @param ielevation
		 * @param azimuthMin
		 */
		static void GetPoleHRIRFromPartitionedTable(const T_HRTFPartitionedTable& table, std::vector<CMonoBuffer<float>>& newHRIR, Common::T_ear ear, int ielevation, float azimuthMin) {
			auto it = table.find(orientation(azimuthMin, ielevation));
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
				SET_RESULT(RESULT_WARNING, "Orientations in GetHRIR_partitioned() not found");
			}
		}


		static THRIRPartitionedStruct GetHRIRDelayFromPartitioned(const T_HRTFPartitionedTable& table, Common::T_ear ear, float _azimuthCenter, float _elevationCenter,
			bool runTimeInterpolation, int32_t _numberOfSubfilters, int32_t _subfilterLength, std::unordered_map<orientation, float> stepVector)
		{
			float sphereBorder = SPHERE_BORDER;
			float epsilon_sewing = EPSILON_SEWING;
			float azimuthMin = DEFAULT_MIN_AZIMUTH;
			float elevationMin = DEFAULT_MIN_ELEVATION;
			float elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			float elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);


			THRIRPartitionedStruct data;


			if (!runTimeInterpolation)
			{
				data = CQuasiUniformSphereDistribution::FindNearest<T_HRTFPartitionedTable, THRIRPartitionedStruct>(table, stepVector, _azimuthCenter, _elevationCenter);
				return data;
			}

			// Calculate Delay using ONLINE Interpolation 			

			// Check if we are close to 360 azimuth or elevation and change to 0
			if (Common::AreSame(_azimuthCenter, sphereBorder, epsilon_sewing)) { _azimuthCenter = 0.0f; }
			if (Common::AreSame(_elevationCenter, sphereBorder, epsilon_sewing)) { _elevationCenter = 0.0f; }

			// Check if we are at a pole
			int ielevation = static_cast<int>(round(_elevationCenter));
			if ((ielevation == elevationNorth) || (ielevation == elevationSouth))
			{
				float leftDelay;
				float rightDelay;
				GetPoleDelayFromHRIRPartitionedTable(table, leftDelay, Common::T_ear::LEFT, ielevation, azimuthMin);
				GetPoleDelayFromHRIRPartitionedTable(table, rightDelay, Common::T_ear::RIGHT, ielevation, azimuthMin);
				data.leftDelay = static_cast<uint64_t>(leftDelay);
				data.rightDelay = static_cast<uint64_t>(rightDelay);
				return data;
			}

			// We search if the point already exists
			auto it = table.find(orientation(_azimuthCenter, _elevationCenter));
			if (it != table.end())
			{
				THRIRPartitionedStruct temp;
				temp.leftDelay = it->second.leftDelay;;
				temp.rightDelay = it->second.rightDelay;
				return temp;
			}

			const THRIRPartitionedStruct temp = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<T_HRTFPartitionedTable, THRIRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuthCenter, _elevationCenter, stepVector, CHRTFAuxiliarMethods::CalculateDelay_FromBarycentricCoordinates());
			return temp;
		}


		static void GetPoleDelayFromHRIRPartitionedTable(const T_HRTFPartitionedTable& table, float& HRIR_delay, Common::T_ear ear, int ielevation, float azimuthMin)
		{
			//In the sphere poles the azimuth is always 0 degrees
			auto it = table.find(orientation(azimuthMin, ielevation));
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
				SET_RESULT(RESULT_WARNING, "Orientations in GetHRIRDelay() not found");
			}

		}


		/** \brief	Calculate the ITD value for a specific source
		*   \param [in]	_azimuth		source azimuth in degrees
		*   \param [in]	_elevation		source elevation in degrees
		*   \param [in]	ear				ear where the ITD is calculated (RIGHT, LEFT)
		*   \return ITD ITD calculated with the current listener head circunference
		*   \eh Nothing is reported to the error handler.
		*/
		static unsigned long CalculateCustomizedDelay(float _azimuth, float _elevation, Common::CCranialGeometry _cranialGeometry, Common::T_ear ear)
		{
			Common::CGlobalParameters globalParameters;
			float rAzimuth = _azimuth * PI / 180;
			float rElevation = _elevation * PI / 180;			

			//Calculate the customized delay
			unsigned long customizedDelay = 0;
			float interauralAzimuth = std::asin(std::sin(rAzimuth) * std::cos(rElevation));
			float ITD = CalculateITDFromHeadRadius(_cranialGeometry.GetHeadRadius(), interauralAzimuth, globalParameters.GetSoundSpeed());

			if ((ITD > 0 && ear == Common::T_ear::RIGHT) || (ITD < 0 && ear == Common::T_ear::LEFT)) {
				customizedDelay = static_cast <unsigned long> (round(std::abs(globalParameters.GetSampleRate() * ITD)));
			}
			return customizedDelay;
		}

		//Calculate the ITD using the Woodworth formula which depend on the interaural azimuth and the listener head radious
		//param		_headRadious		listener head radius, set by the App
		//param		_interauralAzimuth	source interaural azimuth
		//return	float				customizated ITD
		static float CalculateITDFromHeadRadius(float _headRadius, float _interauralAzimuth, float _soundSpeed) {
			//Calculate the ITD (from https://www.lpi.tel.uva.es/~nacho/docencia/ing_ond_1/trabajos_05_06/io5/public_html/ & http://interface.cipic.ucdavis.edu/sound/tutorial/psych.html)
			float ITD = _headRadius * (_interauralAzimuth + std::sin(_interauralAzimuth)) / _soundSpeed; //_azimuth in radians!
			return ITD;
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
		struct CalculatePartitionedHRIR_FromBarycentricCoordinates_LeftEar{
		const THRIRPartitionedStruct operator()(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)
		{
			THRIRPartitionedStruct data;
			
			// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
			auto it1 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
			auto it2 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
			auto it3 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

			if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
			{
				int subfilterLength = HRIR_partitioned_SubfilterLength;
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
			else {
				SET_RESULT(RESULT_WARNING, "Orientations in CalculatePartitionedHRIR_FromBarycentricCoordinates_LeftEar() not found");
			}

			return data;
		}
		};

		/**
		 * @brief Calculate HRIR subfilters using a barycentric coordinates of the three nearest orientation.
		 * @param ear
		 * @param barycentricCoordinates
		 * @param orientation_pto1
		 * @param orientation_pto2
		 * @param orientation_pto3
		 * @return
		*/
		struct CalculatePartitionedHRIR_FromBarycentricCoordinates_RightEar {
			const THRIRPartitionedStruct operator()(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)
			{
				THRIRPartitionedStruct data;

				// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
				auto it1 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(orientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{
					int subfilterLength = HRIR_partitioned_SubfilterLength;

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
					SET_RESULT(RESULT_WARNING, "Orientations in CalculatePartitionedHRIR_FromBarycentricCoordinates_RightEar() not found");
				}
				return data;
			}
		};

		/**
		 * @brief Calculate HRIR DELAY using a barycentric coordinates of the three nearest orientation, in number of samples
		 * @param ear
		 * @param barycentricCoordinates
		 * @param orientation_pto1
		 * @param orientation_pto2
		 * @param orientation_pto3
		 * @return
		*/
		struct CalculateDelay_FromBarycentricCoordinates{
		
			const THRIRPartitionedStruct operator()	(const T_HRTFPartitionedTable& t_HRTF_Resampled_partitioned, int32_t HRIR_partitioned_NumberOfSubfilters, int32_t HRIR_partitioned_SubfilterLength, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)
			{
				THRIRPartitionedStruct data;

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
					SET_RESULT(RESULT_WARNING, "Orientations in CalculateDelay_FromBarycentricCoordinates() not found");
				}
				return data;
			}
		};
		

		/// <summary>
		/// Calculate HRIR and delay from a given set of orientations		
		/// </summary>
		/// <param name="_t_HRTF_DataBase"></param>
		/// <param name="_HRIRLength"></param>
		/// <param name="_hemisphereParts"></param>
		/// <returns></returns>
		struct CalculateHRIRFromHemisphereParts{
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

		/// <summary>
		///  		
		/// </summary>
		/// <param name="_t_HRTF_DataBase"></param>
		/// <param name="_HRIRLength"></param>
		/// <returns></returns>
					
		struct CalculateHRIRFromBarycentrics_OfflineInterpolation {

			BRTServices::THRIRStruct operator () (const T_HRTFTable & _table, orientation _orientation1, orientation _orientation2, orientation _orientation3, int _HRIRLength, BRTServices::TBarycentricCoordinatesStruct barycentricCoordinates) {

				BRTServices::THRIRStruct calculatedHRIR;
				calculatedHRIR.leftHRIR.resize(_HRIRLength, 0.0f);
				calculatedHRIR.rightHRIR.resize(_HRIRLength, 0.0f);

				// Calculate the new HRIR with the barycentric coorfinates
				auto it1 = _table.find(_orientation1);
				auto it2 = _table.find(_orientation2);
				auto it3 = _table.find(_orientation3);

				if (it1 != _table.end() && it2 != _table.end() && it3 != _table.end()) {

					for (int i = 0; i < _HRIRLength; i++) {
						calculatedHRIR.leftHRIR[i] = barycentricCoordinates.alpha * it1->second.leftHRIR[i] + barycentricCoordinates.beta * it2->second.leftHRIR[i] + barycentricCoordinates.gamma * it3->second.leftHRIR[i];
						calculatedHRIR.rightHRIR[i] = barycentricCoordinates.alpha * it1->second.rightHRIR[i] + barycentricCoordinates.beta * it2->second.rightHRIR[i] + barycentricCoordinates.gamma * it3->second.rightHRIR[i];
					}

					// Calculate delay
					calculatedHRIR.leftDelay = barycentricCoordinates.alpha * it1->second.leftDelay + barycentricCoordinates.beta * it2->second.leftDelay + barycentricCoordinates.gamma * it3->second.leftDelay;
					calculatedHRIR.rightDelay = barycentricCoordinates.alpha * it1->second.rightDelay + barycentricCoordinates.beta * it2->second.rightDelay + barycentricCoordinates.gamma * it3->second.rightDelay;
					//SET_RESULT(RESULT_OK, "HRIR calculated with interpolation method succesfully");
					return calculatedHRIR;
				}

				else {
					SET_RESULT(RESULT_WARNING, "GetHRIR_InterpolationMethod return empty because HRIR with a specific orientation was not found");
					return calculatedHRIR;
				}

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
			THRIRStruct operator() (const T_HRTFTable& table, const std::vector<orientation>& orientationsList, int _HRIRSize, double _azimuth, double _elevation) {
				// Initialization
				//int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one
				THRIRStruct HRIRZeros;
				HRIRZeros.leftHRIR.resize(_HRIRSize, 0);
				HRIRZeros.rightHRIR.resize(_HRIRSize, 0);
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
			THRIRStruct operator() (const T_HRTFTable& table, const std::vector<orientation>& orientationsList, int _HRIRSize, double _azimuth, double _elevation) {
				// Order list of orientation
				std::vector<T_PairDistanceOrientation> pointsOrderedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(orientationsList, _azimuth, _elevation);
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

					//int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one					
					nearestHRIR.leftHRIR.resize(_HRIRSize, 0);
					nearestHRIR.rightHRIR.resize(_HRIRSize, 0);
				}

				return nearestHRIR;
			}
		};


		//	Split the input HRIR data in subfilters and get the FFT to apply the UPC algorithm
		//param	newData_time	HRIR value in time domain	

		struct SplitAndGetFFT_HRTFData{

			THRIRPartitionedStruct operator()(const THRIRStruct& newData_time, int _bufferSize, int _HRIRPartitioned_NumberOfSubfilters)
			{
				int blockSize = _bufferSize;
				int numberOfBlocks = _HRIRPartitioned_NumberOfSubfilters;
				int data_time_size = newData_time.leftHRIR.size();

				THRIRPartitionedStruct new_DataFFT_Partitioned;
				new_DataFFT_Partitioned.leftHRIR_Partitioned.reserve(numberOfBlocks);
				new_DataFFT_Partitioned.rightHRIR_Partitioned.reserve(numberOfBlocks);
				//Index to go throught the AIR values in time domain
				int index;
				for (int i = 0; i < data_time_size; i = i + blockSize)
				{
					CMonoBuffer<float> left_data_FFT_doubleSize, right_data_FFT_doubleSize;
					//Resize with double size and zeros to make the zero-padded demanded by the algorithm
					left_data_FFT_doubleSize.resize(blockSize * 2, 0.0f);
					right_data_FFT_doubleSize.resize(blockSize * 2, 0.0f);
					//Fill each AIR block (question about this comment: AIR???)
					for (int j = 0; j < blockSize; j++) {
						index = i + j;
						if (index < data_time_size) {
							left_data_FFT_doubleSize[j] = newData_time.leftHRIR[index];
							right_data_FFT_doubleSize[j] = newData_time.rightHRIR[index];
						}
					}
					//FFT
					CMonoBuffer<float> left_data_FFT, right_data_FFT;
					Common::CFFTCalculator::CalculateFFT(left_data_FFT_doubleSize, left_data_FFT);
					Common::CFFTCalculator::CalculateFFT(right_data_FFT_doubleSize, right_data_FFT);
					//Prepare struct to return the value
					new_DataFFT_Partitioned.leftHRIR_Partitioned.push_back(left_data_FFT);
					new_DataFFT_Partitioned.rightHRIR_Partitioned.push_back(right_data_FFT);
				}
				//Store the delays
				new_DataFFT_Partitioned.leftDelay = newData_time.leftDelay;
				new_DataFFT_Partitioned.rightDelay = newData_time.rightDelay;

				return new_DataFFT_Partitioned;
			}
		};		
	};
}
#endif
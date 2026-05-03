/**
* \class CFIRTableAuxiliarMethods
*
* \brief Declaration of CFIRTableAuxiliarMethods class interface
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


#ifndef _CFIR_TABLE_AUXILIAR_METHODS_HPP
#define _CFIR_TABLE_AUXILIAR_METHODS_HPP

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
#include <ServiceModules/SphericalFIRTableDefinitions.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>
#include <ServiceModules/GridsManager.hpp>
#include <ServiceModules/OnlineInterpolation.hpp>


namespace BRTServices {

	/** \brief Auxiliary methods used in different classes working with HRTFs
	*/
	class CFIRTableAuxiliarMethods {
	public:		


		/**
		 * @brief Calculate the windowing of the IR functions of the DataBase Table. 
		 * @param _inTable Input table with the IR data
		 * @param _outTable Output table with the windowed IR data
		 */
		static bool CalculateWindowingIRTable(const BRTServices::TRawSofaData & _inTable, BRTServices::TRawSofaData & _outTable,
			float fadeInBegin, float riseTime, float fadeOutCutoff, float fallTime, int _sampleRate) {

			_outTable.clear();
			_outTable = _inTable;

			bool anyActionDone = false;
			bool fadeInEnabled = (fadeInBegin != 0 || riseTime != 0);
			bool fadeOutEnabled = (fadeOutCutoff != 0 || fallTime != 0);

			if (fadeInEnabled) {
				for (auto it = _outTable.begin(); it != _outTable.end(); it++) {
					it->data.IR.left = Common::CIRWindowing::Process(it->data.IR.left, Common::CIRWindowing::fadein, fadeInBegin, riseTime, _sampleRate);
					it->data.IR.right = Common::CIRWindowing::Process(it->data.IR.right, Common::CIRWindowing::fadein, fadeInBegin, riseTime, _sampleRate);
				}
			}
			if (fadeOutEnabled) {
				for (auto it = _outTable.begin(); it != _outTable.end(); it++) {
					it->data.IR.left = Common::CIRWindowing::Process(it->data.IR.left, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, _sampleRate);
					it->data.IR.right = Common::CIRWindowing::Process(it->data.IR.right, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, _sampleRate);
				}
				anyActionDone = true;
			}
			// Update impulseResponseLength and the number of subfilters
			//impulseResponseLength = _outTable.begin()->data.IR.left.size();
			//partitionedFRNumberOfSubfilters = CalculateNumberOPartitions(impulseResponseLength);
			return anyActionDone;
		}

		/**
		 * @brief Calculate and remove the common delay of every IR functions of the DataBase Table. 
		 */
		static void RemoveCommonDelayFromTable(TRawSofaData & table) {
			//1. Init the minumun value with the fist value of the table
			auto it0 = table.begin();
			uint64_t minimumDelayLeft = it0->data.delay.left; //Vrbl to store the minumun delay value for left ear
			uint64_t minimumDelayRight = it0->data.delay.right; //Vrbl to store the minumun delay value for right ear

			//2. Find the common delay
			//Scan the whole table looking for the minimum delay for left and right ears
			for (auto it = table.begin(); it != table.end(); it++) {
				//Left ear
				if (it->data.delay.left < minimumDelayLeft) {
					minimumDelayLeft = it->data.delay.left;
				}
				//Right ear
				if (it->data.delay.right < minimumDelayRight) {
					minimumDelayRight = it->data.delay.right;
				}
			}
			//3. Delete the common delay
			//Scan the whole table substracting the common delay to every delays for both ears separately
			//The common delay of each canal have been calculated and subtracted separately in order to correct the asymmetry of the measurement
			if (minimumDelayRight != 0 || minimumDelayLeft != 0) {
				for (auto it = table.begin(); it != table.end(); it++) {
					it->data.delay.left -= minimumDelayLeft; //Left ear
					it->data.delay.right -= minimumDelayRight; //Right ear
				}
			}
			SET_RESULT(RESULT_OK, "Common delay deleted (" + std::to_string(minimumDelayLeft) + "," + std::to_string(minimumDelayRight) + ") from nonInterpolatedHRTF table succesfully");
		}

		

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
		static const TFRPartitions GetHRIRFromPartitionedTable(const TSphericalFIRTablePartitioned & table, Common::T_ear ear, float _azimuth, float _elevation,
			bool runTimeInterpolation, int32_t _numberOfSubfilters, int32_t _subfilterLength, std::unordered_map<TOrientation, float> stepVector)
		{

			float sphereBorder = SPHERE_BORDER;
			float epsilon_sewing = EPSILON_SEWING;
			float azimuthMin = DEFAULT_MIN_AZIMUTH;
			float elevationMin = DEFAULT_MIN_ELEVATION;
			float elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			float elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

			TFRPartitions newHRIR;
						

			if (!runTimeInterpolation) {
				TFRPartitionedStruct temp = CQuasiUniformSphereDistribution::FindNearest<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, stepVector, _azimuth, _elevation);

				if (ear == Common::T_ear::LEFT) { newHRIR = temp.IR.left; }
				else if (ear == Common::T_ear::RIGHT) { newHRIR = temp.IR.right; }
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
				Common::CEarPair<TFRPartitions> data;
				GetPoleHRIRFromPartitionedTable(table, data, ielevation, azimuthMin);
				if (ear == Common::T_ear::LEFT) {
					return data.left;
				}
				else {
					return data.right;
				}				
			}

			// We search if the point already exists
			auto it = table.find(TOrientation(_azimuth, _elevation));
			if (it != table.end())
			{
				if (ear == Common::T_ear::LEFT)
				{
					newHRIR = it->second.IR.left;
				}
				else
				{
					newHRIR = it->second.IR.right;
				}
				return newHRIR;
			}

			// ONLINE Interpolation 	
			if (ear == Common::T_ear::LEFT) {
				const TFRPartitionedStruct data = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CFIRTableAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_LeftEar());
				return data.IR.left;
			}
			else
			{
				const TFRPartitionedStruct data = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CFIRTableAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_RightEar());
				return data.IR.right;
			}
		}

		static const Common::CEarPair<TFRPartitions> GetHRIRFromPartitionedTable_2Ears(const TSphericalFIRTablePartitioned & table, float _azimuth, float _elevation,
								bool runTimeInterpolation, int32_t _numberOfSubfilters, int32_t _subfilterLength, std::unordered_map<TOrientation, float> stepVector) {

			float sphereBorder = SPHERE_BORDER;
			float epsilon_sewing = EPSILON_SEWING;
			float azimuthMin = DEFAULT_MIN_AZIMUTH;
			float elevationMin = DEFAULT_MIN_ELEVATION;
			float elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			float elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

			//std::vector<CMonoBuffer<float>> newHRIR;
			Common::CEarPair<TFRPartitions> data;

			if (!runTimeInterpolation) {								
				TFRPartitionedStruct temp = CQuasiUniformSphereDistribution::FindNearest<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, stepVector, _azimuth, _elevation);
				data = std::move(temp.IR);
				/*if (ear == Common::T_ear::LEFT) {
					newHRIR = temp.IR.left;
				} else if (ear == Common::T_ear::RIGHT) {
					newHRIR = temp.IR.right;
				} else {
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get HRIR for a wrong ear (BOTH or NONE)");
				}*/

				return data;
			}

			//  We have to do the run time interpolation -- (runTimeInterpolation = true)

			// Check if we are close to 360 azimuth or elevation and change to 0
			if (Common::AreSame(_azimuth, sphereBorder, epsilon_sewing)) {
				_azimuth = azimuthMin;
			}
			if (Common::AreSame(_elevation, sphereBorder, epsilon_sewing)) {
				_elevation = elevationMin;
			}

			// Check if we are at a pole
			int ielevation = static_cast<int>(round(_elevation));
			if ((ielevation == elevationNorth) || (ielevation == elevationSouth)) {
				GetPoleHRIRFromPartitionedTable(table, data, ielevation, azimuthMin);
				return data;
			}

			// We search if the point already exists
			auto it = table.find(TOrientation(_azimuth, _elevation));
			if (it != table.end()) {
				data = it->second.IR;
				/*if (ear == Common::T_ear::LEFT) {
					newHRIR = it->second.IR.left;
				} else {
					newHRIR = it->second.IR.right;
				}*/
				return data;
			}

			// ONLINE Interpolation
			//if (ear == Common::T_ear::LEFT) {
				//const TFRPartitionedStruct data = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CFIRTableAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_LeftEar());
			//	return data.IR.left;
			//} else {
				//const TFRPartitionedStruct data = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CFIRTableAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_RightEar());
				//return data.IR.right;
			//}
			TFRPartitionedStruct auxLeft = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CFIRTableAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_LeftEar());
			TFRPartitionedStruct auxRight = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuth, _elevation, stepVector, CFIRTableAuxiliarMethods::CalculatePartitionedHRIR_FromBarycentricCoordinates_RightEar());
			data.left = std::move(auxLeft.IR.left);
			data.right =std::move(auxRight.IR.right);
            return data; 
		}
		/**
		 * @brief Get HRIR from a pole
		 * @param table Table with the HRIR data
		 * @param newHRIR
		 * @param ear
		 * @param ielevation
		 * @param azimuthMin
		 */
		static void GetPoleHRIRFromPartitionedTable(const TSphericalFIRTablePartitioned & table, Common::CEarPair<TFRPartitions> & newHRIR, int ielevation, float azimuthMin) {
			auto it = table.find(TOrientation(azimuthMin, ielevation));
			if (it != table.end())
			{
				newHRIR = it->second.IR;
				/*if (ear == Common::T_ear::LEFT)
				{
					newHRIR = it->second.IR.left;
				}
				else
				{
					newHRIR = it->second.IR.right;
				}*/
			}
			else
			{
				SET_RESULT(RESULT_WARNING, "Orientations in GetHRIR_partitioned() not found");
			}
		}


		//static TFRPartitionedStruct GetHRIRDelayFromPartitioned(const TSphericalFIRTablePartitioned& table, Common::T_ear ear, float _azimuthCenter, float _elevationCenter,
		//	bool runTimeInterpolation, int32_t _numberOfSubfilters, int32_t _subfilterLength, std::unordered_map<TOrientation, float> stepVector)
		//{
		//	float sphereBorder = SPHERE_BORDER;
		//	float epsilon_sewing = EPSILON_SEWING;
		//	float azimuthMin = DEFAULT_MIN_AZIMUTH;
		//	float elevationMin = DEFAULT_MIN_ELEVATION;
		//	float elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
		//	float elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);


		//	TFRPartitionedStruct data;


		//	if (!runTimeInterpolation)
		//	{
		//		data = CQuasiUniformSphereDistribution::FindNearest<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, stepVector, _azimuthCenter, _elevationCenter);
		//		return data;
		//	}

		//	// Calculate Delay using ONLINE Interpolation 			

		//	// Check if we are close to 360 azimuth or elevation and change to 0
		//	if (Common::AreSame(_azimuthCenter, sphereBorder, epsilon_sewing)) { _azimuthCenter = 0.0f; }
		//	if (Common::AreSame(_elevationCenter, sphereBorder, epsilon_sewing)) { _elevationCenter = 0.0f; }

		//	// Check if we are at a pole
		//	int ielevation = static_cast<int>(round(_elevationCenter));
		//	if ((ielevation == elevationNorth) || (ielevation == elevationSouth))
		//	{
		//		float leftDelay;
		//		float rightDelay;
		//		GetPoleDelayFromHRIRPartitionedTable(table, leftDelay, Common::T_ear::LEFT, ielevation, azimuthMin);
		//		GetPoleDelayFromHRIRPartitionedTable(table, rightDelay, Common::T_ear::RIGHT, ielevation, azimuthMin);
		//		data.delay.left = static_cast<uint64_t>(leftDelay);
		//		data.delay.right = static_cast<uint64_t>(rightDelay);
		//		return data;
		//	}

		//	// We search if the point already exists
		//	auto it = table.find(TOrientation(_azimuthCenter, _elevationCenter));
		//	if (it != table.end())
		//	{
		//		TFRPartitionedStruct temp;
		//		temp.delay.left = it->second.delay.left;;
		//		temp.delay.right = it->second.delay.right;
		//		return temp;
		//	}

		//	const TFRPartitionedStruct temp = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuthCenter, _elevationCenter, stepVector, CFIRTableAuxiliarMethods::CalculateDelay_FromBarycentricCoordinates());
		//	return temp;
		//}
		//
		//static void GetPoleDelayFromHRIRPartitionedTable(const TSphericalFIRTablePartitioned& table, float& HRIR_delay, Common::T_ear ear, int ielevation, float azimuthMin)
		//{
		//	//In the sphere poles the azimuth is always 0 degrees
		//	auto it = table.find(TOrientation(azimuthMin, ielevation));
		//	if (it != table.end())
		//	{
		//		if (ear == Common::T_ear::LEFT)
		//		{
		//			HRIR_delay = it->second.delay.left;
		//		}
		//		else
		//		{
		//			HRIR_delay = it->second.delay.right;
		//		}
		//	}
		//	else
		//	{
		//		SET_RESULT(RESULT_WARNING, "Orientations in GetHRIRDelay() not found");
		//	}

		//}

		static const Common::CEarPair<uint64_t> GetHRIRDelayFromPartitioned_2Ears(const TSphericalFIRTablePartitioned & table, float _azimuthCenter, float _elevationCenter,
			bool runTimeInterpolation, int32_t _numberOfSubfilters, int32_t _subfilterLength, std::unordered_map<TOrientation, float> stepVector) {

			float sphereBorder = SPHERE_BORDER;
			float epsilon_sewing = EPSILON_SEWING;
			float azimuthMin = DEFAULT_MIN_AZIMUTH;
			float elevationMin = DEFAULT_MIN_ELEVATION;
			float elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			float elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

			Common::CEarPair<uint64_t> foundData;

			if (!runTimeInterpolation) {
				TFRPartitionedStruct aux = CQuasiUniformSphereDistribution::FindNearest<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, stepVector, _azimuthCenter, _elevationCenter);
				foundData.left = aux.delay.left;
				foundData.right = aux.delay.right;
				return foundData;
			}

			// Calculate Delay using ONLINE Interpolation

			// Check if we are close to 360 azimuth or elevation and change to 0
			if (Common::AreSame(_azimuthCenter, sphereBorder, epsilon_sewing)) {
				_azimuthCenter = 0.0f;
			}
			if (Common::AreSame(_elevationCenter, sphereBorder, epsilon_sewing)) {
				_elevationCenter = 0.0f;
			}

			// Check if we are at a pole
			int ielevation = static_cast<int>(round(_elevationCenter));
			if ((ielevation == elevationNorth) || (ielevation == elevationSouth)) {
				/*float leftDelay;
				float rightDelay;
				GetPoleDelayFromHRIRPartitionedTable(table, leftDelay, Common::T_ear::LEFT, ielevation, azimuthMin);
				GetPoleDelayFromHRIRPartitionedTable(table, rightDelay, Common::T_ear::RIGHT, ielevation, azimuthMin);
				data.delay.left = static_cast<uint64_t>(leftDelay);
				data.delay.right = static_cast<uint64_t>(rightDelay);*/
				return GetPoleDelayFromHRIRPartitionedTable_2Ears(table, ielevation, azimuthMin);				
			}

			// We search if the point already exists
			auto it = table.find(TOrientation(_azimuthCenter, _elevationCenter));
			if (it != table.end()) {
				/*TFRPartitionedStruct temp;
				temp.delay.left = it->second.delay.left;
				temp.delay.right = it->second.delay.right;*/
				foundData = it->second.delay;
				return foundData;
			}

			const TFRPartitionedStruct temp = CSlopesMethodOnlineInterpolator::CalculateTF_OnlineMethod<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(table, _numberOfSubfilters, _subfilterLength, _azimuthCenter, _elevationCenter, stepVector, CFIRTableAuxiliarMethods::CalculateDelay_FromBarycentricCoordinates());			
			return temp.delay;
		}

		static const Common::CEarPair<uint64_t> GetPoleDelayFromHRIRPartitionedTable_2Ears(const TSphericalFIRTablePartitioned & table, int ielevation, float azimuthMin) {
			
			Common::CEarPair<uint64_t> data;
			//In the sphere poles the azimuth is always 0 degrees
			auto it = table.find(TOrientation(azimuthMin, ielevation));
			if (it != table.end()) {
				data.left = it->second.delay.left;
				data.right = it->second.delay.right;				
			} else {
				SET_RESULT(RESULT_WARNING, "Orientations in GetHRIRDelay() not found");
			}
			return data;
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
		const TFRPartitionedStruct operator()(const TSphericalFIRTablePartitioned& t_HRTF_Resampled_partitioned, int32_t partitionedFRNumberOfSubfilters, int32_t partitionedFRSubfilterLength, TBarycentricCoordinatesStruct barycentricCoordinates, TOrientation orientation_pto1, TOrientation orientation_pto2, TOrientation orientation_pto3)
		{
			TFRPartitionedStruct data;
			
			// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
			auto it1 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto1.azimuth, orientation_pto1.elevation));
			auto it2 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto2.azimuth, orientation_pto2.elevation));
			auto it3 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto3.azimuth, orientation_pto3.elevation));

			if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
			{
				int subfilterLength = partitionedFRSubfilterLength;
				data.IR.left.resize(partitionedFRNumberOfSubfilters);

				for (int subfilterID = 0; subfilterID < partitionedFRNumberOfSubfilters; subfilterID++)
				{
					data.IR.left[subfilterID].resize(subfilterLength);
					for (int i = 0; i < subfilterLength; i++)
					{
						data.IR.left[subfilterID][i] = barycentricCoordinates.alpha * it1->second.IR.left[subfilterID][i] + barycentricCoordinates.beta * it2->second.IR.left[subfilterID][i] + barycentricCoordinates.gamma * it3->second.IR.left[subfilterID][i];
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
			const TFRPartitionedStruct operator()(const TSphericalFIRTablePartitioned& t_HRTF_Resampled_partitioned, int32_t partitionedFRNumberOfSubfilters, int32_t partitionedFRSubfilterLength, TBarycentricCoordinatesStruct barycentricCoordinates, TOrientation orientation_pto1, TOrientation orientation_pto2, TOrientation orientation_pto3)
			{
				TFRPartitionedStruct data;

				// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
				auto it1 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{
					int subfilterLength = partitionedFRSubfilterLength;

					data.IR.right.resize(partitionedFRNumberOfSubfilters);
					for (int subfilterID = 0; subfilterID < partitionedFRNumberOfSubfilters; subfilterID++)
					{
						data.IR.right[subfilterID].resize(subfilterLength, 0.0f);
						for (int i = 0; i < subfilterLength; i++)
						{
							data.IR.right[subfilterID][i] = barycentricCoordinates.alpha * it1->second.IR.right[subfilterID][i] + barycentricCoordinates.beta * it2->second.IR.right[subfilterID][i] + barycentricCoordinates.gamma * it3->second.IR.right[subfilterID][i];
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
		
			const TFRPartitionedStruct operator()	(const TSphericalFIRTablePartitioned& t_HRTF_Resampled_partitioned, int32_t partitionedFRNumberOfSubfilters, int32_t partitionedFRSubfilterLength, TBarycentricCoordinatesStruct barycentricCoordinates, TOrientation orientation_pto1, TOrientation orientation_pto2, TOrientation orientation_pto3)
			{
				TFRPartitionedStruct data;

				// Find the HRIR for the given t_HRTF_DataBase_ListOfOrientations
				auto it1 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto1.azimuth, orientation_pto1.elevation));
				auto it2 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto2.azimuth, orientation_pto2.elevation));
				auto it3 = t_HRTF_Resampled_partitioned.find(TOrientation(orientation_pto3.azimuth, orientation_pto3.elevation));

				if (it1 != t_HRTF_Resampled_partitioned.end() && it2 != t_HRTF_Resampled_partitioned.end() && it3 != t_HRTF_Resampled_partitioned.end())
				{
					data.delay.left = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.delay.left + barycentricCoordinates.beta * it2->second.delay.left + barycentricCoordinates.gamma * it3->second.delay.left));
					data.delay.right = static_cast <unsigned long> (round(barycentricCoordinates.alpha * it1->second.delay.right + barycentricCoordinates.beta * it2->second.delay.right + barycentricCoordinates.gamma * it3->second.delay.right));
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
			BRTServices::TIRStruct operator()(TRawSofaTable & _t_HRTF_DataBase, int _HRIRLength, std::vector<std::vector<TOrientation>> _hemisphereParts) {

				BRTServices::TIRStruct calculatedHRIR;

				//Calculate the delay and the HRIR of each hemisphere part
				float totalDelay_left = 0.0f;
				float totalDelay_right = 0.0f;

				std::vector< BRTServices::TIRStruct> newHRIR;
				newHRIR.resize(_hemisphereParts.size());

				for (int q = 0; q < _hemisphereParts.size(); q++)
				{
					newHRIR[q].IR.left.resize(_HRIRLength, 0.0f);
					newHRIR[q].IR.right.resize(_HRIRLength, 0.0f);

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
						auto itHRIR = _t_HRTF_DataBase.find(TOrientation(it->azimuth, it->elevation));

						//Get the delay
						newHRIR[q].delay.left = (newHRIR[q].delay.left + itHRIR->second.delay.left);
						newHRIR[q].delay.right = (newHRIR[q].delay.right + itHRIR->second.delay.right);

						//Get the HRIR
						for (int i = 0; i < _HRIRLength; i++) {
							newHRIR[q].IR.left[i] = (newHRIR[q].IR.left[i] + itHRIR->second.IR.left[i]);
							newHRIR[q].IR.right[i] = (newHRIR[q].IR.right[i] + itHRIR->second.IR.right[i]);
						}
					}//END loop hemisphere part

					 //Multiply by the factor (weighted sum)
					 // TODO: Use the previous loop to multiply by the factor
					 //Delay 
					totalDelay_left = totalDelay_left + (scaleFactor * newHRIR[q].delay.left);
					totalDelay_right = totalDelay_right + (scaleFactor * newHRIR[q].delay.right);
					//HRIR
					for (int i = 0; i < _HRIRLength; i++)
					{
						newHRIR[q].IR.left[i] = newHRIR[q].IR.left[i] * scaleFactor;
						newHRIR[q].IR.right[i] = newHRIR[q].IR.right[i] * scaleFactor;
					}
				}

				//Get the FINAL values
				float scaleFactor_final = 1.0f / _hemisphereParts.size();

				//Calculate Final delay
				calculatedHRIR.delay.left = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_left));
				calculatedHRIR.delay.right = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_right));

				//calculate Final HRIR
				calculatedHRIR.IR.left.resize(_HRIRLength, 0.0f);
				calculatedHRIR.IR.right.resize(_HRIRLength, 0.0f);

				for (int i = 0; i < _HRIRLength; i++)
				{
					for (int q = 0; q < _hemisphereParts.size(); q++)
					{
						calculatedHRIR.IR.left[i] = calculatedHRIR.IR.left[i] + newHRIR[q].IR.left[i];
						calculatedHRIR.IR.right[i] = calculatedHRIR.IR.right[i] + newHRIR[q].IR.right[i];
					}
				}
				for (int i = 0; i < _HRIRLength; i++)
				{
					calculatedHRIR.IR.left[i] = calculatedHRIR.IR.left[i] * scaleFactor_final;
					calculatedHRIR.IR.right[i] = calculatedHRIR.IR.right[i] * scaleFactor_final;
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

			BRTServices::TIRStruct operator()(const TRawSofaTable & _table, TOrientation _orientation1, TOrientation _orientation2, TOrientation _orientation3, int _HRIRLength, BRTServices::TBarycentricCoordinatesStruct barycentricCoordinates) {

				BRTServices::TIRStruct calculatedHRIR;
				calculatedHRIR.IR.left.resize(_HRIRLength, 0.0f);
				calculatedHRIR.IR.right.resize(_HRIRLength, 0.0f);

				// Calculate the new HRIR with the barycentric coorfinates
				auto it1 = _table.find(_orientation1);
				auto it2 = _table.find(_orientation2);
				auto it3 = _table.find(_orientation3);

				if (it1 != _table.end() && it2 != _table.end() && it3 != _table.end()) {

					for (int i = 0; i < _HRIRLength; i++) {
						calculatedHRIR.IR.left[i] = barycentricCoordinates.alpha * it1->second.IR.left[i] + barycentricCoordinates.beta * it2->second.IR.left[i] + barycentricCoordinates.gamma * it3->second.IR.left[i];
						calculatedHRIR.IR.right[i] = barycentricCoordinates.alpha * it1->second.IR.right[i] + barycentricCoordinates.beta * it2->second.IR.right[i] + barycentricCoordinates.gamma * it3->second.IR.right[i];
					}

					// Calculate delay
					calculatedHRIR.delay.left = barycentricCoordinates.alpha * it1->second.delay.left + barycentricCoordinates.beta * it2->second.delay.left + barycentricCoordinates.gamma * it3->second.delay.left;
					calculatedHRIR.delay.right = barycentricCoordinates.alpha * it1->second.delay.right + barycentricCoordinates.beta * it2->second.delay.right + barycentricCoordinates.gamma * it3->second.delay.right;
					//SET_RESULT(RESULT_OK, "HRIR calculated with interpolation method succesfully");
					return calculatedHRIR;
				}

				else {
					SET_RESULT(RESULT_WARNING, "GetHRIR_InterpolationMethod return empty because HRIR with a specific TOrientation was not found");
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
			TIRStruct operator()(const TRawSofaTable & table, const std::vector<TOrientation> & orientationsList, int _HRIRSize, double _azimuth, double _elevation) {
				// Initialization
				//int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one
				TIRStruct HRIRZeros;
				HRIRZeros.IR.left.resize(_HRIRSize, 0);
				HRIRZeros.IR.right.resize(_HRIRSize, 0);
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
			TIRStruct operator()(const TRawSofaTable & table, const std::vector<TOrientation> & orientationsList, int _HRIRSize, double _azimuth, double _elevation) {
				// Order list of orientation
				std::vector<T_PairDistanceOrientation> pointsOrderedByDistance = CInterpolationAuxiliarMethods::GetListOrderedDistancesToPoint(orientationsList, _azimuth, _elevation);
				// Get nearest
				double nearestAzimuth = pointsOrderedByDistance.begin()->second.azimuth;
				double nearestElevation = pointsOrderedByDistance.begin()->second.elevation;
				// Find nearest HRIR and copy
				TIRStruct nearestHRIR;

				auto it = table.find(TOrientation(nearestAzimuth, nearestElevation));
				if (it != table.end()) {
					nearestHRIR = it->second;
				}
				else {
					SET_RESULT(RESULT_WARNING, "No point close enough to make the extrapolation has been found, this must not happen.");

					//int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one					
					nearestHRIR.IR.left.resize(_HRIRSize, 0);
					nearestHRIR.IR.right.resize(_HRIRSize, 0);
				}

				return nearestHRIR;
			}
		};


		//	Split the input HRIR data in subfilters and get the FFT to apply the UPC algorithm
		//param	newData_time	HRIR value in time domain	

		struct SplitAndGetFFT_HRTFData{

			TFRPartitionedStruct operator()(const TIRStruct& newData_time, int _bufferSize, int _HRIRPartitioned_NumberOfSubfilters)
			{
				int blockSize = _bufferSize;
				int numberOfBlocks = _HRIRPartitioned_NumberOfSubfilters;
				int data_time_size = newData_time.IR.left.size();

				TFRPartitionedStruct new_DataFFT_Partitioned;
				new_DataFFT_Partitioned.IR.left.reserve(numberOfBlocks);
				new_DataFFT_Partitioned.IR.right.reserve(numberOfBlocks);
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
							left_data_FFT_doubleSize[j] = newData_time.IR.left[index];
							right_data_FFT_doubleSize[j] = newData_time.IR.right[index];
						}
					}
					//FFT
					CMonoBuffer<float> left_data_FFT, right_data_FFT;
					Common::CFFTCalculator::CalculateFFT(left_data_FFT_doubleSize, left_data_FFT);
					Common::CFFTCalculator::CalculateFFT(right_data_FFT_doubleSize, right_data_FFT);
					//Prepare struct to return the value
					new_DataFFT_Partitioned.IR.left.push_back(left_data_FFT);
					new_DataFFT_Partitioned.IR.right.push_back(right_data_FFT);
				}
				//Store the delays
				new_DataFFT_Partitioned.delay.left = newData_time.delay.left;
				new_DataFFT_Partitioned.delay.right = newData_time.delay.right;

				return new_DataFFT_Partitioned;
			}
		};

		struct SplitAndGetFFT_FRData {

			TFRPartitionedStruct operator()(const TIRStruct & newData_time, int _bufferSize, int _HRIRPartitioned_NumberOfSubfilters) {
				int blockSize = _bufferSize;
				int numberOfBlocks = _HRIRPartitioned_NumberOfSubfilters;
				int data_time_size = newData_time.IR.left.size();

				TFRPartitionedStruct new_DataFFT_Partitioned;
				new_DataFFT_Partitioned.IR.left.reserve(numberOfBlocks);
				new_DataFFT_Partitioned.IR.right.reserve(numberOfBlocks);
				new_DataFFT_Partitioned.orientation = newData_time.orientation;
				//Index to go throught the AIR values in time domain
				int index;
				for (int i = 0; i < data_time_size; i = i + blockSize) {
					CMonoBuffer<float> left_data_FFT_doubleSize, right_data_FFT_doubleSize;
					//Resize with double size and zeros to make the zero-padded demanded by the algorithm
					left_data_FFT_doubleSize.resize(blockSize * 2, 0.0f);
					right_data_FFT_doubleSize.resize(blockSize * 2, 0.0f);
					//Fill each AIR block (question about this comment: AIR???)
					for (int j = 0; j < blockSize; j++) {
						index = i + j;
						if (index < data_time_size) {
							left_data_FFT_doubleSize[j] = newData_time.IR.left[index];
							right_data_FFT_doubleSize[j] = newData_time.IR.right[index];
						}
					}
					//FFT
					CMonoBuffer<float> left_data_FFT, right_data_FFT;
					Common::CFFTCalculator::CalculateFFT(left_data_FFT_doubleSize, left_data_FFT);
					Common::CFFTCalculator::CalculateFFT(right_data_FFT_doubleSize, right_data_FFT);
					//Prepare struct to return the value
					new_DataFFT_Partitioned.IR.left.push_back(left_data_FFT);
					new_DataFFT_Partitioned.IR.right.push_back(right_data_FFT);
				}
				//Store the delays
				new_DataFFT_Partitioned.delay.left = newData_time.delay.left;
				new_DataFFT_Partitioned.delay.right = newData_time.delay.right;

				return new_DataFFT_Partitioned;
			}
		};		
	};
}
#endif

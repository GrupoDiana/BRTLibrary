/**
* \class CHRTF
*
* \brief Declaration of CHRTF class interface
* \date	June 2023
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


#ifndef _CHRTF_H_
#define _CHRTF_H_

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/Fprocessor.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRTFAuxiliarMethods.hpp>
#include <ServiceModules/OnlineInterpolation.hpp>
#include <ServiceModules/GridsManager.hpp>
#include <ServiceModules/Extrapolation.hpp>
#include <ServiceModules/OfflineInterpolation.hpp>
#include <ServiceModules/OfflineInterpolationAuxiliarMethods.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>


namespace BRTBase { class CListener; }

namespace BRTServices
{
	
	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CHRTF : public CServicesBase
	{
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CHRTF()
			:enableCustomizedITD{ false }, resamplingStep{ DEFAULT_RESAMPLING_STEP }, gapThreshold{ DEFAULT_GAP_THRESHOLD }, HRIRLength{ 0 }, fileName{ "" },
			HRTFLoaded{ false }, setupInProgress{ false }, distanceOfMeasurement{ DEFAULT_HRTF_MEASURED_DISTANCE }, headRadius{ DEFAULT_LISTENER_HEAD_RADIOUS }, leftEarLocalPosition{ Common::CVector3() }, rightEarLocalPosition{ Common::CVector3() },
			azimuthMin{ DEFAULT_MIN_AZIMUTH }, azimuthMax{ DEFAULT_MAX_AZIMUTH }, elevationMin{ DEFAULT_MIN_ELEVATION }, elevationMax{ DEFAULT_MAX_ELEVATION }, sphereBorder{ SPHERE_BORDER },
			epsilon_sewing{ EPSILON_SEWING }, samplingRate{ -1 }, elevationNorth{ 0 }, elevationSouth{ 0 }, bufferSize{ 0 }, extrapolationMethod{ TExtrapolationMethod::nearestPoint }
		{ }

		/** \brief Get size of each HRIR buffer
		*	\retval size number of samples of each HRIR buffer for one ear
		*   \eh Nothing is reported to the error handler.
		*/
		int32_t GetHRIRLength() const
		{
			return HRIRLength;
		}

		void SetResamplingStep(int _resamplingStep) {
			resamplingStep = _resamplingStep;
		}

		int GetResamplingStep() {
			return resamplingStep;
		}

		/** \brief Start a new HRTF configuration
		*	\param [in] _HRIRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _HRIRLength, float _distance, std::string _extrapolationMethod)
		{
			//if ((ownerListener != nullptr) && ownerListener->ownerCore!=nullptr)
			{
				//Update parameters			
				HRIRLength = _HRIRLength;
				distanceOfMeasurement = _distance;
				bufferSize = globalParameters.GetBufferSize();
				SetExtrapolationMethod(_extrapolationMethod);

				float partitions = (float)HRIRLength / (float)bufferSize;
				HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
				elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

				//Clear every table			
				t_HRTF_DataBase.clear();
				t_HRTF_Resampled_frequency.clear();
				t_HRTF_Resampled_partitioned.clear();

				//Change class state
				setupInProgress = true;
				HRTFLoaded = false;


				SET_RESULT(RESULT_OK, "HRTF Setup started");
			}
			/*else
			{
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "Error in HRTF Begin Setup: OwnerCore or OwnerListener are nullPtr");
			}	*/
		}

		/** \brief Set the full HRIR matrix.
		*	\param [in] newTable full table with all HRIR data
		*   \eh Nothing is reported to the error handler.
		*/
		void AddHRTFTable(T_HRTFTable&& newTable)
		{
			if (setupInProgress) {
				t_HRTF_DataBase = newTable;
			}
		}

		/** \brief Add a new HRIR to the HRTF table
		*	\param [in] azimuth azimuth angle in degrees
		*	\param [in] elevation elevation angle in degrees
		*	\param [in] newHRIR HRIR data for both ears
		*   \eh Warnings may be reported to the error handler.
		*/
		//void AddHRIR(float _azimuth, float _elevation, THRIRStruct&& newHRIR)
		//{
		//	if (setupInProgress) {
		//		Common::CVector3 cartessianPos;
		//		cartessianPos.SetFromAED(_azimuth, _elevation, GetHRTFDistanceOfMeasurement());
		//		auto returnValue = t_HRTF_DataBase.emplace(orientation(_azimuth, _elevation, cartessianPos), std::forward<THRIRStruct>(newHRIR));
		//		//Error handler
		//		if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
		//	}
		//}

		void AddHRIR(double _azimuth, double _elevation, THRIRStruct&& newHRIR)
		{			
			if (setupInProgress) {				
				_azimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_azimuth);
				_elevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
				//Common::CVector3 cartessianPos;
				//cartessianPos.SetFromAED(_azimuth, _elevation, GetHRTFDistanceOfMeasurement());
				//auto returnValue = t_HRTF_DataBase.emplace(orientation(_azimuth, _elevation, cartessianPos), std::forward<THRIRStruct>(newHRIR));				
				auto returnValue = t_HRTF_DataBase.emplace(orientation(_azimuth, _elevation), std::forward<THRIRStruct>(newHRIR));
				//Error handler
				if (!returnValue.second) { 
					SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
		}


		/** \brief Stop the HRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup()
		{
			if (setupInProgress) {
				if (!t_HRTF_DataBase.empty())
				{
					
					RemoveCommonDelay_HRTFDataBaseTable();				// Delete the common delay of every HRIR functions of the DataBase Table					
					t_HRTF_DataBase_ListOfOrientations = offlineInterpolation.CalculateListOfOrientations(t_HRTF_DataBase);
					CalculateExtrapolation();							// Make the extrapolation if it's needed
					// Preparation of table read from sofa file
					offlineInterpolation.CalculateTF_InPoles<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, HRIRLength, resamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromHemisphereParts());
					offlineInterpolation.CalculateTF_SphericalCaps<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, HRIRLength, gapThreshold, resamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());
					t_HRTF_DataBase_ListOfOrientations = offlineInterpolation.CalculateListOfOrientations(t_HRTF_DataBase);
					//Creation and filling of resampling HRTF table
					quasiUniformSphereDistribution.CreateGrid<T_HRTFPartitionedTable, THRIRPartitionedStruct>(t_HRTF_Resampled_partitioned, stepVector, resamplingStep);
					offlineInterpolation.FillResampledTable(t_HRTF_DataBase, t_HRTF_Resampled_partitioned, bufferSize, HRIRLength, HRIR_partitioned_NumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_HRTFData());


					//Setup values
					auto it = t_HRTF_Resampled_partitioned.begin();
					HRIR_partitioned_SubfilterLength = it->second.leftHRIR_Partitioned[0].size();
					setupInProgress = false;
					HRTFLoaded = true;

					SET_RESULT(RESULT_OK, "HRTF Matrix resample completed succesfully");
					return true;
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_HRTF_DataBase map has not been set");
				}
			}
			return false;
		}


		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableHRTFCustomizedITD() {
			enableCustomizedITD = true;
		}

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableHRTFCustomizedITD() {
			enableCustomizedITD = false;
		}

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsHRTFCustomizedITDEnabled()
		{
			return enableCustomizedITD;
		}
		

		/** \brief Get interpolated and partitioned HRIR buffer with Delay, for one ear
		*	\param [in] ear for which ear we want to get the HRIR
		*	\param [in] _azimuth azimuth angle in degrees
		*	\param [in] _elevation elevation angle in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		const std::vector<CMonoBuffer<float>> GetHRIR_partitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation) const
		{
			std::vector<CMonoBuffer<float>> newHRIR;
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get HRIR for a wrong ear (BOTH or NONE)");
				return newHRIR;
			}

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRTF Setup in progress return empty");
				return newHRIR;

			}
			if (!runTimeInterpolation) {				
				THRIRPartitionedStruct temp = quasiUniformSphereDistribution.FindNearest<T_HRTFPartitionedTable, THRIRPartitionedStruct>(t_HRTF_Resampled_partitioned, stepVector, _azimuth, _elevation);
				
				if (ear == Common::T_ear::LEFT)			{ newHRIR = temp.leftHRIR_Partitioned; }
				else if (ear == Common::T_ear::RIGHT)	{ newHRIR = temp.rightHRIR_Partitioned;}
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
				GetPoleHRIR_partitioned(newHRIR, ear, ielevation);
				return newHRIR;
			}
			
			// We search if the point already exists
			auto it = t_HRTF_Resampled_partitioned.find(orientation(_azimuth, _elevation));
			if (it != t_HRTF_Resampled_partitioned.end())
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
			//const THRIRPartitionedStruct data = midPointOnlineInterpolator.CalculateHRIRPartitioned_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation, stepVector);
			const THRIRPartitionedStruct data = slopesMethodOnlineInterpolator.CalculateHRIRPartitioned_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuth, _elevation, stepVector);
			if (ear == Common::T_ear::LEFT) {
				return data.leftHRIR_Partitioned;
			}
			else
			{
				return data.rightHRIR_Partitioned;
			}									
		}
		
		void GetPoleHRIR_partitioned(std::vector<CMonoBuffer<float>>& newHRIR, Common::T_ear ear, int ielevation) const {
			auto it = t_HRTF_Resampled_partitioned.find(orientation(azimuthMin, ielevation));
			if (it != t_HRTF_Resampled_partitioned.end())
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

		/** \brief Get the HRIR delay, in number of samples, for one ear
		*	\param [in] ear for which ear we want to get the HRIR
		*	\param [in] _azimuthCenter azimuth angle from the source and the listener head center in degrees
		*	\param [in] _elevationCenter elevation angle from the source and the listener head center in degrees
		*	\param [in] runTimeInterpolation switch run-time interpolation
		*	\retval HRIR interpolated buffer with delay for specified ear
		*   \eh On error, an error code is reported to the error handler.
		*       Warnings may be reported to the error handler.
		*/
		THRIRPartitionedStruct GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation)
		{
			//float HRIR_delay = 0.0f;
			THRIRPartitionedStruct data;

			/*if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetHRIRDelay: Attempt to get the delay of the HRIR for a wrong ear (BOTH or NONE)");
				return HRIR_delay;
			}*/

			if (setupInProgress)
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: HRTF Setup in progress return empty");
				return data;
			}

			//Modify delay if customized delay is activate
			if (enableCustomizedITD)
			{
				//HRIR_delay = GetCustomizedDelay(_azimuthCenter, _elevationCenter, ear);
				data.leftDelay = GetCustomizedDelay(_azimuthCenter, _elevationCenter, Common::T_ear::LEFT);
				data.rightDelay = GetCustomizedDelay(_azimuthCenter, _elevationCenter, Common::T_ear::RIGHT);
				return data;
			}
	
			if (!runTimeInterpolation)
			{
				//float leftDelay;
				//float rightDelay;				
				//quasiUniformSphereDistribution.FindNearestDelay(t_HRTF_Resampled_partitioned, leftDelay, stepVector, Common::T_ear::LEFT, _azimuthCenter, _elevationCenter);
				//quasiUniformSphereDistribution.FindNearestDelay(t_HRTF_Resampled_partitioned, rightDelay, stepVector, Common::T_ear::RIGHT, _azimuthCenter, _elevationCenter);
				data = quasiUniformSphereDistribution.FindNearest<T_HRTFPartitionedTable, THRIRPartitionedStruct>(t_HRTF_Resampled_partitioned, stepVector, _azimuthCenter, _elevationCenter);							

				//data.leftDelay	= static_cast<uint64_t>(temp.leftDelay);
				//data.rightDelay = static_cast<uint64_t>(temp.rightDelay);
				return data;				
			}
		
			// Check if we are close to 360 azimuth or elevation and change to 0
			if (Common::AreSame(_azimuthCenter, sphereBorder, epsilon_sewing)) { _azimuthCenter = 0.0f; }
			if (Common::AreSame(_elevationCenter, sphereBorder, epsilon_sewing)) { _elevationCenter = 0.0f; }

			// Check if we are at a pole
			int ielevation = static_cast<int>(round(_elevationCenter));
			if ((ielevation == elevationNorth) || (ielevation == elevationSouth))
			{
				float leftDelay;
				float rightDelay;
				GetPoleDelay_Partitioned(leftDelay, Common::T_ear::LEFT, ielevation);
				GetPoleDelay_Partitioned(rightDelay,Common::T_ear::RIGHT, ielevation);
				data.leftDelay = static_cast<uint64_t>(leftDelay);
				data.rightDelay = static_cast<uint64_t>(rightDelay);
				return data;
			}

			// We search if the point already exists
			auto it = t_HRTF_Resampled_partitioned.find(orientation(_azimuthCenter, _elevationCenter));
			if (it != t_HRTF_Resampled_partitioned.end())
			{
				THRIRPartitionedStruct temp;				
				temp.leftDelay = it->second.leftDelay;;						
				temp.rightDelay = it->second.rightDelay;			
				return temp;
			}

			// ONLINE Interpolation 
			//return GetHRIRDelayInterpolationMethod(ear, _azimuthCenter, _elevationCenter, resamplingStep, stepVector);	
			const THRIRPartitionedStruct temp = slopesMethodOnlineInterpolator.CalculateDelay_onlineMethod(t_HRTF_Resampled_partitioned, HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, ear, _azimuthCenter, _elevationCenter, stepVector);
			return temp;
		}

		void GetPoleDelay_Partitioned(float& HRIR_delay, Common::T_ear ear, int ielevation) const
		{
			//In the sphere poles the azimuth is always 0 degrees
			auto it = t_HRTF_Resampled_partitioned.find(orientation(azimuthMin, ielevation));
			if (it != t_HRTF_Resampled_partitioned.end())
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

		/** \brief	Get the number of subfilters (blocks) in which the HRIR has been partitioned
		*	\retval n Number of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetHRIRNumberOfSubfilters() const {
			return HRIR_partitioned_NumberOfSubfilters;
		}

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetHRIRSubfilterLength() const {
			return HRIR_partitioned_SubfilterLength;
		}

		/** \brief	Get if the HRTF has been loaded
		*	\retval isLoadead bool var that is true if the HRTF has been loaded
		*   \eh Nothing is reported to the error handler.
		*/		
		bool IsHRTFLoaded()
		{
			return HRTFLoaded;
		}

		/** \brief Get raw HRTF table
		*	\retval table raw HRTF table
		*   \eh Nothing is reported to the error handler.
		*/		
		const T_HRTFTable& GetRawHRTFTable() const
		{
			return t_HRTF_DataBase;
		}

		/** \brief	Calculate the ITD value for a specific source
		*   \param [in]	_azimuth		source azimuth in degrees
		*   \param [in]	_elevation		source elevation in degrees
		*   \param [in]	ear				ear where the ITD is calculated (RIGHT, LEFT)
		*   \return ITD ITD calculated with the current listener head circunference
		*   \eh Nothing is reported to the error handler.
		*/
		const unsigned long GetCustomizedDelay(float _azimuth, float _elevation, Common::T_ear ear)const
		//const unsigned long CHRTF::GetCustomizedDelay(float _azimuth, float _elevation, Common::T_ear ear)  const
		{

			float rAzimuth = _azimuth * PI / 180;
			float rElevation = _elevation * PI / 180;

			//Calculate the customized delay
			unsigned long customizedDelay = 0;
			float interauralAzimuth = std::asin(std::sin(rAzimuth) * std::cos(rElevation));

			float ITD = CalculateITDFromHeadRadius(headRadius /*ownerListener->GetHeadRadius()*/, interauralAzimuth);

			if ((ITD > 0 && ear == Common::T_ear::RIGHT) || (ITD < 0 && ear == Common::T_ear::LEFT)) {
				customizedDelay = static_cast <unsigned long> (round(std::abs(globalParameters.GetSampleRate() * ITD)));
			}
			return customizedDelay;
		}


		/** \brief	Get the distance where the HRTF has been measured
		*   \return distance of the speakers structure to calculate the HRTF
		*   \eh Nothing is reported to the error handler.
		*/		
		float GetHRTFDistanceOfMeasurement() {
			return distanceOfMeasurement;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetTitle(std::string _title) {
			title = _title;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetDatabaseName(std::string _databaseName) {
			databaseName = _databaseName;
		}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		void SetListenerShortName(std::string _listenerShortName) {
			listenerShortName = _listenerShortName;
		}


		/** \brief Set the name of the SOFA file 
		*    \param [in]	_fileName		string contains filename
		*/
		void SetFilename(std::string _fileName) {
			fileName = _fileName;
		}

		/** \brief Get the name of the SOFA file 
		*   \return string contains filename
		*/
		std::string GetFilename() { 
			return fileName;
		}

		/** \brief	Get the radius of the listener head
		*   \return listenerHeadRadius in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() {
			return headRadius;
		}

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition( Common::T_ear _ear, Common::CVector3 _earPosition) {
			if (_ear == Common::T_ear::LEFT)		{ leftEarLocalPosition = _earPosition; }
			else if (_ear == Common::T_ear::RIGHT)	{ rightEarLocalPosition = _earPosition; }
			else if (_ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
			}
		}

		/** \brief	Get the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) {
			if (enableCustomizedITD) {
				return CalculateEarLocalPositionFromHeadRadius(_ear);
			}
			else {
				if (_ear == Common::T_ear::LEFT)		{ return leftEarLocalPosition; }
				else if (_ear == Common::T_ear::RIGHT)	{ return rightEarLocalPosition; }
				else // either _ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE
				{
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
					return Common::CVector3();
				}
			}
		}

		/** \brief	Calculate the relative position of one ear taking into account the listener head radius
		*	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 CalculateEarLocalPositionFromHeadRadius(Common::T_ear ear) {
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}

			Common::CVector3 earLocalPosition = Common::CVector3::ZERO();
			if (ear == Common::T_ear::LEFT) {
				earLocalPosition.SetAxis(RIGHT_AXIS, -headRadius);
			}
			else
				earLocalPosition.SetAxis(RIGHT_AXIS, headRadius);

			return earLocalPosition;
		}

		/** \brief Set the sampling rate for the HRTF
		*	\param [in] sampling rate
		*/
		void SetSamplingRate(int _samplingRate) {
			samplingRate = _samplingRate;
		}

		/** \brief Ask for the sampling rate
		*	\retval sampling step
		*/
		int GetSamplingRate() {
			return samplingRate;
		}


	private:
		
		enum TExtrapolationMethod { zeroInsertion, nearestPoint };
		///////////////
		// ATTRIBUTES
		///////////////		
		int32_t HRIRLength;								// HRIR vector length
		int32_t bufferSize;								// Input signal buffer size		
		int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter
		float distanceOfMeasurement;					//Distance where the HRIR have been measurement
		float headRadius;								// Head radius of listener 
		Common::CVector3 leftEarLocalPosition;			// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;			// Listener right ear relative position
		TExtrapolationMethod extrapolationMethod;		// Methods that is going to be used to extrapolate

		float sphereBorder;								// Define spheere "sewing"
		float epsilon_sewing;

		float azimuthMin, azimuthMax, elevationMin, elevationMax, elevationNorth, elevationSouth;	// Variables that define limits of work area

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int resamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool enableCustomizedITD;					// Indicate the use of a customized delay
		int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap

		std::unordered_map<orientation, float> stepVector;		// Store hrtf interpolated grids steps

		std::string title;
		std::string databaseName;
		std::string listenerShortName;
		std::string fileName;
		int samplingRate;

		// HRTF tables			
		T_HRTFTable				t_HRTF_DataBase;
		std::vector<orientation> t_HRTF_DataBase_ListOfOrientations;
		
		T_HRTFTable				t_HRTF_Resampled_frequency;
		T_HRTFPartitionedTable	t_HRTF_Resampled_partitioned;
		
		// Empty object to return in some methods
		THRIRStruct						emptyHRIR;
		THRIRPartitionedStruct			emptyHRIR_partitioned;
		CMonoBuffer<float>				emptyMonoBuffer;
		oneEarHRIR_struct				emptyOneEarHRIR;
		TOneEarHRIRPartitionedStruct	emptyOneEarHRIR_partitioned;

		Common::CGlobalParameters globalParameters;

		// Processors
		CQuasiUniformSphereDistribution quasiUniformSphereDistribution;
		CSlopesMethodOnlineInterpolator slopesMethodOnlineInterpolator;
		COfflineInterpolation offlineInterpolation;
		CExtrapolation extrapolation;		

		friend class CHRTFTester;
				
		
		/////////////
		// METHODS
		/////////////

		// Calculate and remove the common delay of every HRIR functions of the DataBase Table. Off line Method, called from EndSetUp()		
		void RemoveCommonDelay_HRTFDataBaseTable()
		{
			//1. Init the minumun value with the fist value of the table
			auto it0 = t_HRTF_DataBase.begin();
			unsigned long minimumDelayLeft = it0->second.leftDelay;		//Vrbl to store the minumun delay value for left ear
			unsigned long minimumDelayRight = it0->second.rightDelay;	//Vrbl to store the minumun delay value for right ear

			//2. Find the common delay
			//Scan the whole table looking for the minimum delay for left and right ears
			for (auto it = t_HRTF_DataBase.begin(); it != t_HRTF_DataBase.end(); it++) {
				//Left ear
				if (it->second.leftDelay < minimumDelayLeft) {
					minimumDelayLeft = it->second.leftDelay;
				}
				//Right ear
				if (it->second.rightDelay < minimumDelayRight) {
					minimumDelayRight = it->second.rightDelay;
				}
			}

			//3. Delete the common delay
			//Scan the whole table substracting the common delay to every delays for both ears separately
			//The common delay of each canal have been calculated and subtracted separately in order to correct the asymmetry of the measurement
			if (minimumDelayRight != 0 || minimumDelayLeft != 0)
			{
				for (auto it = t_HRTF_DataBase.begin(); it != t_HRTF_DataBase.end(); it++)
				{
					it->second.leftDelay = it->second.leftDelay - minimumDelayLeft;		//Left ear
					it->second.rightDelay = it->second.rightDelay - minimumDelayRight;	//Right ear
				}
			}

			//SET_RESULT(RESULT_OK, "Common delay deleted from HRTF table succesfully");
		}

		//Calculate the ITD using the Lord Rayleight formula which depend on the interaural azimuth and the listener head radious
		//param		_headRadious		listener head radius, set by the App
		//param		_interauralAzimuth	source interaural azimuth
		//return	float				customizated ITD
		const  float CalculateITDFromHeadRadius(float _headRadius, float _interauralAzimuth)const{
			//Calculate the ITD (from https://www.lpi.tel.uva.es/~nacho/docencia/ing_ond_1/trabajos_05_06/io5/public_html/ & http://interface.cipic.ucdavis.edu/sound/tutorial/psych.html)
			float ITD = _headRadius * (_interauralAzimuth + std::sin(_interauralAzimuth)) / globalParameters.GetSoundSpeed(); //_azimuth in radians!
			return 0;// ITD;
		}
				
		
		/**
		 * @brief Set the extrapolation method that is going to be used
		 * @param _extrapolationMethod 
		*/
		void SetExtrapolationMethod(std::string _extrapolationMethod) {
		
			if (_extrapolationMethod == EXTRAPOLATION_METHOD_ZEROINSERTION_STRING) {
				extrapolationMethod = TExtrapolationMethod::zeroInsertion;
			}
			else if (_extrapolationMethod == EXTRAPOLATION_METHOD_NEARESTPOINT_STRING) {
				extrapolationMethod = TExtrapolationMethod::nearestPoint;
			}
			else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Extrapolation Method not identified.");
				extrapolationMethod = TExtrapolationMethod::nearestPoint;
			}

		}
		/**
		 * @brief Call the extrapolation method
		*/
		void CalculateExtrapolation() {
			// Select the one that extrapolates with zeros or the one that extrapolates based on the nearest point according to some parameter.			
			if (extrapolationMethod == TExtrapolationMethod::zeroInsertion) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation with zeros will be performed to fill it.");
				extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, t_HRTF_DataBase_ListOfOrientations, HRIRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetZerosHRIR());
			}
			else if (extrapolationMethod == TExtrapolationMethod::nearestPoint) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation will be made to the nearest point to fill it.");
				extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, t_HRTF_DataBase_ListOfOrientations, HRIRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetNearestPointHRIR());
			}
			else {
				SET_RESULT(RESULT_ERROR_NOTSET, "Extrapolation Method not set up.");
				// Do nothing
			}									
		}

		// Reset HRTF		
		void Reset() {

			//Change class state
			setupInProgress = false;
			HRTFLoaded = false;

			//Clear every table			
			t_HRTF_DataBase.clear();
			t_HRTF_Resampled_frequency.clear();
			t_HRTF_Resampled_partitioned.clear();

			//Update parameters			
			HRIRLength = 0;
			bufferSize = 0;
			resamplingStep = DEFAULT_RESAMPLING_STEP;
		}			
	};
}
#endif

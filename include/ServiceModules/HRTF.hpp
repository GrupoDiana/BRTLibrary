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
#include <ServiceModules/OfflineInterpolation.hpp>
#include <ServiceModules/OnlineInterpolation.hpp>
#include <ServiceModules/GridsManager.hpp>


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
			epsilon_sewing{ EPSILON_SEWING }, samplingRate{ -1 }
		{}

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
		void BeginSetup(int32_t _HRIRLength, float _distance)
		{
			//if ((ownerListener != nullptr) && ownerListener->ownerCore!=nullptr)
			{
				//Update parameters			
				HRIRLength = _HRIRLength;
				distanceOfMeasurement = _distance;
				bufferSize = globalParameters.GetBufferSize();

				float partitions = (float)HRIRLength / (float)bufferSize;
				HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				elevationNorth = CHRTFAuxiliarMethods::GetPoleElevation(TPole::north);
				elevationSouth = CHRTFAuxiliarMethods::GetPoleElevation(TPole::south);

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
		void AddHRIR(float _azimuth, float _elevation, THRIRStruct&& newHRIR)
		{
			if (setupInProgress) {
				Common::CVector3 cartessianPos;
				cartessianPos.SetFromAED(_azimuth, _elevation, GetHRTFDistanceOfMeasurement());
				auto returnValue = t_HRTF_DataBase.emplace(orientation(_azimuth, _elevation, cartessianPos), std::forward<THRIRStruct>(newHRIR));
				//Error handler
				if (!returnValue.second) { SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
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
					//Delete the common delay of every HRIR functions of the DataBase Table
					RemoveCommonDelay_HRTFDataBaseTable();
					// Preparation of table read from sofa file
					CalculateHRIR_InPoles(resamplingStep);
					FillOutTableOfAzimuth360(resamplingStep);
					FillSphericalCap_HRTF(gapThreshold, resamplingStep);
					CalculateListOfOrientations_T_HRTF_DataBase();

					//Creation and filling of resampling HRTF table
					quasiUniformSphereDistribution.CreateGrid(t_HRTF_Resampled_partitioned, stepVector, resamplingStep);
					FillResampledTable();

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

		/**
		 * @brief Fill vector with the list of orientations of the T_HRTF_DataBase table
		*/
		void CalculateListOfOrientations_T_HRTF_DataBase() {
			t_HRTF_DataBase_ListOfOrientations.reserve(t_HRTF_DataBase.size());
			for (auto& kv : t_HRTF_DataBase)
			{
				t_HRTF_DataBase_ListOfOrientations.push_back(kv.first);
			}
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
				//FindNearestHRIR_NewGrid(newHRIR, _azimuth, _elevation, ear, stepVector);
				quasiUniformSphereDistribution.FindNearestHRIR(t_HRTF_Resampled_partitioned, newHRIR, stepVector, ear, _azimuth, _elevation);
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

			//Run time interpolation ON
			
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
				float leftDelay;
				float rightDelay;
				quasiUniformSphereDistribution.FindNearestDelay(t_HRTF_Resampled_partitioned, leftDelay, stepVector, Common::T_ear::LEFT, _azimuthCenter, _elevationCenter);
				quasiUniformSphereDistribution.FindNearestDelay(t_HRTF_Resampled_partitioned, rightDelay, stepVector, Common::T_ear::RIGHT, _azimuthCenter, _elevationCenter);

				data.leftDelay	= static_cast<uint64_t>(leftDelay);
				data.rightDelay = static_cast<uint64_t>(rightDelay);
				return data;				
			}

			//  We have to do the run time interpolation -- (runTimeInterpolation = true)

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
				//Run time interpolation ON
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

		/** \brief Set the sampling rate for the SRTF
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


		float sphereBorder;								// Define spheere "sewing"
		float epsilon_sewing;

		int azimuthMin, azimuthMax, elevationMin, elevationMax, elevationNorth, elevationSouth;	// Variables that define limits of work area

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int resamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool enableCustomizedITD;					// Indicate the use of a customized delay
		int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap

		std::unordered_map<orientation, float> stepVector;

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


		CQuasiUniformSphereDistribution quasiUniformSphereDistribution;
		CDistanceBasedInterpolator distanceBasedInterpolator;
		//CMidPointOnlineInterpolator midPointOnlineInterpolator;
		CSlopesMethodOnlineInterpolator slopesMethodOnlineInterpolator;

		friend class CHRTFTester;
		/////////////
		// METHODS
		/////////////

		//	Fill out the HRTF for every azimuth and two specific elevations: 90 and 270 degrees		
		void CalculateHRIR_InPoles(int _resamplingStep)
		{
			THRIRStruct precalculatedHRIR_270, precalculatedHRIR_90;
			bool found = false;
			//Clasify every HRIR of the HRTF into the two hemispheres by their t_HRTF_DataBase_ListOfOrientations
			std::vector<orientation> keys_southernHemisphere, keys_northenHemisphere;
			int iAzimuthPoles = azimuthMin;
			int iElevationNorthPole = elevationNorth;
			int iElevationSouthPole = elevationSouth;

			//	NORTHERN HEMOSPHERE POLES (90 degrees elevation ) ____________________________________________________________________________
			auto it90 = t_HRTF_DataBase.find(orientation(iAzimuthPoles, iElevationNorthPole)); 

			if (it90 != t_HRTF_DataBase.end())
			{ 
				precalculatedHRIR_90 = it90->second; 
			}
			else
			{
				keys_northenHemisphere.reserve(t_HRTF_DataBase.size());
				
				for (auto& it : t_HRTF_DataBase)
				{
					if (it.first.elevation < iElevationNorthPole) { keys_northenHemisphere.push_back(it.first); }
				}
				// sort using a custom function object
				struct { bool operator()(orientation a, orientation b) const { return a.elevation > b.elevation;}
				} customLess;
				std::sort(keys_northenHemisphere.begin(), keys_northenHemisphere.end(), customLess);

				precalculatedHRIR_90 = CalculateHRIR_InOneHemispherePole(keys_northenHemisphere);

				SET_RESULT(RESULT_WARNING, "HRIR interpolated in the pole [ " + std::to_string(iAzimuthPoles) + ", " + std::to_string(iElevationNorthPole) + "]");
			}

			//	SOURTHERN HEMOSPHERE POLES (270 degrees elevation) ____________________________________________________________________________
			auto it270 = t_HRTF_DataBase.find(orientation(iAzimuthPoles, iElevationSouthPole));

			if (it270 != t_HRTF_DataBase.end())
			{
				precalculatedHRIR_270 = it270->second;
			}
			else
			{
				keys_southernHemisphere.reserve(t_HRTF_DataBase.size());
				for (auto& it : t_HRTF_DataBase)
				{
					if (it.first.elevation > iElevationSouthPole) { keys_southernHemisphere.push_back(it.first);}
				}
				//Get a vector of iterators ordered from highest to lowest elevation.		
				struct {
					bool operator()(orientation a, orientation b) const	{ return a.elevation < b.elevation; }
				} customLess;
				std::sort(keys_southernHemisphere.begin(), keys_southernHemisphere.end(), customLess);

				precalculatedHRIR_270 = CalculateHRIR_InOneHemispherePole(keys_southernHemisphere);
				
				SET_RESULT(RESULT_WARNING, "HRIR interpolated in the pole [ " + std::to_string(iAzimuthPoles) + ", " + std::to_string(iElevationSouthPole) + "]");
			}
			// Fill out the table for "every azimuth in the pole" ____________________________________________________________________________
			for (int az = azimuthMin; az < azimuthMax; az = az + _resamplingStep)
			{
				//Elevation 90 degrees
				t_HRTF_DataBase.emplace(orientation(az, iElevationNorthPole), precalculatedHRIR_90);
				//Elevation 270 degrees
				t_HRTF_DataBase.emplace(orientation(az, iElevationSouthPole), precalculatedHRIR_270);
			}
		}

		//	Calculate the HRIR in the pole of one of the hemispheres
		//param hemisphereParts	vector of the HRTF t_HRTF_DataBase_ListOfOrientations of the hemisphere		
		THRIRStruct CalculateHRIR_InOneHemispherePole(std::vector<orientation> keys_hemisphere)
		{
			THRIRStruct calculatedHRIR;
			std::vector < std::vector <orientation>> hemisphereParts;
			hemisphereParts.resize(NUMBER_OF_PARTS); 
			int border = std::ceil(sphereBorder / NUMBER_OF_PARTS);
			auto currentElevation = keys_hemisphere.begin()->elevation;
			for (auto& it : keys_hemisphere)
			{
				if (it.elevation == currentElevation)
				{
					//Clasify each orientation in its corresponding hemisphere part group (by the azimuth value). The number of the parts and the size depend on the NUMBER_OF_PARTS value
					int32_t currentAzimuth = it.azimuth;
					for (int j = 0; j < NUMBER_OF_PARTS; j++)
					{
						if (currentAzimuth >= (border * j) && currentAzimuth < (border * (j + 1))) {
							hemisphereParts[j].push_back(it);
							break;
						}
					}
				}
				else
				{
					//Check if there are values in every hemisphere part
					bool everyPartWithValues = true;
					for (int j = 0; j < NUMBER_OF_PARTS; j++)
					{
						everyPartWithValues = everyPartWithValues && (hemisphereParts[j].size() > 0);
					}

					//If hemisphere every part has at list a value, the algorithm of select the interpolated HRIR finish
					if (everyPartWithValues) { break; }
					//else, the algorithm continue, unless the elevation is MAX_DISTANCE_BETWEEN_ELEVATIONS bigger than the first selected elevation
					else
					{
						currentElevation = it.elevation;
						if (currentElevation > (keys_hemisphere.begin()->elevation + MAX_DISTANCE_BETWEEN_ELEVATIONS))
						{
							break;
						}
						else 
						{
							int32_t currentAzimuth = it.azimuth;
							for (int j = 0; j < NUMBER_OF_PARTS; j++)
							{
								//The iterator (it) has moved fordward, so this orientation must be clasified a hemispehere part group
								if (currentAzimuth >= (border * j) && currentAzimuth < (border * (j + 1))) {
									hemisphereParts[j].push_back(it);
									break;
								}
							}
						}
					} //END else 'if(everyPartWithValues)'
				}// END else of 'if(it.elevation == currentElevation)'
			}//END loop keys_hemisphere

			 //Calculate the delay and the HRIR of each hemisphere part
			float totalDelay_left = 0.0f;
			float totalDelay_right = 0.0f;

			std::vector< THRIRStruct> newHRIR;
			newHRIR.resize(hemisphereParts.size());

			for (int q = 0; q < hemisphereParts.size(); q++)
			{
				newHRIR[q].leftHRIR.resize(HRIRLength, 0.0f);
				newHRIR[q].rightHRIR.resize(HRIRLength, 0.0f);

				float scaleFactor;
				if (hemisphereParts[q].size())
				{
					scaleFactor = 1.0f / hemisphereParts[q].size(); 
				}
				else
				{
					scaleFactor = 0.0f;
				}

				for (auto it = hemisphereParts[q].begin(); it != hemisphereParts[q].end(); it++)
				{
					auto itHRIR = t_HRTF_DataBase.find(orientation(it->azimuth, it->elevation));

					//Get the delay
					newHRIR[q].leftDelay = (newHRIR[q].leftDelay + itHRIR->second.leftDelay);
					newHRIR[q].rightDelay = (newHRIR[q].rightDelay + itHRIR->second.rightDelay);

					//Get the HRIR
					for (int i = 0; i < HRIRLength; i++) {
						newHRIR[q].leftHRIR[i] = (newHRIR[q].leftHRIR[i] + itHRIR->second.leftHRIR[i]);
						newHRIR[q].rightHRIR[i] = (newHRIR[q].rightHRIR[i] + itHRIR->second.rightHRIR[i]);
					}


				}//END loop hemisphere part

				 //Multiply by the factor (weighted sum)
				 //Delay 
				totalDelay_left = totalDelay_left + (scaleFactor * newHRIR[q].leftDelay);
				totalDelay_right = totalDelay_right + (scaleFactor * newHRIR[q].rightDelay);
				//HRIR
				for (int i = 0; i < HRIRLength; i++)
				{
					newHRIR[q].leftHRIR[i] = newHRIR[q].leftHRIR[i] * scaleFactor;
					newHRIR[q].rightHRIR[i] = newHRIR[q].rightHRIR[i] * scaleFactor;
				}
			}

			//Get the FINAL values
			float scaleFactor_final = 1.0f / hemisphereParts.size();

			//Calculate Final delay
			calculatedHRIR.leftDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_left));
			calculatedHRIR.rightDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_right));

			//calculate Final HRIR
			calculatedHRIR.leftHRIR.resize(HRIRLength, 0.0f);
			calculatedHRIR.rightHRIR.resize(HRIRLength, 0.0f);

			for (int i = 0; i < HRIRLength; i++)
			{
				for (int q = 0; q < hemisphereParts.size(); q++)
				{
					calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] + newHRIR[q].leftHRIR[i];
					calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] + newHRIR[q].rightHRIR[i];
				}
			}
			for (int i = 0; i < HRIRLength; i++)
			{
				calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] * scaleFactor_final;
				calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] * scaleFactor_final;
			}

			return calculatedHRIR;
		}

		/// <summary>
		/// Get HRIR of azimith 0 and emplace again with azimuth 360 in the HRTF database table for every elevations
		/// </summary>
		/// <param name="_resamplingStep"></param>
		void FillOutTableOfAzimuth360(int _resamplingStep) {
			for (int el = elevationMin; el <= elevationNorth; el = el + _resamplingStep)
			{
				GetAndEmplaceHRIRinAzimuth360(el);
			}
			for (int el = elevationSouth; el < elevationMax; el = el + _resamplingStep)
			{
				GetAndEmplaceHRIRinAzimuth360(el);
			}
		}

		/// <summary>
		/// Get HRIR of azimith 0 and emplace again with azimuth 360 in the HRTF database table for an specific elevation
		/// </summary>
		/// <param name="_elevation"></param>
		void GetAndEmplaceHRIRinAzimuth360(float _elevation) {
			auto it = t_HRTF_DataBase.find(orientation(azimuthMin, _elevation));
			if (it != t_HRTF_DataBase.end()) {
				t_HRTF_DataBase.emplace(orientation(azimuthMax, _elevation), it->second);
			}
		}

		/// <summary>
		/// Fill Spherical Cap Gap of an HRTF making Interpolation between pole and the 2 nearest points.
		/// </summary>
		/// <param name="_gapThreshold"></param>
		/// <param name="_resamplingStep"></param>
		void FillSphericalCap_HRTF(int _gapThreshold, int _resamplingStep)
		{
			// Initialize some variables
			float max_dist_elev = 0;
			int elev_Step = _resamplingStep;
			int pole;
			float elev_south, elev_north, distance;
			std::vector<orientation> orientations, north_hemisphere, south_hemisphere;
			orientation bufferOrientation;

			// Create a vector with all the t_HRTF_DataBase_ListOfOrientations of the Database
			orientations.reserve(t_HRTF_DataBase.size());
			for (auto& itr : t_HRTF_DataBase)
			{
				orientations.push_back(itr.first);
			}

			//  Sort t_HRTF_DataBase_ListOfOrientations of the DataBase with a lambda function in sort.
			std::sort(orientations.begin(), orientations.end(), [](orientation a, orientation b) {return a.elevation < b.elevation; });

			//	Separating t_HRTF_DataBase_ListOfOrientations of both hemispheres
			std::copy_if(orientations.begin(), orientations.end(), back_inserter(south_hemisphere), [](orientation n) {return n.elevation > 180; }); //SOUTH
			std::copy_if(orientations.begin(), orientations.end(), back_inserter(north_hemisphere), [](orientation n) {return n.elevation < 180; }); //NORTH


			reverse(north_hemisphere.begin(), north_hemisphere.end());

			// SOUTH HEMISPHERE
			CalculateDistanceBetweenPoleandLastRing(south_hemisphere, max_dist_elev, elev_south);

			if (max_dist_elev > _gapThreshold)
			{
				//pole = ELEVATION_SOUTH_POLE; // 270
				Calculate_and_EmplaceHRIR(elevationSouth, south_hemisphere, elev_south, elev_Step);
			}
			// Reset var to use it in north hemisphere
			max_dist_elev = 0;

			// NORTH HEMISPHERE
			CalculateDistanceBetweenPoleandLastRing(north_hemisphere, max_dist_elev, elev_north);

			if (max_dist_elev > _gapThreshold)
			{
				//pole = ELEVATION_NORTH_POLE; //90
				Calculate_and_EmplaceHRIR(elevationNorth, north_hemisphere, elev_north, elev_Step);
			}
		}

		/// <summary>
		/// Calculate the max distance between pole and the nearest ring, to know if there is a gap in any spherical cap
		/// </summary>
		/// <param name="_hemisphere"></param>
		/// <param name="_max_dist_elev"></param>
		/// <param name="elevationLastRing"></param>
		void CalculateDistanceBetweenPoleandLastRing(std::vector<orientation>& _hemisphere, float& _max_dist_elev, float& elevationLastRing)
		{
			for (int jj = 1; jj < _hemisphere.size(); jj++)
			{
				// distance between 2 t_HRTF_DataBase_ListOfOrientations, always positive
				if (_hemisphere[jj].elevation != _hemisphere[0].elevation)
				{
					_max_dist_elev = abs(_hemisphere[jj].elevation - _hemisphere[0].elevation); // Distance positive for all poles
					elevationLastRing = _hemisphere[jj].elevation;
					break;
				}
			}
		}

		/// <summary>
		/// Calculate the HRIR we need by interpolation and emplace it in the database
		/// </summary>
		/// <param name="_hemisphere"></param>
		/// <param name="elevationLastRing"></param>
		/// <param name="_fillStep"></param>
		void Calculate_and_EmplaceHRIR(int _pole, std::vector<orientation> _hemisphere, float _elevationLastRing, int _fillStep)
		{
			std::vector<orientation> lastRingOrientationList;
			std::vector<T_PairDistanceOrientation> sortedList;
			int azimuth_Step = _fillStep;
			THRIRStruct HRIR_interpolated;

			// Get a list with only the points of the nearest known ring
			for (auto& itr : _hemisphere)
			{
				if (itr.elevation == _elevationLastRing)
				{
					lastRingOrientationList.push_back(itr);
				}
			}
			// SOUTH HEMISPHERE
			if (_pole == ELEVATION_SOUTH_POLE)
			{
				for (float elevat = _pole + _fillStep; elevat < _elevationLastRing; elevat = elevat + _fillStep)
				{
					for (float azim = azimuthMin; azim <= azimuthMax; azim = azim + azimuth_Step)
					{
						//sortedList = distanceBasedInterpolator.GetSortedDistancesList(lastRingOrientationList, azim, elevat);
						HRIR_interpolated = distanceBasedInterpolator.CalculateHRIR_offlineMethod(t_HRTF_DataBase, lastRingOrientationList, azim, elevat, HRIRLength, _pole);
						t_HRTF_DataBase.emplace(orientation(azim, elevat), HRIR_interpolated);
					}
				}
			}	// NORTH HEMISPHERE
			else if (_pole == ELEVATION_NORTH_POLE)
			{
				for (float elevat = _elevationLastRing + _fillStep; elevat < _pole; elevat = elevat + _fillStep)
				{
					for (float azim = azimuthMin; azim <= azimuthMax; azim = azim + azimuth_Step)
					{
						//sortedList = distanceBasedInterpolator.GetSortedDistancesList(lastRingOrientationList, azim, elevat);
						HRIR_interpolated = distanceBasedInterpolator.CalculateHRIR_offlineMethod(t_HRTF_DataBase, lastRingOrientationList, azim, elevat, HRIRLength, _pole);
						t_HRTF_DataBase.emplace(orientation(azim, elevat), HRIR_interpolated);
					}
				}
			}
		}	

		void FillResampledTable() {
			int numOfInterpolatedHRIRs = 0;
			for (auto& it : t_HRTF_Resampled_partitioned)
			{
				if (CalculateAndEmplaceNewPartitionedHRIR(it.first.azimuth, it.first.elevation)) { numOfInterpolatedHRIRs++; }
			}
			SET_RESULT(RESULT_WARNING, "Number of interpolated HRIRs: " + std::to_string(numOfInterpolatedHRIRs));
		}

		/// <summary>
		/// Calculate the resample HRIR using the Barycentric interpolation Method 
		/// </summary>
		/// <param name="newAzimuth"></param>
		/// <param name="newElevation"></param>
		/// <returns>interpolatedHRIRs: true if the HRIR has been calculated using the interpolation</returns>
		bool CalculateAndEmplaceNewPartitionedHRIR(float _azimuth, float _elevation) {
			THRIRStruct interpolatedHRIR;
			bool bHRIRInterpolated = false;
			auto it = t_HRTF_DataBase.find(orientation(_azimuth, _elevation));
			if (it != t_HRTF_DataBase.end())
			{
				//Fill out HRTF partitioned table.IR in frequency domain
				THRIRPartitionedStruct newHRIR_partitioned;
				newHRIR_partitioned = SplitAndGetFFT_HRTFData(it->second);
				t_HRTF_Resampled_partitioned[orientation(_azimuth, _elevation)] = std::forward<THRIRPartitionedStruct>(newHRIR_partitioned);				
			}
			else
			{				
				// Get a list sorted by distances to the orientation of interest
				//std::vector<T_PairDistanceOrientation> sortedList = distanceBasedInterpolator.GetSortedDistancesList(t_HRTF_DataBase_ListOfOrientations, _azimuth, _elevation);
				//Get the interpolated HRIR 
				interpolatedHRIR = distanceBasedInterpolator.CalculateHRIR_offlineMethod(t_HRTF_DataBase, t_HRTF_DataBase_ListOfOrientations, _azimuth, _elevation, HRIRLength);
				bHRIRInterpolated = true;

				//Fill out HRTF partitioned table.IR in frequency domain
				THRIRPartitionedStruct newHRIR_partitioned;
				newHRIR_partitioned = SplitAndGetFFT_HRTFData(interpolatedHRIR);
				t_HRTF_Resampled_partitioned[orientation(_azimuth, _elevation)] =  std::forward<THRIRPartitionedStruct>(newHRIR_partitioned);				
			}
			return bHRIRInterpolated;
		
		}

		//	Split the input HRIR data in subfilters and get the FFT to apply the UPC algorithm
		//param	newData_time	HRIR value in time domain		
		THRIRPartitionedStruct SplitAndGetFFT_HRTFData(const THRIRStruct& newData_time)
		{
			int blockSize = bufferSize;
			int numberOfBlocks = HRIR_partitioned_NumberOfSubfilters;
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
				Common::CFprocessor::CalculateFFT(left_data_FFT_doubleSize, left_data_FFT);
				Common::CFprocessor::CalculateFFT(right_data_FFT_doubleSize, right_data_FFT);
				//Prepare struct to return the value
				new_DataFFT_Partitioned.leftHRIR_Partitioned.push_back(left_data_FFT);
				new_DataFFT_Partitioned.rightHRIR_Partitioned.push_back(right_data_FFT);
			}
			//Store the delays
			new_DataFFT_Partitioned.leftDelay = newData_time.leftDelay;
			new_DataFFT_Partitioned.rightDelay = newData_time.rightDelay;

			return new_DataFFT_Partitioned;
		}
					

		//		Calculate and remove the common delay of every HRIR functions of the DataBase Table. Off line Method, called from EndSetUp()		
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

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


#ifndef _CHRTF_H_
#define _CHRTF_H_

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/FFTCalculator.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/CranicalGeometry.hpp>
#include <ServiceModules/ServicesBase.hpp>
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
			:enableWoodworthITD{ false }, gridSamplingStep{ DEFAULT_GRIDSAMPLING_STEP }, gapThreshold{ DEFAULT_GAP_THRESHOLD }, HRIRLength{ 0 }, fileName{ "" },
			HRTFLoaded{ false }, setupInProgress{ false }, distanceOfMeasurement{ DEFAULT_HRTF_MEASURED_DISTANCE },
			azimuthMin{ DEFAULT_MIN_AZIMUTH }, azimuthMax{ DEFAULT_MAX_AZIMUTH }, elevationMin{ DEFAULT_MIN_ELEVATION }, elevationMax{ DEFAULT_MAX_ELEVATION }, sphereBorder{ SPHERE_BORDER },
			epsilon_sewing{ EPSILON_SEWING }, samplingRate{ -1 }, elevationNorth{ 0 }, elevationSouth{ 0 }, extrapolationMethod{ TEXTRAPOLATION_METHOD::nearest_point }
		{ }

		/** \brief Get size of each HRIR buffer
		*	\retval size number of samples of each HRIR buffer for one ear
		*   \eh Nothing is reported to the error handler.
		*/
		int32_t GetHRIRLength() const
		{
			return HRIRLength;
		}

		/**
		 * @brief Set sampling step for HRIR resampling
		 * @param _resamplingStep 
		 */
		void SetGridSamplingStep(int _samplingStep) {
			gridSamplingStep = _samplingStep;
		}
		/**
		 * @brief Get sampling step defined for HRIR resampling
		 * @return 
		 */
		int GetGridSamplingStep() {
			return gridSamplingStep;
		}

		/** \brief Start a new HRTF configuration
		*	\param [in] _HRIRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool BeginSetup(int32_t _HRIRLength, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) override {		
			std::lock_guard<std::mutex> l(mutex);
			//Change class state
			setupInProgress = true;
			HRTFLoaded = false;
			//Clear every table			
			t_HRTF_DataBase.clear();
			t_HRTF_Resampled_partitioned.clear();
			stepVector.clear();

			//Update parameters			
			HRIRLength = _HRIRLength;				
			extrapolationMethod = _extrapolationMethod;
			float partitions = (float)HRIRLength / (float)globalParameters.GetBufferSize();
			HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));
			elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);
					
			SET_RESULT(RESULT_OK, "HRTF Setup started");
			return true;
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
		

		void AddHRIR(double _azimuth, double _elevation, double _distance, Common::CVector3 listenerPosition, THRIRStruct&& newHRIR) override {			
			if (setupInProgress) {				
				_azimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_azimuth);
				_elevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);						
				auto returnValue = t_HRTF_DataBase.emplace(orientation(_azimuth, _elevation, _distance), std::forward<THRIRStruct>(newHRIR));
				//Error handler
				if (!returnValue.second) { 
					SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in t_HRTF_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]"); }
			}
		}


		/** \brief Stop the HRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup() override {
			std::lock_guard<std::mutex> l(mutex);
			if (setupInProgress) {
				if (!t_HRTF_DataBase.empty())
				{					
					distanceOfMeasurement = t_HRTF_DataBase.begin()->first.distance;	// Get first Distance as the distance of measurement //TODO Change
					
					// Preparation of table read from sofa file
					RemoveCommonDelay_HRTFDataBaseTable();				// Delete the common delay of every HRIR functions of the DataBase Table					
					std::vector<orientation> _orientationList = offlineInterpolation.CalculateListOfOrientations(t_HRTF_DataBase);
					CalculateExtrapolation(_orientationList);							// Make the extrapolation if it's needed
					offlineInterpolation.CalculateTF_InPoles<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, HRIRLength, gridSamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromHemisphereParts());
					offlineInterpolation.CalculateTF_SphericalCaps<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, HRIRLength, gapThreshold, gridSamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());
					//Creation and filling of resampling HRTF table
					//_orientationList = offlineInterpolation.CalculateListOfOrientations(t_HRTF_DataBase);
					CQuasiUniformSphereDistribution::CreateGrid<T_HRTFPartitionedTable, THRIRPartitionedStruct>(t_HRTF_Resampled_partitioned, stepVector, gridSamplingStep);
					offlineInterpolation.FillResampledTable<T_HRTFTable, T_HRTFPartitionedTable, BRTServices::THRIRStruct, BRTServices::THRIRPartitionedStruct> (t_HRTF_DataBase, t_HRTF_Resampled_partitioned, globalParameters.GetBufferSize(), HRIRLength, HRIR_partitioned_NumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_HRTFData(), CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());					

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
		void EnableWoodworthITD() {	enableWoodworthITD = true;	}

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableWoodworthITD() { enableWoodworthITD = false; }

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsWoodworthITDEnabled() {	return enableWoodworthITD;	}
		

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
		const std::vector<CMonoBuffer<float>> GetHRIRPartitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation, const Common::CTransform& /* _listenerLocation*/ ) const override
		{
			std::lock_guard<std::mutex> l(mutex);
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

			return CHRTFAuxiliarMethods::GetHRIRFromPartitionedTable(t_HRTF_Resampled_partitioned, ear, _azimuth, _elevation, runTimeInterpolation, 
				HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, stepVector);										
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
		THRIRPartitionedStruct GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation,	Common::CTransform& _listenerLocation)
		{			
			std::lock_guard<std::mutex> l(mutex);
			THRIRPartitionedStruct data;
			
			if (setupInProgress)
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: HRTF Setup in progress return empty");
				return data;
			}
			
			// Modify delay if customized delay is activate
			if (enableWoodworthITD)
			{			
				data.leftDelay = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry,Common::T_ear::LEFT);
				data.rightDelay = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::RIGHT);
				return data;
			}

			return CHRTFAuxiliarMethods::GetHRIRDelayFromPartitioned(t_HRTF_Resampled_partitioned, ear, _azimuthCenter, _elevationCenter, runTimeInterpolation,
				HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, stepVector);													
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

		/** \brief	Set the radius of the listener head
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHeadRadius(float _headRadius) 
		{
			std::lock_guard<std::mutex> l(mutex);
			if (_headRadius >= 0.0f) {
				// First time this is called we save the original cranial geometry. A bit ugly but it works.
				if (originalCranialGeometry.GetHeadRadius() == -1) { originalCranialGeometry = cranialGeometry; }				
				cranialGeometry.SetHeadRadius(_headRadius);
			}
			else{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Head Radius must be  greater than 0.");
			}
				
		}

		/** \brief	Get the radius of the listener head
		*   \return listenerHeadRadius in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() {
			return cranialGeometry.GetHeadRadius();
		}

		/**
		 * @brief Return to original ear positions and head radius.
		 */
		void RestoreHeadRadius() {
			std::lock_guard<std::mutex> l(mutex);
			cranialGeometry = originalCranialGeometry;
		}

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition( Common::T_ear _ear, Common::CVector3 _earPosition) {
			if (_ear == Common::T_ear::LEFT){
				cranialGeometry.SetLeftEarPosition(_earPosition);
			}
			else if (_ear == Common::T_ear::RIGHT) {
				cranialGeometry.SetRightEarPosition(_earPosition);
			}			
			else {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
			}
				
		}

		/** \brief	Get the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) {			
			if (_ear == Common::T_ear::LEFT)		{ return cranialGeometry.GetLeftEarLocalPosition(); }
			else if (_ear == Common::T_ear::RIGHT) { return cranialGeometry.GetRightEarLocalPosition(); }
			else 
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}
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
		///////////////
		// ATTRIBUTES
		///////////////		

		mutable std::mutex mutex;								// Thread management

		int32_t HRIRLength;								// HRIR vector length
		//int32_t bufferSize;								// Input signal buffer size		
		int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter
		float distanceOfMeasurement;					// Distance where the HRIR have been measurement		
		Common::CCranialGeometry cranialGeometry;					// Cranial geometry of the listener
		Common::CCranialGeometry originalCranialGeometry;		// Cranial geometry of the listener
		TEXTRAPOLATION_METHOD extrapolationMethod;		// Methods that is going to be used to extrapolate


		float sphereBorder;								// Define spheere "sewing"
		float epsilon_sewing;

		float azimuthMin, azimuthMax, elevationMin, elevationMax, elevationNorth, elevationSouth;	// Variables that define limits of work area

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		//bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int gridSamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool enableWoodworthITD;					// Indicate the use of a customized delay
		int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap

		

		std::string title;
		std::string databaseName;
		std::string listenerShortName;
		std::string fileName;
		int samplingRate;

		// HRTF tables							
		T_HRTFTable				t_HRTF_DataBase;				// Store original data, normally read from SOFA file
		T_HRTFPartitionedTable	t_HRTF_Resampled_partitioned;	// Data in our grid, interpolated 
		std::unordered_map<orientation, float> stepVector;		// Store hrtf interpolated grids steps

		// Empty object to return in some methods
		THRIRStruct						emptyHRIR;
		THRIRPartitionedStruct			emptyHRIR_partitioned;
		CMonoBuffer<float>				emptyMonoBuffer;		
		Common::CGlobalParameters globalParameters;

		// Processors
		//CQuasiUniformSphereDistribution quasiUniformSphereDistribution;		
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
												

		//}
		/**
		 * @brief Call the extrapolation method
		*/
		void CalculateExtrapolation(std::vector<orientation>& _orientationList) {
			// Select the one that extrapolates with zeros or the one that extrapolates based on the nearest point according to some parameter.			
			if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::zero_insertion) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation with zeros will be performed to fill it.");
				extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, _orientationList, HRIRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetZerosHRIR());
			}
			else if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::nearest_point) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation will be made to the nearest point to fill it.");
				extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(t_HRTF_DataBase, _orientationList, HRIRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetNearestPointHRIR());
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
			t_HRTF_Resampled_partitioned.clear();

			//Update parameters			
			HRIRLength = 0;			
			gridSamplingStep = DEFAULT_GRIDSAMPLING_STEP;
		}			
	};
}
#endif

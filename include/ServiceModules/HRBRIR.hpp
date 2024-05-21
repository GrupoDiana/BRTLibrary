/**
* \class CHRBRIR
*
* \brief Declaration of CHRTF class interface
* \date	March 2024
*
*\authors 3DI - DIANA Research Group(University of Malaga), in alphabetical order : M.Cuevas - Rodriguez, D.Gonzalez - Toledo, L.Molina - Tanco, F.Morales - Benitez ||
*Coordinated by, A.Reyes - Lecuona(University of Malaga) ||
*\b Contact : areyes@uma.es
*
*\b Contributions : (additional authors / contributors can be added here)
*
*\b Project : SONICOM ||
*\b Website : https://www.sonicom.eu/
*
*\b Copyright : University of Malaga 2024. Code based in the 3DTI Toolkit library(https ://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
	*
*\b Licence : This program is free software, you can redistribute it and /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
*\b Acknowledgement : This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CHRBRIR_H_
#define _CHRBRIR_H_

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
#include <Common/CranicalGeometry.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/HRTFAuxiliarMethods.hpp>

namespace BRTServices
{
			
	class CHRBRIR : public CServicesBase
	{
	public:

		CHRBRIR() : setupInProgress{ false }, samplingRate{ 0 }, HRIRLength{-1}, gridSamplingStep{ DEFAULT_GRIDSAMPLING_STEP }, 
			gapThreshold {DEFAULT_GAP_THRESHOLD}, sphereBorder{ SPHERE_BORDER }, epsilon_sewing{ EPSILON_SEWING }, extrapolationMethod{ TEXTRAPOLATION_METHOD::zero_insertion },
			azimuthMin{ DEFAULT_MIN_AZIMUTH }, azimuthMax{ DEFAULT_MAX_AZIMUTH }, elevationMin{ DEFAULT_MIN_ELEVATION }, elevationMax{ DEFAULT_MAX_ELEVATION },
			elevationNorth{ 0 }, elevationSouth{ 0 }, title{""},	databaseName{""}, listenerShortName{""}, fileName{""} {}



		/** \brief Start a new HRBRIR configuration
		*	\param [in] _HRIRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void BeginSetup(int32_t _HRIRLength, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod)
		{
			//if ((ownerListener != nullptr) && ownerListener->ownerCore!=nullptr)
			{
				////Update parameters			
				HRIRLength = _HRIRLength;								
				extrapolationMethod = _extrapolationMethod;

				float partitions = (float)HRIRLength / (float)globalParameters.GetBufferSize();
				HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
				elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

				////Clear every table		
				t_HRBRIR_DataBase.clear();
				t_HRBRIR_Resampled_partitioned.clear();
				t_HRBRIR_DataBase_ListenerPositions.clear();
				t_HRBRIR_DataBase_EmitterPositions.clear();
				
				//Change class state
				setupInProgress = true;
				HRTFLoaded = false;


				SET_RESULT(RESULT_OK, "HRBRIR Setup started");
			}			
		}

		void AddHRBRIR(double _azimuth, double _elevation, double _distance, Common::CVector3 emitterPosition, Common::CVector3 listenerPosition, THRIRStruct&& newHRBRIR) {
			if (setupInProgress) {
				_azimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_azimuth);
				_elevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
				bool error = false;
				//Check if the listenerPosition is already in the table
				auto it = t_HRBRIR_DataBase.find(TDuplaVector3(listenerPosition, emitterPosition));
				if (it != t_HRBRIR_DataBase.end())
				{
					auto returnValue = it->second.emplace(orientation(_azimuth, _elevation, _distance), std::forward<THRIRStruct>(newHRBRIR));
					if (!returnValue.second) {
						error = true;
					}
				}
				else
				{
					T_HRTFTable orientationTable;
					auto returnValue = orientationTable.emplace(orientation(_azimuth, _elevation, _distance), std::forward<THRIRStruct>(newHRBRIR));
					if (returnValue.second) {
						auto returnValue2 = t_HRBRIR_DataBase.emplace(TDuplaVector3(listenerPosition, emitterPosition), std::forward<T_HRTFTable>(orientationTable));
						if (returnValue2.second) {
							AddToListenersPositions(listenerPosition);
							AddToEmitterPositions(emitterPosition);							
						}
						else {
							error = true;
						}
					}
					else {
						error = true;
					}
				}
				if (error) {
					SET_RESULT(RESULT_WARNING, "Error emplacing HRBRIR in t_HRBRIR_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
				}
			}
		}
				
		/** \brief Stop the HRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup()
		{
			if (setupInProgress) {
				if (!t_HRBRIR_DataBase.empty())
				{
					//distanceOfMeasurement = t_HRTF_DataBase.begin()->first.distance;	// Get first Distance as the distance of measurement //TODO Change

					// Preparation of table read from sofa file
					for (auto it = t_HRBRIR_DataBase.begin(); it != t_HRBRIR_DataBase.end(); it++) {						
						//RemoveCommonDelay_HRTFDataBaseTable();				// Delete the common delay of every HRIR functions of the DataBase Table											
						std::vector<orientation> orientationsList = offlineInterpolation.CalculateListOfOrientations(it->second);
						CalculateExtrapolation(it->second, orientationsList);	// Make the extrapolation if it's needed
						offlineInterpolation.CalculateTF_InPoles<T_HRTFTable, BRTServices::THRIRStruct>(it->second, HRIRLength, gridSamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromHemisphereParts());
						offlineInterpolation.CalculateTF_SphericalCaps<T_HRTFTable, BRTServices::THRIRStruct>(it->second, HRIRLength, gapThreshold, gridSamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());
						//Creation and filling of resampling HRTF table
						//orientationsList = offlineInterpolation.CalculateListOfOrientations(it->second);
						T_HRTFPartitionedTable tempPartitionedTable;
						quasiUniformSphereDistribution.CreateGrid<T_HRTFPartitionedTable, THRIRPartitionedStruct>(tempPartitionedTable, stepVector, gridSamplingStep);
						offlineInterpolation.FillResampledTable<T_HRTFTable, T_HRTFPartitionedTable, BRTServices::THRIRStruct, BRTServices::THRIRPartitionedStruct>(it->second, tempPartitionedTable, globalParameters.GetBufferSize(), HRIRLength, HRIR_partitioned_NumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_HRTFData(), CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());
					

						t_HRBRIR_Resampled_partitioned.emplace(it->first, std::forward<T_HRTFPartitionedTable>(tempPartitionedTable));
					}													

					// Setup values
					HRIR_partitioned_SubfilterLength = t_HRBRIR_Resampled_partitioned.begin()->second.begin()->second.leftHRIR_Partitioned[0].size();					
					setupInProgress = false;
					HRTFLoaded = true;

					SET_RESULT(RESULT_OK, "HRBRIR Matrix resample completed succesfully");
					return true;
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_HRBRIR_DataBase map has not been set");
				}
			}
			return false;
		}


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
						
		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) {
			if (_ear == Common::T_ear::LEFT) {
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
			if (_ear == Common::T_ear::LEFT) { return cranialGeometry.GetLeftEarLocalPosition(); }
			else if (_ear == Common::T_ear::RIGHT) { return cranialGeometry.GetRightEarLocalPosition(); }
			else
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}
		}

		/** \brief	Set the radius of the listener head
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHeadRadius(float _headRadius)
		{
			if (_headRadius >= 0.0f) {
				// First time this is called we save the original cranial geometry. A bit ugly but it works.
				if (originalCranialGeometry.GetHeadRadius() == -1) { originalCranialGeometry = cranialGeometry; }
				cranialGeometry.SetHeadRadius(_headRadius);
			}
			else {
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
			cranialGeometry = originalCranialGeometry;
		}

		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableWoodworthITD() { enableWoodworthITD = true; }

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableWoodworthITD() { enableWoodworthITD = false; }

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsWoodworthITDEnabled() { return enableWoodworthITD; }

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
		const std::vector<CMonoBuffer<float>> GetHRBRIRPartitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation, Common::CTransform& _listenerLocation, Common::CTransform& _sourceLocation) const
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

			// Find Table to use
			Common::CVector3 nearestListenerPosition = FindNearestListenerPosition(_listenerLocation.GetPosition());
			Common::CVector3 nearestEmitterPosition = FindNearestEmitterPosition(_sourceLocation.GetPosition());
			auto selectedTable = t_HRBRIR_Resampled_partitioned.find(TDuplaVector3(nearestListenerPosition, nearestEmitterPosition));

			// Process
			return  CHRTFAuxiliarMethods::GetHRIRFromPartitionedTable(selectedTable->second, ear, _azimuth, _elevation, runTimeInterpolation,
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
		THRIRPartitionedStruct GetHRBRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation, Common::CTransform& _listenerLocation, Common::CTransform& _sourceLocation)
		{
			THRIRPartitionedStruct data;

			if (setupInProgress)
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: HRTF Setup in progress return empty");
				return data;
			}

			//Modify delay if customized delay is activate
			if (enableWoodworthITD)
			{
				data.leftDelay = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::LEFT);
				data.rightDelay = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::RIGHT);
				return data;
			}

			// Find Table to use
			Common::CVector3 nearestListenerPosition = FindNearestListenerPosition(_listenerLocation.GetPosition());
			Common::CVector3 nearestEmitterPosition = FindNearestEmitterPosition(_sourceLocation.GetPosition());
			auto selectedTable = t_HRBRIR_Resampled_partitioned.find(TDuplaVector3(nearestListenerPosition, nearestEmitterPosition));

			return CHRTFAuxiliarMethods::GetHRIRDelayFromPartitioned(selectedTable->second, ear, _azimuthCenter, _elevationCenter, runTimeInterpolation,
				HRIR_partitioned_NumberOfSubfilters, HRIR_partitioned_SubfilterLength, stepVector);
		}

	private:
		
		/////////////
		// METHODS
		/////////////
		
		/**
		* @brief Call the extrapolation method
		*/
		void CalculateExtrapolation(T_HRTFTable& _table, std::vector<orientation>& _orientationList) {
			// Select the one that extrapolates with zeros or the one that extrapolates based on the nearest point according to some parameter.			
			if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::zero_insertion) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation with zeros will be performed to fill it.");
				extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(_table, _orientationList, HRIRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetZerosHRIR());
			}
			else if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::nearest_point) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation will be made to the nearest point to fill it.");
				extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(_table, _orientationList, HRIRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetNearestPointHRIR());
			}
			else {
				SET_RESULT(RESULT_ERROR_NOTSET, "Extrapolation Method not set up.");
				// Do nothing
			}
		}


		void AddToListenersPositions( Common::CVector3& _listenerPosition) {			
			//Check if the listenerPosition is already in the table
			auto it = std::find(t_HRBRIR_DataBase_ListenerPositions.begin(), t_HRBRIR_DataBase_ListenerPositions.end(), _listenerPosition);
			if (it == t_HRBRIR_DataBase_ListenerPositions.end()){
				t_HRBRIR_DataBase_ListenerPositions.push_back(_listenerPosition);
			}			
		}

		void AddToEmitterPositions(Common::CVector3& _emitterPosition) {
			//Check if the listenerPosition is already in the table
			auto it = std::find(t_HRBRIR_DataBase_EmitterPositions.begin(), t_HRBRIR_DataBase_EmitterPositions.end(), _emitterPosition);
			if (it == t_HRBRIR_DataBase_EmitterPositions.end()) {
				t_HRBRIR_DataBase_EmitterPositions.push_back(_emitterPosition);
			}
		}


		Common::CVector3 FindNearestListenerPosition(Common::CVector3& _listenerPosition) const {
			
			Common::CTransform _listenerLocation(_listenerPosition);						
			Common::CTransform nearestListenerLocation(t_HRBRIR_DataBase_ListenerPositions[0]);									
			float minDistance = _listenerLocation.GetVectorTo(nearestListenerLocation).GetDistance();
			
			for (auto it = t_HRBRIR_DataBase_ListenerPositions.begin() + 1; it != t_HRBRIR_DataBase_ListenerPositions.end(); it++) {
				
				float distance = _listenerLocation.GetVectorTo(Common::CTransform(*it)).GetDistance();
				if (distance < minDistance) {
					minDistance = distance;
					nearestListenerLocation = *it;
				}
			}
			return nearestListenerLocation.GetPosition();
		}


		Common::CVector3 FindNearestEmitterPosition(Common::CVector3& _listenerPosition) const {
			// For now we do nothing, we just return the first one.
			return t_HRBRIR_DataBase_EmitterPositions[0];		
		}




		///////////////
		// ATTRIBUTES
		///////////////	
		Common::CGlobalParameters globalParameters;

		std::string title;
		std::string databaseName;
		std::string listenerShortName;
		std::string fileName;
		
		int samplingRate;

		int32_t HRIRLength;								// HRIR vector length
		//int32_t bufferSize;								// Input signal buffer size		
		int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter
		//float distanceOfMeasurement;					//Distance where the HRIR have been measurement
		Common::CCranialGeometry cranialGeometry;					// Cranial geometry of the listener
		Common::CCranialGeometry originalCranialGeometry;		// Cranial geometry of the listener
		TEXTRAPOLATION_METHOD extrapolationMethod;	// Methods that is going to be used to extrapolate
		
		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly		
		bool enableWoodworthITD;					// Indicate the use of a customized delay

		int gridSamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap

		float sphereBorder;							// Define spheere "sewing"
		float epsilon_sewing;						// Define spheere "sewing"
		
		float azimuthMin;							// Variables that define limits of work area
		float azimuthMax;							// Variables that define limits of work area
		float elevationMin; 						// Variables that define limits of work area
		float elevationMax;							// Variables that define limits of work area
		float elevationNorth;						// Variables that define limits of work area
		float elevationSouth;						// Variables that define limits of work area


		// HRBRIR tables			
		T_HRBRIRTable					t_HRBRIR_DataBase;					// Store original data, normally read from SOFA file
		T_HRBRIRPartitionedTable		t_HRBRIR_Resampled_partitioned;		// Data in our grid, interpolated 
		std::unordered_map<orientation, float> stepVector;					// Store hrtf interpolated grids steps
		
		std::vector<Common::CVector3>	t_HRBRIR_DataBase_ListenerPositions;
		std::vector<Common::CVector3>	t_HRBRIR_DataBase_EmitterPositions;

		// Processors
		CQuasiUniformSphereDistribution quasiUniformSphereDistribution;
		CMidPointOnlineInterpolator midPointOnlineInterpolator;
		CSlopesMethodOnlineInterpolator slopesMethodOnlineInterpolator;
		COfflineInterpolation offlineInterpolation;
		CExtrapolation extrapolation;
	};
}
#endif
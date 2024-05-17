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
#include <ServiceModules/ServiceModuleInterfaces.hpp>

namespace BRTServices
{
	
	//typedef std::unordered_map<TVector3, T_HRTFTable> T_HRBRIRTable;
	typedef std::unordered_map<TDuplaVector3, T_HRTFTable> T_HRBRIRTable;


	class CHRBRIR : public CServicesBase
	{
	public:

		CHRBRIR() : setupInProgress{ false }, samplingRate{ 0 }, HRIRLength{-1}, bufferSize{-1}, gridSamplingStep{-1}, gapThreshold {DEFAULT_GAP_THRESHOLD}, title{""},
			databaseName{""}, listenerShortName{""}, fileName{""} {}



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
				//distanceOfMeasurement = _distance;
				//bufferSize = globalParameters.GetBufferSize();
				extrapolationMethod = _extrapolationMethod;

				//float partitions = (float)HRIRLength / (float)bufferSize;
				//HRIR_partitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));

				//elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
				//elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

				////Clear every table		
				t_HRBRIR_DataBase.clear();
				t_HRBRIR_DataBase_ListenerPositions.clear();
				//t_HRBRIR_DataBase_ListOfOrientations.clear();

				//t_HRTF_Resampled_frequency.clear();
				//t_HRTF_Resampled_partitioned.clear();

				//Change class state
				setupInProgress = true;
				//HRTFLoaded = false;


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
					//for (auto it = t_HRBRIR_DataBase.begin(); it != t_HRBRIR_DataBase.end(); it++) {						
					//	//RemoveCommonDelay_HRTFDataBaseTable();				// Delete the common delay of every HRIR functions of the DataBase Table											
					//	std::vector<orientation> orientationsList = offlineInterpolation.CalculateListOfOrientations(it->second);
					//	CalculateExtrapolation(it->second, orientationsList);	// Make the extrapolation if it's needed
					//	offlineInterpolation.CalculateTF_InPoles<T_HRTFTable, BRTServices::THRIRStruct>(it->second, HRIRLength, samplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromHemisphereParts());
					//	offlineInterpolation.CalculateTF_SphericalCaps<T_HRTFTable, BRTServices::THRIRStruct>(it->second, HRIRLength, gapThreshold, samplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());
					//}										

					
					
					////Creation and filling of resampling HRTF table
					//t_HRTF_DataBase_ListOfOrientations = offlineInterpolation.CalculateListOfOrientations(t_HRTF_DataBase);
					//quasiUniformSphereDistribution.CreateGrid<T_HRTFPartitionedTable, THRIRPartitionedStruct>(t_HRTF_Resampled_partitioned, stepVector, resamplingStep);
					//offlineInterpolation.FillResampledTable<T_HRTFTable, T_HRTFPartitionedTable, BRTServices::THRIRStruct, BRTServices::THRIRPartitionedStruct>(t_HRTF_DataBase, t_HRTF_Resampled_partitioned, bufferSize, HRIRLength, HRIR_partitioned_NumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_HRTFData(), CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());

					//////TESTING:
					////for (auto it = t_HRTF_Resampled_partitioned.begin(); it != t_HRTF_Resampled_partitioned.end(); it++) {
					////	if (it->second.leftHRIR_Partitioned.size() == 0 || it->second.rightHRIR_Partitioned.size() == 0) {
					////		SET_RESULT(RESULT_ERROR_NOTSET, "The t_HRTF_Resampled_partitioned has an empty HRIR in position [" + std::to_string(it->first.azimuth) + ", " + std::to_string(it->first.elevation) + "]");
					////	}
					////
					////}

					////Setup values
					//auto it = t_HRTF_Resampled_partitioned.begin();
					//HRIR_partitioned_SubfilterLength = it->second.leftHRIR_Partitioned[0].size();
					//setupInProgress = false;
					//HRTFLoaded = true;

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
		
		/**
		 * @brief Set the sampling step for the IR
		 * The intended angular separation between two adjacent points at the equator or two adjacent points in the median plane (azimuth=0)
		 * @param _samplingStep 
		 */
		void SetSamplingStep(int _resamplingStep) {
			gridSamplingStep = _resamplingStep;
		}

		/** \brief Ask for the sampling step
		*	\retval sampling step
		*/
		int GetSamplingStep() {
			return gridSamplingStep;
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


		void AddToListenersPositions(Common::CVector3 _listenerPosition) {			
			//Check if the listenerPosition is already in the table
			auto it = std::find(t_HRBRIR_DataBase_ListenerPositions.begin(), t_HRBRIR_DataBase_ListenerPositions.end(), _listenerPosition);
			if (it == t_HRBRIR_DataBase_ListenerPositions.end()){
				t_HRBRIR_DataBase_ListenerPositions.push_back(_listenerPosition);
			}			
		}

		void AddToEmitterPositions(Common::CVector3 _emitterPosition) {
			//Check if the listenerPosition is already in the table
			auto it = std::find(t_HRBRIR_DataBase_EmitterPositions.begin(), t_HRBRIR_DataBase_EmitterPositions.end(), _emitterPosition);
			if (it == t_HRBRIR_DataBase_EmitterPositions.end()) {
				t_HRBRIR_DataBase_EmitterPositions.push_back(_emitterPosition);
			}
		}

		///////////////
		// ATTRIBUTES
		///////////////	
		std::string title;
		std::string databaseName;
		std::string listenerShortName;
		std::string fileName;
		
		int samplingRate;

		int32_t HRIRLength;								// HRIR vector length
		int32_t bufferSize;								// Input signal buffer size		
		//int32_t HRIR_partitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		//int32_t HRIR_partitioned_SubfilterLength;		// Size of one HRIR subfilter
		//float distanceOfMeasurement;					//Distance where the HRIR have been measurement
		CCranialGeometry cranialGeometry;					// Cranial geometry of the listener
		CCranialGeometry originalCranialGeometry;		// Cranial geometry of the listener
		TEXTRAPOLATION_METHOD extrapolationMethod;	// Methods that is going to be used to extrapolate
		
		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		//bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		//bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int gridSamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		//bool enableWoodworthITD;					// Indicate the use of a customized delay
		int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap


		// HRBRIR tables			
		T_HRBRIRTable					t_HRBRIR_DataBase;		
		//std::vector<orientation>		t_HRBRIR_DataBase_ListOfOrientations;
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
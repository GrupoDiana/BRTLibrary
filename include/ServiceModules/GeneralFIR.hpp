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


#ifndef _CGeneralFIR_HPP_
#define _CGeneralFIR_HPP_

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/CranicalGeometry.hpp>
#include <ServiceModules/ServicesBase.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRTFAuxiliarMethods.hpp>
#include <ServiceModules/OfflineInterpolation.hpp>
#include <ServiceModules/OfflineInterpolationAuxiliarMethods.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>
#include <ServiceModules/KDTree.hpp>

namespace BRTBase { class CListener; }

namespace BRTServices
{	
	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CGeneralFIR : public CServicesBase
	{		
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CGeneralFIR()
			: IRLength{ 0 }			
			, FIRLoaded{ false }
			, setupInProgress{ false }
			, distanceOfMeasurement{ DEFAULT_HRTF_MEASURED_DISTANCE }
			, samplingRate{ -1 }
			, IR_TFpartitioned_NumberOfSubfilters { 0 }
			, IR_TFpartitioned_SubfilterLength { 0 }
			, numberOfEars { 0 }
		{ 
			serviceType = TServiceType::ir_database;
		}

		/** \brief Get size of each HRIR buffer
		*	\retval size number of samples of each HRIR buffer for one ear
		*   \eh Nothing is reported to the error handler.
		*/
		int32_t GetIRLength() const
		{
			return IRLength;
		}

		/**
		 * @brief Set the number of ears
		 * @param _numberOfEars number of ears
		 */
		void SetNumberOfEars(int & _numberOfEars) override {
			numberOfEars = _numberOfEars;
		}

		/**
		 * @brief get the number of ears
		 * @return number of ears
		 */
		int GetNumberOfEars() override {
			return numberOfEars;
		}
		
		/**
		 * @brief Set current cranial geometry as default
		 */
		/*void SetCranialGeometryAsDefault() override {
			originalCranialGeometry = cranialGeometry;
		}*/


		/** \brief Start a new HRTF configuration
		*	\param [in] _IRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool BeginSetup(int32_t _IRLength, BRTServices::TEXTRAPOLATION_METHOD _extrapolationMethod) override {		
			std::lock_guard<std::mutex> l(mutex);
			//Change class state
			setupInProgress = true;
			FIRLoaded = false;
			//Clear every table			
			t_IR_DataBase.clear();
			t_IRTF_Partitioned.clear();			

			//Update parameters			
			IRLength = _IRLength;							
			float partitions = (float)IRLength / (float)globalParameters.GetBufferSize();
			IR_TFpartitioned_NumberOfSubfilters = static_cast<int>(std::ceil(partitions));
							
			SET_RESULT(RESULT_OK, "General IR Setup started");
			return true;
		}

		/** \brief Set the full HRIR matrix.
		*	\param [in] newTable full table with all HRIR data
		*   \eh Nothing is reported to the error handler.
		*/
		void AddIRTable(T_HRTFTable&& newTable)
		{
			if (setupInProgress) {
				t_IR_DataBase = newTable;
			}
		}
		

		void AddHRIR(double _azimuth, double _elevation, double _distance, Common::CVector3 listenerPosition, THRIRStruct&& newHRIR) override {			
			if (setupInProgress) {				
				_azimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_azimuth);
				_elevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);						
				auto returnValue = t_IR_DataBase.emplace(orientation(_azimuth, _elevation, _distance), std::forward<THRIRStruct>(newHRIR));
				
				if (!returnValue.second) {
					// Error 
					if (t_IR_DataBase.find(orientation(_azimuth, _elevation, _distance)) != t_IR_DataBase.end()) {
						SET_RESULT(RESULT_WARNING, "Error emplacing IR in t_IR_DataBase map, already exists a HRIR in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
					} else {
						SET_RESULT(RESULT_WARNING, "Error emplacing IR in t_IR_DataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
					}
				}
			}
		}


		/** \brief Stop the HRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup() override {
			std::lock_guard<std::mutex> l(mutex);
						
			if (setupInProgress) {
				spatiallyOriented = false;
				if (!t_IR_DataBase.empty())
				{					
					distanceOfMeasurement = t_IR_DataBase.begin()->first.distance;	// Get first Distance as the distance of measurement //TODO Change
					
					if (t_IR_DataBase.size() > 1) spatiallyOriented = true;
					
					SetupPartitionedTable(); // Prepare and fill partitioned table

					SetupSearchTree(); // Prepare search Tree
					
					//Setup parameters
					auto it = t_IRTF_Partitioned.begin();
					IR_TFpartitioned_SubfilterLength = it->second.leftHRIR_Partitioned[0].size();
					setupInProgress = false;
					FIRLoaded = true;

					SET_RESULT(RESULT_OK, "The processing of the General IR matrix has been successfully completed.");
					return true;
				}
				else
				{
					// TO DO: Should be ASSERT?
					SET_RESULT(RESULT_ERROR_NOTSET, "The t_IR_DataBase map has not been set");
				}
			}
			return false;
		}

		

		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		//void EnableWoodworthITD() {	enableWoodworthITD = true;	}

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		//void DisableWoodworthITD() { enableWoodworthITD = false; }

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		//bool IsWoodworthITDEnabled() {	return enableWoodworthITD;	}
		
		/**
		 * @brief Get IR buffer in time domain for one ear by azimuth, elevation and distance
		 * @param _azimuth azimuth angle in degrees
		 * @param _elevation elevation angle in degrees
		 * @param _distance distance in meters
		 * @param _ear ear for which we want to get the IR
		 * @return IR buffer in time domain for the specified ear
		 */
		const CMonoBuffer<float> GetIRTimeDomain(const float & _azimuth, const float & _elevation, const float & _distance, const Common::T_ear & _ear) const override {
			std::lock_guard<std::mutex> l(mutex);
			CMonoBuffer<float> newHRIR;

			if (_ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get IR for a wrong ear (BOTH or NONE)");
				return newHRIR;
			}

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetIRTimeDomain: FIR Setup in progress return empty");
				return newHRIR;
			}
						
			auto it = t_IR_DataBase.find(orientation(_azimuth, _elevation, _distance));
			if (it != t_IR_DataBase.end()) {
				if (_ear == Common::T_ear::LEFT) {
					return it->second.leftHRIR;
				} else if (_ear == Common::T_ear::RIGHT) {
					return it->second.rightHRIR;
				}
			} else {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRTimeDomain: Requested azimuth and elevation not found in FIR table");
			}
			return newHRIR;	
		};
		
		/**
		 * @brief Get partitioned IR buffer for one ear for a non-spatially oriented FIR table
		 * @param ear for which we want to get the IR
		 * @return IR partitioned for the specified ear
		 */
		const std::vector<CMonoBuffer<float>> GetIRTFPartitioned(const Common::T_ear & _ear) const override {			
			std::lock_guard<std::mutex> l(mutex);
			std::vector<CMonoBuffer<float>> newHRIR;
						
			if (_ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get IR for a wrong ear (BOTH or NONE)");
				return newHRIR;
			}

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetIRPartitioned: FIR Setup in progress return empty");
				return newHRIR;
			}

			if (spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitioned: The FIR table is spatially oriented.");
				return newHRIR;
			}
			
			auto it = t_IRTF_Partitioned.begin();
			//auto it = t_IRTF_Partitioned.find(orientation(0, 0));
			if (it != t_IRTF_Partitioned.end()) {
				if (_ear == Common::T_ear::LEFT) {
					newHRIR = it->second.leftHRIR_Partitioned;
				} else if (_ear == Common::T_ear::RIGHT) {
					newHRIR = it->second.rightHRIR_Partitioned;
				} else {
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitioned: Attempt to get IR for a wrong ear");
				}
			} else {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRPartitioned: Requested azimuth and elevation not found in FIR table");
			}
			return newHRIR;	
		};

		void GetIRTFPartitioned2Ears(std::vector<CMonoBuffer<float>> & leftEarIRTF, std::vector<CMonoBuffer<float>> & rightEarIRTF) const override {
			std::lock_guard<std::mutex> l(mutex);
			
			leftEarIRTF = std::vector<CMonoBuffer<float>>();
			rightEarIRTF = std::vector<CMonoBuffer<float>>();
			
			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetIRPartitioned: FIR Setup in progress return empty");
				return;
			}

			if (spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitioned: The FIR table is spatially oriented.");
				return;
			}

			auto it = t_IRTF_Partitioned.begin();
			//auto it = t_IRTF_Partitioned.find(orientation(0, 0));
			if (it != t_IRTF_Partitioned.end()) {				
				leftEarIRTF = it->second.leftHRIR_Partitioned;
				rightEarIRTF = it->second.rightHRIR_Partitioned;				
			} else {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRPartitioned: Requested azimuth and elevation not found in FIR table");
			}			
		};

		/**
		 * @brief Get partitioned IR buffer for one ear by azimuth and elevation for a spatially oriented FIR table
		 * @param _azimuth azimuth angle in degrees
		 * @param _elevation elevation angle in degrees
		 * @param ear ear for which we want to get the IR
		 * @param _findNearest If an exact match is not found, find the nearest available HRIR
		 * @return IR partitioned for the specified ear
		 */
		const std::vector<CMonoBuffer<float>> GetIRTFPartitionedSpatiallyOriented(const float& _azimuth, const float& _elevation, const Common::T_ear& ear, bool _findNearest) const override		
		{
			std::lock_guard<std::mutex> l(mutex);
			std::vector<CMonoBuffer<float>> newHRIR;
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get IR for a wrong ear (BOTH or NONE)");
				return newHRIR;
			}

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRTF Setup in progress return empty");
				return newHRIR;
			}
			if (!spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitionedSpatiallyOriented: The FIR table is not spatially oriented.");
				return newHRIR;
			}
			
			THRIRPartitionedStruct foundData;
			auto it = t_IRTF_Partitioned.find(orientation(_azimuth, _elevation));
			if (it != t_IRTF_Partitioned.end()) {	
				// Exact match found
				foundData = it->second;				
			} else {
				if (!_findNearest) {
					SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRPartitionedSpatiallyOriented: Requested azimuth and elevation not found in FIR table");
					return newHRIR;
				}
				// No exact match, find nearest
				orientation nearest = searchTree.nearest(_azimuth, _elevation);
				auto it = t_IRTF_Partitioned.find(nearest);
				if (it != t_IRTF_Partitioned.end()) {
					foundData = it->second;					
				} else {
					// ERROR: This should not happen
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetHRIR_partitioned: KD-Tree returned an orientation not present in FIR table");					
				}								
			}	
			if (ear == Common::T_ear::LEFT) {
				return foundData.leftHRIR_Partitioned;
			} else if (ear == Common::T_ear::RIGHT) {
				return foundData.rightHRIR_Partitioned;
			} 
			return newHRIR;									
		}
		
		void GetIRTFPartitionedSpatiallyOriented2Ears(std::vector<CMonoBuffer<float>> & leftEarIRTF, std::vector<CMonoBuffer<float>> & rightEarIRTF, const float & _azimuth, const float & _elevation, bool _findNearest) const override 
		{ 
			std::lock_guard<std::mutex> l(mutex);
			
			leftEarIRTF = std::vector<CMonoBuffer<float>>();
			rightEarIRTF = std::vector<CMonoBuffer<float>>();

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: HRTF Setup in progress return empty");
				return;
			}
			if (!spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitionedSpatiallyOriented: The FIR table is not spatially oriented.");
				return;
			}

			THRIRPartitionedStruct foundData;
			auto it = t_IRTF_Partitioned.find(orientation(_azimuth, _elevation));
			if (it != t_IRTF_Partitioned.end()) {
				// Exact match found
				foundData = it->second;
			} else {
				if (!_findNearest) {
					SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRPartitionedSpatiallyOriented: Requested azimuth and elevation not found in FIR table");
					return;
				}
				// No exact match, find nearest
				orientation nearest = searchTree.nearest(_azimuth, _elevation);
				auto it = t_IRTF_Partitioned.find(nearest);
				if (it != t_IRTF_Partitioned.end()) {
					foundData = it->second;
				} else {
					// ERROR: This should not happen
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetHRIR_partitioned: KD-Tree returned an orientation not present in FIR table");
				}
			}
			leftEarIRTF = std::move(foundData.leftHRIR_Partitioned);
			rightEarIRTF = std::move(foundData.rightHRIR_Partitioned);			
		};

			
		/** \brief	Get the number of subfilters (blocks) in which the HRIR has been partitioned
		*	\retval n Number of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetIRTFNumberOfSubfilters() const {
			return IR_TFpartitioned_NumberOfSubfilters;
		}

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetIRTFSubfilterLength() const override {
			return IR_TFpartitioned_SubfilterLength;
		}

		/** \brief	Get if the HRTF has been loaded
		*	\retval isLoadead bool var that is true if the HRTF has been loaded
		*   \eh Nothing is reported to the error handler.
		*/		
		bool IsLoaded()
		{
			return FIRLoaded;
		}

		/** \brief Get raw HRTF table
		*	\retval table raw HRTF table
		*   \eh Nothing is reported to the error handler.
		*/		
		const T_HRTFTable& GetIRTimeDomainTable() const
		{
			return t_IR_DataBase;
		}

		/** \brief	Get the distance where the HRTF has been measured
		*   \return distance of the speakers structure to calculate the HRTF
		*   \eh Nothing is reported to the error handler.
		*/		
		float GetHRTFDistanceOfMeasurement() override {
			return distanceOfMeasurement;
		}

		///** \brief Set the title of the SOFA file
		//*    \param [in]	_title		string contains title
		//*/
		//void SetTitle(std::string _title) override {
		//	title = _title;
		//}

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		/*void SetDatabaseName(std::string _databaseName) override {
			databaseName = _databaseName;
		}*/

		/** \brief Set the title of the SOFA file
		*    \param [in]	_title		string contains title
		*/
		/*void SetListenerShortName(std::string _listenerShortName) override {
			listenerShortName = _listenerShortName;
		}*/


		/** \brief Set the name of the SOFA file 
		*    \param [in]	_fileName		string contains filename
		*/
		//void SetFilename(std::string _fileName) override {
		//	fileName = _fileName;
		//}

		///** \brief Get the name of the SOFA file 
		//*   \return string contains filename
		//*/
		//std::string GetFilename() override { 
		//	return fileName;
		//}

		/** \brief	Set the radius of the listener head
		*   \eh Nothing is reported to the error handler.
		*/
		//void SetHeadRadius(float _headRadius) override
		//{
		//	std::lock_guard<std::mutex> l(mutex);
		//	if (_headRadius >= 0.0f) {
		//		// First time this is called we save the original cranial geometry. A bit ugly but it works.
		//		if (originalCranialGeometry.GetHeadRadius() == -1) { originalCranialGeometry = cranialGeometry; }				
		//		cranialGeometry.SetHeadRadius(_headRadius);
		//	}
		//	else{
		//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Head Radius must be  greater than 0.");
		//	}
		//		
		//}

		/** \brief	Get the radius of the listener head
		*   \return listenerHeadRadius in meters
		*   \eh Nothing is reported to the error handler.
		*/
		/*float GetHeadRadius() override {
			return cranialGeometry.GetHeadRadius();
		}*/

		/**
		 * @brief Return to original ear positions and head radius.
		 */
		/*void RestoreHeadRadius() override {
			std::lock_guard<std::mutex> l(mutex);
			cranialGeometry = originalCranialGeometry;
		}*/

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) override {
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

		/**
		 * @brief Set current cranial geometry as default
		 */
		/*void SetCranialGeometryAsDefault() override {
			originalCranialGeometry = cranialGeometry;
		}*/

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

		/////////////
		// METHODS
		/////////////
		void SetupPartitionedTable() {
			// Prepare partitioned table
			for (auto & it : t_IR_DataBase) {
				t_IRTF_Partitioned.emplace(it.first, THRIRPartitionedStruct());
			}
			// Get IR in frequency Domain (partitioned) for each position of the table
			offlineInterpolation.FillResampledTable<T_HRTFTable, T_HRTFPartitionedTable, BRTServices::THRIRStruct, BRTServices::THRIRPartitionedStruct>(t_IR_DataBase, t_IRTF_Partitioned, globalParameters.GetBufferSize(), IRLength, IR_TFpartitioned_NumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_HRTFData(), CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());
		}
		
		void SetupSearchTree() {
			//Fill KD-Tree with the orientations of the resampled HRTF table
			std::vector<orientation> keys;
			keys.reserve(t_IRTF_Partitioned.size());
			for (const auto & kv : t_IRTF_Partitioned) {
				keys.push_back(kv.first);
			}
			searchTree.build(std::move(keys));
		
		}


		// Calculate and remove the common delay of every HRIR functions of the DataBase Table. Off line Method, called from EndSetUp()
		void RemoveCommonDelay_HRTFDataBaseTable() {
			//1. Init the minumun value with the fist value of the table
			auto it0 = t_IR_DataBase.begin();
			unsigned long minimumDelayLeft = it0->second.leftDelay; //Vrbl to store the minumun delay value for left ear
			unsigned long minimumDelayRight = it0->second.rightDelay; //Vrbl to store the minumun delay value for right ear

			//2. Find the common delay
			//Scan the whole table looking for the minimum delay for left and right ears
			for (auto it = t_IR_DataBase.begin(); it != t_IR_DataBase.end(); it++) {
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
			if (minimumDelayRight != 0 || minimumDelayLeft != 0) {
				for (auto it = t_IR_DataBase.begin(); it != t_IR_DataBase.end(); it++) {
					it->second.leftDelay = it->second.leftDelay - minimumDelayLeft; //Left ear
					it->second.rightDelay = it->second.rightDelay - minimumDelayRight; //Right ear
				}
			}
			//SET_RESULT(RESULT_OK, "Common delay deleted from HRTF table succesfully");
		}

		//}
		/**
		 * @brief Call the extrapolation method
		*/
		//void CalculateExtrapolation(std::vector<orientation>& _orientationList) {
		//	// Select the one that extrapolates with zeros or the one that extrapolates based on the nearest point according to some parameter.
		//	if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::zero_insertion) {
		//		SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation with zeros will be performed to fill it.");
		//		extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(t_IR_DataBase, _orientationList, IRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetZerosHRIR());
		//	}
		//	else if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::nearest_point) {
		//		SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded HRTF sofa file, an extrapolation will be made to the nearest point to fill it.");
		//		extrapolation.Process<T_HRTFTable, BRTServices::THRIRStruct>(t_IR_DataBase, _orientationList, IRLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetNearestPointHRIR());
		//	}
		//	else {
		//		SET_RESULT(RESULT_ERROR_NOTSET, "Extrapolation Method not set up.");
		//		// Do nothing
		//	}
		//}

		// Reset HRTF
		void Reset() {

			//Change class state
			setupInProgress = false;
			FIRLoaded = false;

			//Clear every table
			t_IR_DataBase.clear();
			t_IRTF_Partitioned.clear();

			//Update parameters
			IRLength = 0;
			//gridSamplingStep = DEFAULT_GRIDSAMPLING_STEP;
		}		


		///////////////
		// ATTRIBUTES
		///////////////		

		mutable std::mutex mutex;								// Thread management

		int32_t IRLength;								// HRIR vector length	
		int32_t IR_TFpartitioned_NumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t IR_TFpartitioned_SubfilterLength;		// Size of one HRIR subfilter
		float distanceOfMeasurement;					// Distance where the HRIR have been measurement		
		Common::CCranialGeometry cranialGeometry;					// Cranial geometry of the listener
		//Common::CCranialGeometry originalCranialGeometry;		// Cranial geometry of the listener
		//TEXTRAPOLATION_METHOD extrapolationMethod;		// Methods that is going to be used to extrapolate

		//float sphereBorder;								// Define spheere "sewing"
		//float epsilon_sewing;

		//float azimuthMin, azimuthMax, elevationMin, elevationMax, elevationNorth, elevationSouth;	// Variables that define limits of work area

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool FIRLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		//bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		//int gridSamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		//bool enableWoodworthITD;					// Indicate the use of a customized delay
		//int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap
		
		//std::string title;
		//std::string databaseName;
		//std::string listenerShortName;
		//std::string fileName;
		int samplingRate;
		int numberOfEars;

		// Tables							
		T_HRTFTable				t_IR_DataBase;				// Store original data, normally read from SOFA file
		T_HRTFPartitionedTable	t_IRTF_Partitioned;	// Data in our grid, interpolated 		
		KDTree<orientation> searchTree; // KDTree for fast nearest neighbor search

		// Empty object to return in some methods
		THRIRStruct						emptyHRIR;
		THRIRPartitionedStruct			emptyHRIR_partitioned;		
		Common::CGlobalParameters globalParameters;

		// Processors
		//CQuasiUniformSphereDistribution quasiUniformSphereDistribution;		
		COfflineInterpolation offlineInterpolation;
		//CExtrapolation extrapolation;		

		friend class CHRTFTester;
				
		
			
	};
}
#endif

/**
* \class CSphericalFIRTable
*
* \brief Declaration of CSphericalFIRTable class
* \date	Jan 2026
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _SPHERICAL_FIR_TABLE_HPP_
#define _SPHERICAL_FIR_TABLE_HPP_

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
#include <Common/IRWindowing.hpp>
#include <ServiceModules/ServicesBase.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRTFAuxiliarMethods.hpp>
#include <ServiceModules/OfflineInterpolation.hpp>
#include <ServiceModules/OfflineInterpolationAuxiliarMethods.hpp>
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>
#include <ServiceModules/SphericalSearchKDTree.hpp>

namespace BRTBase { class CListener; }

namespace BRTServices
{	
	/** \details This class gets impulse response data to compose HRTFs and implements different algorithms to interpolate the HRIR functions.
	*/
	class CSphericalFIRTable : public CServicesBase
	{		
	public:
		/** \brief Default Constructor
		*	\details By default, customized ITD is switched off, resampling step is set to 5 degrees and listener is a null pointer
		*   \eh Nothing is reported to the error handler.
		*/
		CSphericalFIRTable()
			: IRLength{ 0 }			
			//, dataReady{ false }
			, setupInProgress{ false }
			, customITD { false }
			//, samplingRate{ -1 }
			, partitionedFRNumberOfSubfilters { 0 }
			, partitionedFRSubfilterLength { 0 }
			, numberOfEars { 0 }			
			, cranialGeometry { Common::CCranialGeometry() }
			, originalCranialGeometry { Common::CCranialGeometry() }
			, fadeInBegin { 0 }
			, riseTime { 0 }
			, fadeOutCutoff { 0 }
			, fallTime { 0 }			
		{ 			
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
		void SetCranialGeometryAsDefault() override {
			originalCranialGeometry = cranialGeometry;
		}

		/** \brief Switch on ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableWoodworthITD() { customITD = true; }

		/** \brief Switch off ITD customization in accordance with the listener head radius
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableWoodworthITD() { customITD = false; }

		/** \brief Get the flag for HRTF cutomized ITD process
		*	\retval HRTFCustomizedITD if true, the HRTF ITD customization process based on the head circumference is enabled
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsWoodworthITDEnabled() { return customITD; }

		/** \brief	Get the number of subfilters (blocks) in which the HRIR has been partitioned
		*	\retval n Number of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/
		const int32_t GetTFNumberOfSubfilters() const {
			return partitionedFRNumberOfSubfilters;
		}

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/
		const int32_t GetTFSubfilterLength() const override {
			return partitionedFRSubfilterLength;
		}

		/** \brief	Get if the HRTF has been loaded
		*	\retval isLoadead bool var that is true if the HRTF has been loaded
		*   \eh Nothing is reported to the error handler.
		*/
		/*bool IsLoaded() {
			return dataReady;
		}*/

		/** \brief Get raw HRTF table
		*	\retval table raw HRTF table
		*   \eh Nothing is reported to the error handler.
		*/
		/*const T_HRTFTable& GetIRTimeDomainTable() const
		{
			return t_IR_DataBase;
		}*/

		/** \brief	Get the distance where the HRTF has been measured
		*   \return distance of the speakers structure to calculate the HRTF
		*   \eh Nothing is reported to the error handler.
		*/
		/*float GetHRTFDistanceOfMeasurement() override {
			return distanceOfMeasurement;
		}*/

		/** \brief	Set the radius of the listener head
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHeadRadius(float _headRadius) override {
			std::lock_guard<std::mutex> l(mutex);
			if (_headRadius >= 0.0f) {
				// First time this is called we save the original cranial geometry. A bit ugly but it works.
				if (originalCranialGeometry.GetHeadRadius() == -1) {
					originalCranialGeometry = cranialGeometry;
				}
				cranialGeometry.SetHeadRadius(_headRadius);
			} else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Head Radius must be greater than 0.");
			}
		}

		/** \brief	Get the radius of the listener head
		*   \return listenerHeadRadius in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() override {
			return cranialGeometry.GetHeadRadius();
		}

		/**
		 * @brief Return to original ear positions and head radius.
		 */
		void RestoreHeadRadius() override {
			std::lock_guard<std::mutex> l(mutex);
			cranialGeometry = originalCranialGeometry;
		}

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) override {
			if (_ear == Common::T_ear::LEFT) {
				cranialGeometry.SetLeftEarPosition(_earPosition);
			} else if (_ear == Common::T_ear::RIGHT) {
				cranialGeometry.SetRightEarPosition(_earPosition);
			} else {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
			}
		}

		/** \brief	Get the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) override {
			if (_ear == Common::T_ear::LEFT) {
				return cranialGeometry.GetLeftEarLocalPosition();
			} else if (_ear == Common::T_ear::RIGHT) {
				return cranialGeometry.GetRightEarLocalPosition();
			} else {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}
		}

		
		double GetDistanceOfMeasurement(const Common::CTransform & _referenceLocation, const double & _azimuth, const double & _elevation, const double & _distance) const override {
			
			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_referenceLocation, _distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return 0;
			}			
			float distance = distanceBucket->table.begin()->second.orientation.distance; // All the IRs in the same distance bucket have the same distance, so we can take the distance of the first one.
			return distance; 				
		}
		
		/**
		 * @brief Set parameters for the windowing IR process
		 * @param _windowThreshold The midpoint of the window in time (seconds), that is, where the window reaches 0.5.
		 * @param _windowRiseTime time (secons) for the window to go from 0 to 1. A value of zero would represent the step window. 
		 */
		void SetWindowingParameters(float _fadeInBegin, float _riseTime, float _fadeOutCutoff, float _fallTime) override {
			mutex.lock();
			fadeInBegin = _fadeInBegin;
			riseTime = _riseTime;
			fadeOutCutoff = _fadeOutCutoff;
			fallTime = _fallTime;

			mutex.unlock();
			if (dataReady) {
				setupInProgress = true;
				dataReady = false;				
				partitionedFRDataBase.clear();
				referencePositionSearchList.clear();
				EndSetup();
			}
		}

		/**
		 * @brief Get parameters for the windowing IR process
		 * @param [out] _windowThreshold 
		 * @param [out] _windowRiseTime 
		 */
		void GetWindowingParameters(float & _fadeInWindowThreshold, float & _fadeInWindowRiseTime, float & _fadeOutWindowThreshold, float & _fadeOutWindowRiseTime) override {
			_fadeInWindowThreshold = fadeInBegin;
			_fadeInWindowRiseTime = riseTime;
			_fadeOutWindowThreshold = fadeOutCutoff;
			_fadeOutWindowRiseTime = fallTime;
		}


		/** \brief Start a new HRTF configuration
		*	\param [in] _IRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/		
		bool BeginSetup(const int32_t & _IRLength, const BRTServices::TEXTRAPOLATION_METHOD & _extrapolationMethod) override {			
			
			std::lock_guard<std::mutex> l(mutex);
			
			//Change class state			
			setupInProgress = true;
			dataReady = false;
			spatiallyOriented = false;

			// Clear every table						
			sofaIRDataBase.clear();			
			partitionedFRDataBase.clear();
			referencePositionSearchList.clear();

			// Init parameters						
			IRLength = _IRLength;				
			partitionedFRNumberOfSubfilters = CalculateNumberOPartitions(IRLength);			
							
			SET_RESULT(RESULT_OK, "Spherical FIR Table - Setup started");
			return true;
		}

		/**
		 * @brief 
		 * @param _referencePosition 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _distance 
		 * @param _newIR 
		 */
		void AddIR(const Common::CVector3 & _referencePosition, const double & _azimuth, const double & _elevation, const double & _distance, THRIRStruct && _newIR) override { 
			//bool error = false;
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Cannot add IR - Setup not in progress");
				return;
			}			
			
			const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
			const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);

			TIRStruct newIRData(TOrientation(_azimuthInRage, _elevationInRange, _distance), std::forward<THRIRStruct>(_newIR));

			TSofaDataBucket newData;
			newData.referencePosition = _referencePosition;
			newData.data = std::forward<TIRStruct>(newIRData);
			sofaIRDataBase.push_back(std::forward<TSofaDataBucket>(newData));				
		}

		/** \brief Stop the HRTF configuration
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool EndSetup() override {
			std::lock_guard<std::mutex> l(mutex);
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Cannot end setup - Setup not in progress");
				return false;
			}
			if (serviceType == TServiceType::none) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Cannot end setup - Service type not defined");
				return false;
			}
			
			if (!sofaIRDataBase.empty())
			{										
				if (sofaIRDataBase.size() > 1) spatiallyOriented = true;					
					
				TRawSofaData windowingIRTable;
				CalculateWindowingIRTable(sofaIRDataBase, windowingIRTable);
				if (serviceType == TServiceType::hrir_database) {
					RemoveCommonDelayFromTable(windowingIRTable); 
				}					
				SetupPartitionedTable(windowingIRTable);	// Prepare and fill all the partitioned table					
				SortFRTableByDistance();					// Sort frequency domain table by distance from listener
				BuildSearchTrees();							// Prepare all Search Trees
				BuildReferenceListFromMap();				// Prepare reference position search list					

				//Setup parameters
				if (partitionedFRDataBase.size() != 0) {
					auto it = partitionedFRDataBase.begin();
					if (it->second.distances.size() != 0) { 
						if (it->second.distances.begin()->table.size() != 0) {
							if (it->second.distances.begin()->table.begin()->second.IR.left.size() != 0) {								
								partitionedFRSubfilterLength = it->second.distances.begin()->table.begin()->second.IR.left[0].size();								
								setupInProgress = false;
								dataReady = true;
								SET_RESULT(RESULT_OK, "The processing of the IR matrix has been successfully completed.");
								return true;
							}
						}											
					}					
				}																
			}						
			SET_RESULT(RESULT_ERROR_NOTSET, "EndSetup:: Processing of the FIR table has failed.");			
			return false;
		}
					
		/**
		 * @brief 
		 * @param _ear 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _runTimeInterpolation 
		 * @param _referenceLocation 
		 * @return 
		 */
		// TODO delete this method and use GetFR_PartitionedSpatiallyOriented instead
		const TFRPartitions GetHRIRPartitioned(Common::T_ear _ear, float _azimuth, float _elevation, bool _runTimeInterpolation, const Common::CTransform & _referenceLocation) const override {

			return GetFR_SpatiallyOriented(_azimuth, _elevation, 0, _referenceLocation, _ear, _runTimeInterpolation);
		}
				
		/**
		 * @brief 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _referenceLocation 
		 * @param ear 
		 * @param _findNearest 
		 * @return 
		 */
		const TFRPartitions GetFR_SpatiallyOriented(const float & _azimuth, const float & _elevation, const float & _distance, const Common::CTransform & _referenceLocation, const Common::T_ear & ear, bool _findNearest) const override		
		{
			//std::lock_guard<std::mutex> l(mutex);
			std::vector<CMonoBuffer<float>> _IR;
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get IR for a wrong ear (BOTH or NONE)");
				return _IR;
			}
			Common::CEarPair<TFRPartitions> foundData = GetFR_SpatiallyOriented_2Ears(_azimuth, _elevation, _distance, _referenceLocation, _findNearest);

			if (ear == Common::T_ear::LEFT) {
				return foundData.left;
			} else if (ear == Common::T_ear::RIGHT) {
				return foundData.right;
			}
		}
		
		/**
		 * @brief 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _referenceLocation 
		 * @param _findNearest 
		 * @return 
		 */
		const Common::CEarPair<TFRPartitions> GetFR_SpatiallyOriented_2Ears(const float & _azimuth, const float & _elevation, const float & _distance, const Common::CTransform & _referenceLocation, bool _findNearest) const override {		
			std::lock_guard<std::mutex> l(mutex);
			
			Common::CEarPair<TFRPartitions> foundData;			

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetFRPartitioned_SpatiallyOriented_2Ears: Service setup in progress");
				return foundData;
			}
			if (!spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetFRPartitioned_SpatiallyOriented_2Ears: The FIR table is not spatially oriented.");
				return foundData;
			}
			
			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_referenceLocation, _distance);			
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return foundData;
			}									
			
			// Find data in selected table
			TFRPartitionedStruct aux = GetDataFromPartitionedSpatiallyOriented(distanceBucket, _azimuth, _elevation, _findNearest);
						
			foundData.left = std::move(aux.IR.left);
			foundData.right = std::move(aux.IR.right);
			return foundData;			
		}

		
		const Common::CEarPair<TFRPartitions> GetFR_2Ears() const override {

			std::lock_guard<std::mutex> l(mutex);
			Common::CEarPair<TFRPartitions> foundData;

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetFR_Partitioned_2Ears: Service setup in progress");				
				return foundData;
			}
			if (spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetFR_Partitioned_2Ears: The FIR table is not spatially oriented.");
				return foundData;
			}
			
			Common::CVector3 _referenceLocation = Common::CVector3();
			float _azimuth = 0;
			float _elevation = 0;
			float _distance = 0;
			bool _findNearest = true;

			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_referenceLocation, _distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return foundData;
			}

			// Find data in selected table
			TFRPartitionedStruct aux = GetDataFromPartitionedSpatiallyOriented(distanceBucket, _azimuth, _elevation, _findNearest);

			foundData.left = std::move(aux.IR.left);
			foundData.right = std::move(aux.IR.right);
			return foundData;
		}


		////////////////

		// TODO delete this method and use GetFR_Delay instead
		THRIRPartitionedStruct GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool _findNearest, Common::CTransform & _referenceLocation) override { 
			Common::CEarPair<uint64_t> data = GetFR_Delay(_azimuthCenter, _elevationCenter, 0, _referenceLocation, _findNearest);
			THRIRPartitionedStruct result;
			result.leftDelay = data.left;
			result.rightDelay = data.right;
			return result;
		}

		/**
		 * @brief 
		 * @param _azimuthCenter 
		 * @param _elevationCenter 
		 * @param _referenceLocation 
		 * @param _findNearest 
		 * @return 
		 */
		
		const Common::CEarPair<uint64_t> GetFR_Delay(const float & _azimuthCenter, const float & _elevationCenter, const float & _distance, const Common::CTransform & _referenceLocation, bool _findNearest) const override {		
			std::lock_guard<std::mutex> l(mutex);

			Common::CEarPair<uint64_t> data { 0, 0 };			

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetFR_Delay: nonInterpolatedHRTF Setup in progress return empty");
				return data;
			}

			//Modify delay if customized delay is activate
			if (customITD)
			{
				data.left = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::LEFT);
				data.right = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::RIGHT);
				return data;
			}
			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_referenceLocation, _distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return data;
			}				
			// Find data in selected table
			TFRPartitionedStruct aux = GetDataFromPartitionedSpatiallyOriented(distanceBucket, _azimuthCenter, _elevationCenter, _findNearest);
			
			data.left = aux.delay.left;
			data.right = aux.delay.right;
			return data;
		}		


		std::vector<Common::CVector3> GetReferencePositions() const override { 
			std::vector<Common::CVector3> aux;
			for (const auto & refPair : partitionedFRDataBase) {
				aux.push_back(refPair.second.referencePos);
			}
			return aux;
		}
	private:	

		////////////////////
		// SETUP METHODS
		////////////////////

		/**
		 * @brief Calculate the number of subfilters needed to partition the HRIR
		 */
		int CalculateNumberOPartitions(int _irLength) {
			float partitions = (float)_irLength / (float)globalParameters.GetBufferSize();
			return static_cast<int>(std::ceil(partitions));
		}

		void SetupPartitionedTable(const TRawSofaData & _originalDataBase) {
			
			for (auto itRawData = _originalDataBase.begin(); itRawData != _originalDataBase.end(); itRawData++) {
				const Common::CVector3 _referencePosition = itRawData->referencePosition;
				const double _azimuth = itRawData->data.orientation.azimuth;
				const double _elevation = itRawData->data.orientation.elevation;
				const double _distance = itRawData->data.orientation.distance;

				const TVector3_key refKey(_referencePosition);
				// Check if the reference Position is already in the table
				auto refIt = partitionedFRDataBase.find(refKey);
				if (refIt == partitionedFRDataBase.end()) {
					// Create new reference position
					TReferenceBucket newRef;
					newRef.referenceKey = refKey;
					newRef.referencePos = _referencePosition;

					auto inserted = partitionedFRDataBase.emplace(refKey, std::move(newRef));
					if (!inserted.second) {
						SET_RESULT(RESULT_ERROR_BADALLOC, "Error emplacing IR into database. Reference Position [" + std::to_string(_referencePosition.x) + ", " + std::to_string(_referencePosition.y) + std::to_string(_referencePosition.z) + "], n position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + ", " + std::to_string(_distance) + "]");
						return;
					}
					refIt = inserted.first;
				}

				// Find o create orientation bucket
				TReferenceBucket & refBucket = refIt->second;
				TDistanceBucket * distanceBucket = nullptr;

				const int32_t distance_mm = quantise_dist_mm(_distance);
				for (auto & distBucketIt : refBucket.distances) {
					if (distBucketIt.distance_mm == distance_mm) {
						distanceBucket = &distBucketIt;
						break;
					}
				}

				if (!distanceBucket) {
					// Create new distance bucket
					TDistanceBucket newDist;
					newDist.distance_mm = distance_mm;
					refBucket.distances.push_back(std::move(newDist));
					distanceBucket = &refBucket.distances.back();
				}

				// Emplace new IR into orientation table
				const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
				const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);

				TFRPartitionedStruct newPartitionedIRData;
				CalculatePartitionedIR(itRawData->data, newPartitionedIRData, globalParameters.GetBufferSize(), partitionedFRNumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_FRData());
				//Emplace new IR into orientation table
				auto emplaced = distanceBucket->table.emplace(TOrientation_key(_azimuthInRage, _elevationInRange, _distance), std::move(newPartitionedIRData));
				if (!emplaced.second) {
					SET_RESULT(RESULT_ERROR_BADALLOC, "Error emplacing IR into database. Reference Position [" + std::to_string(_referencePosition.x) + ", " + std::to_string(_referencePosition.y) + std::to_string(_referencePosition.z) + "], n position [" + std::to_string(_azimuthInRage) + ", " + std::to_string(_elevationInRange) + ", " + std::to_string(_distance) + "]");					
				}
			}
		}
		
		/**
		 * @brief Sort the tables by distances, in any reference_position bucket
		 */
		void SortFRTableByDistance() {
			// 1) For each reference bucket:
			for (auto & refPair : partitionedFRDataBase) {
				TReferenceBucket & refBucket = refPair.second;

				// 1.1) Sort distance buckets by distance_mm
				std::sort(refBucket.distances.begin(), refBucket.distances.end(),
					[](const TDistanceBucket & a, const TDistanceBucket & b) {
						return a.distance_mm < b.distance_mm;
					});			
			}
		}

		void BuildSearchTrees() {
			for (auto & refPair : partitionedFRDataBase) {
				TReferenceBucket & refBucket = refPair.second;
				// 1.2) Build KD-tree for each distance bucket
				for (auto & distBucket : refBucket.distances) {
					// Collect orientations from the table keys
					std::vector<TOrientation> orientations;
					orientations.reserve(distBucket.table.size());

					for (const auto & kv : distBucket.table) {
						orientations.push_back(kv.second.orientation);
					}

					// Rebuild KD-tree (overwrites previous)
					distBucket.searchTree.build(std::move(orientations));
				}
			}
		}
		void BuildReferenceListFromMap() {
			
			referencePositionSearchList.clear();
			referencePositionSearchList.reserve(partitionedFRDataBase.size());

			for (const auto & refPair : partitionedFRDataBase) {
				const TVector3_key & key = refPair.first;				
				referencePositionSearchList.push_back(TReferenceEntry(key, key.x_q, key.y_q, key.z_q));
			}
		}

		
		template <typename Functor>
		void CalculatePartitionedIR(const TIRStruct & _inData, TFRPartitionedStruct & newTF_partitioned, int _bufferSize, int _numberOfSubfilters, Functor f1) {

			TFRPartitionedStruct partitionedData;
			newTF_partitioned = f1(_inData, _bufferSize, _numberOfSubfilters);
			/*newTF_partitioned.delay.left = partitionedData.leftDelay;
			newTF_partitioned.delay.right = partitionedData.rightDelay;
			newTF_partitioned.IR.left = std::move(partitionedData.leftHRIR_Partitioned);
			newTF_partitioned.IR.right = std::move(partitionedData.rightHRIR_Partitioned);*/
			newTF_partitioned.orientation = _inData.orientation;
		}

		/*template <typename Functor>
		void CalculatePartitionedIR(const TIRStruct & _inData, THRIRPartitionedStruct & newTF_partitioned, int _bufferSize, int _numberOfSubfilters, Functor f1) {									
			newTF_partitioned = f1(_inData, _bufferSize, _numberOfSubfilters);	
		}*/

		/*template <typename Functor>
		void FillPartitionedTable(const TSphericalFIRTable & originalTable, TSphericalFIRTablePartitioned & partitionedTable, int _bufferSize, int _numberOfSubfilters, Functor f1) {			
			for (auto & it : originalTable) {				
				THRIRPartitionedStruct newTF_partitioned = f1(it.second, _bufferSize, _numberOfSubfilters);				
				partitionedTable.emplace(it.first, std::forward<THRIRPartitionedStruct>(newTF_partitioned));				
			}			
		}*/

		/**
		 * @brief Prepare search tree table
		 */
		void SetupSearchsTree() {
			/*for (auto & kv : frequencyDomainPartitionedDataBase) {				
				std::vector<orientation> keys;
				keys.reserve(kv.second.table.size());
				for (const auto & kv2 : kv.second.table) {
					keys.push_back(kv2.second.originalOrientation);
				}
				kv.second.searchTree.build(std::move(keys));
			}*/
		}

		//void GetSpheresRadius() {			
		//	for (auto & kv : frequencyDomainPartitionedDataBase) {				
		//		for (const auto & kv2 : kv.second.table) {					
		//			auto it = std::find_if(kv.second.spheresRadius.begin(), kv.second.spheresRadius.end(),
		//				[&](double x) { return Common::AreSameDouble(x, kv2.second.originalOrientation.distance, DISTANCE_RESOLUTION_INV); });
		//			if (it == kv.second.spheresRadius.end()) {
		//				// TODO truncate distances to certain decimal places?
		//				kv.second.spheresRadius.push_back(kv2.second.originalOrientation.distance);
		//			}
		//		}				
		//		std::sort(kv.second.spheresRadius.begin(), kv.second.spheresRadius.end());
		//	}			
		//}


		//////////////////////////////////
		// FIND METHODS
		//////////////////////////////////
		
		/**
		 * @brief Find the nearest reference position stored in the data table (fast for ~10-20 refs).
		 * @param referencePosQuery reference position to compare
		 * @return 
		 */
		const TVector3_key FindNearestReferencePosition(const Common::CVector3 & referencePosQuery) const {
			const TVector3_key queryKey(referencePosQuery);
			
			if (referencePositionSearchList.empty())
				return TVector3_key {}; // no references loaded

			const int32_t qx = queryKey.x_q;
			const int32_t qy = queryKey.y_q;
			const int32_t qz = queryKey.z_q;

			int64_t bestD2 = std::numeric_limits<int64_t>::max();
			const TReferenceEntry * best = nullptr;

			for (const auto & r : referencePositionSearchList) {
				const int64_t dx = static_cast<int64_t>(r.x_mm) - qx;
				const int64_t dy = static_cast<int64_t>(r.y_mm) - qy;
				const int64_t dz = static_cast<int64_t>(r.z_mm) - qz;

				const int64_t d2 = dx * dx + dy * dy + dz * dz;
				if (d2 < bestD2) {
					bestD2 = d2;
					best = &r;
				}
			}

			return best ? best->key : TVector3_key {};
		}

		const TDistanceBucket * FindNearestDistanceBucket(const TReferenceBucket & refBucket, int32_t queryDistanceMm) const {
			const auto & v = refBucket.distances;
			if (v.empty())
				return nullptr;

			// refBucket.distances MUST be sorted by distance_mm (done in finalizeBuild()).
			auto it = std::lower_bound(v.begin(), v.end(), queryDistanceMm,
				[](const TDistanceBucket & b, int32_t key) { return b.distance_mm < key; }
			);
			
			// Nearest: choose closest between it and previous
			if (it == v.begin())
				return &(*it);

			if (it == v.end())
				return &v.back();

			const auto & hi = *it;
			const auto & lo = *(it - 1);

			const int32_t dLo = queryDistanceMm - lo.distance_mm;
			const int32_t dHi = hi.distance_mm - queryDistanceMm;

			return (dLo <= dHi) ? &lo : &hi; // tie: pick any (lo)
		}

		/**
		 * @brief Add a new reference position to the data table
		 * @param _position new reference position
		 */
		//void AddReferencePosition(const Common::CVector3 & _position) {
		//	//Check if the listenerPosition is already in the table
		//	auto it = std::find(referencePositionList.begin(), referencePositionList.end(), _position);
		//	if (it == referencePositionList.end()) {
		//		referencePositionList.push_back(_position);
		//	}
		//}

		/**
		 * @brief Find the nearest position of the listener stored in the data table.
		 * @param _listenerPosition listener position to compare
		 * @return Closest listener position
		 */
		/*Common::CVector3 FindNearestReferencePosition(const Common::CVector3 & _referencePosition) const {

			Common::CTransform _referenceLocation(_referencePosition);
			Common::CTransform nearestListenerLocation(referencePositionList[0]);
			float minDistance = _referenceLocation.GetVectorTo(nearestListenerLocation).GetSqrDistance();

			for (auto it = referencePositionList.begin() + 1; it != referencePositionList.end(); it++) {

				float distance = _referenceLocation.GetVectorTo(Common::CTransform(*it)).GetSqrDistance();
				if (distance < minDistance) {
					minDistance = distance;
					nearestListenerLocation = *it;
				}
			}
			return nearestListenerLocation.GetPosition();
		}*/

		///////////////////////
		// SEARCH TREE METHODS
		///////////////////////
				
		//bool FindNearestOrientationInSearchTree(const CSphericalSearchKDTree<orientation>& searchTree, orientation & _orientation, const Common::CVector3 & _referencePosition, const float & _azimuth, const float & _elevation) const {
		//	
		//	_orientation = searchTree.nearest(_azimuth, _elevation);
		//	
		//	
		//	auto it = searchTreeTable.find(TVector3(_referencePosition));
		//	if (it != searchTreeTable.end()) {				
		//		_orientation = it->second.nearest(_azimuth, _elevation);
		//		return true;
		//	} else {
		//		// ERROR: This should not happen
		//		SET_RESULT(RESULT_ERROR_NOTALLOWED, "FindNearestOrientationInSearchTree: No search tree found for the given reference position");
		//		_orientation = orientation();
		//		return false;
		//	}
		//}

		////////////////////////
		// Get AUX METHODS
		////////////////////////
		

		/**
		 * @brief Find distance bucket for a given reference location and distance
		 * @param _referenceLocation reference location
		 * @param _distance_m distance in meters
		 * @return 
		 */
		const TDistanceBucket * FindDistanceBucket(const Common::CTransform & _referenceLocation, const float & _distance_m) const {
			const TDistanceBucket * distanceBucket = nullptr;
			
			// Pick reference (exact or nearest)
			const TVector3_key nearestListenerPosition = FindNearestReferencePosition(_referenceLocation.GetPosition());
			auto referencePositionIt = partitionedFRDataBase.find(nearestListenerPosition);
			if (referencePositionIt == partitionedFRDataBase.end()) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Reference Position find error");
				return distanceBucket;
			}
			const TReferenceBucket & referencePositionBucket = referencePositionIt->second;

			// Pick distance bucket (exact or nearest)
			const int32_t qDistance_mm = quantise_dist_mm(_distance_m);
			distanceBucket = FindNearestDistanceBucket(referencePositionBucket, qDistance_mm);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return distanceBucket;
			}
			return distanceBucket;
		}

		const TFRPartitionedStruct GetDataFromPartitionedSpatiallyOriented(const TDistanceBucket * distanceBucket, const float & _azimuth, const float & _elevation, bool _findNearest) const {
			const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
			const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);
			
			TFRPartitionedStruct foundData;			
			auto it = distanceBucket->table.find(TOrientation_key(_azimuthInRage, _elevationInRange));
			if (it != distanceBucket->table.end()) {
				// Exact match found
				foundData = it->second;
			} else {
				// No exact match
				if (!_findNearest) {
					SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetDataFromPartitionedSpatiallyOriented: Requested azimuth and elevation not found in FIR table");
					return foundData;
				}
				// Find nearest
				TOrientation nearest = distanceBucket->searchTree.nearest(_azimuthInRage, _elevationInRange);			
				auto it = distanceBucket->table.find(TOrientation_key(nearest));
				if (it != distanceBucket->table.end()) {
					foundData = it->second;
				} else {
					// ERROR: This should not happen
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetDataFromPartitionedSpatiallyOriented: SearchTree returned an orientation not present in FIR table");
				}
			}			
			return foundData;
		}

		//const std::vector<CMonoBuffer<float>> GetFR_PartitionedSpatiallyOriented(const TSphericalFIRTablePartitioned & _table, const float & _azimuth, const float & _elevation, const Common::CTransform & _referenceLocation, const Common::T_ear & ear, bool _findNearest) const {

		//	std::vector<CMonoBuffer<float>> foundFR;

		//	THRIRPartitionedStruct foundData;
		//	auto it = _table.find(orientation(_azimuth, _elevation));
		//	if (it != _table.end()) {
		//		// Exact match found
		//		foundData = it->second;
		//	} else {
		//		// No exact match
		//		if (!_findNearest) {
		//			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRPartitionedSpatiallyOriented: Requested azimuth and elevation not found in FIR table");
		//			return foundFR;
		//		}
		//		// Find nearest
		//		orientation nearest;
		//		bool result = FindNearestOrientationInSearchTree(nearest, _referenceLocation.GetPosition(), _azimuth, _elevation);
		//		if (!result) {
		//			SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitionedSpatiallyOriented: Error finding nearest orientation in search tree");
		//			return foundFR;
		//		}
		//		auto it = _table.find(nearest);
		//		if (it != _table.end()) {
		//			foundData = it->second;
		//		} else {
		//			// ERROR: This should not happen
		//			SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetHRIR_partitioned: KD-Tree returned an orientation not present in FIR table");
		//		}
		//	}
		//	if (ear == Common::T_ear::LEFT) {
		//		return foundData.leftHRIR_Partitioned;
		//	} else if (ear == Common::T_ear::RIGHT) {
		//		return foundData.rightHRIR_Partitioned;
		//	}
		//	return foundFR;
		//}

		//const THRIRPartitionedStruct GetFR_Delay(const TSphericalFIRTablePartitioned & _table, const float& _azimuthCenter, const float & _elevationCenter, const Common::CTransform & _referenceLocation, bool _findNearest)
		//{
		//	THRIRPartitionedStruct delay;
		//	
		//	auto it = _table.find(orientation(_azimuthCenter, _elevationCenter));
		//	if (it != _table.end()) {
		//		// Exact match found				
		//		delay = it->second;
		//	} else {
		//		// No exact match
		//		if (!_findNearest) {
		//			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetIRPartitionedSpatiallyOriented: Requested azimuth and elevation not found in FIR table");
		//			return delay;
		//		}
		//		// Find nearest
		//		orientation nearest;
		//		bool result = FindNearestOrientationInSearchTree(nearest, _referenceLocation.GetPosition(), _azimuth, _elevation);
		//		if (!result) {
		//			SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetIRPartitionedSpatiallyOriented: Error finding nearest orientation in search tree");
		//			return delay;
		//		}
		//		auto it = _table.find(nearest);
		//		if (it != _table.end()) {					
		//			delay.leftDelay = it->second.leftDelay;
		//			delay.rightDelay = it->second.rightDelay;
		//		} else {
		//			// ERROR: This should not happen
		//			SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetHRIR_partitioned: KD-Tree returned an orientation not present in FIR table");
		//		}
		//	}			
		//	return delay;
		//}

		///////////////////
		// Common Delay
		///////////////////
		// 		
		/**
		 * @brief Calculate and remove the common delay of every IR functions of the DataBase Table. 
		 */
		void RemoveCommonDelayFromTable(TRawSofaData & table) {									
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
		

		/**
		 * @brief Check if the windowing process is configured
		 * @return 
		 */
		bool IsFadeInWindowingConfigured() {
			return fadeInBegin != 0 || riseTime != 0;
		}
		/**
		 * @brief Check if the windowing process is configured
		 * @return 
		 */
		bool IsFadeOutWindowingConfigured() {
			return fadeOutCutoff != 0 || fallTime != 0;
		}


		void CalculateWindowingIRTable(const TRawSofaData & _inTable, TRawSofaData & _outTable) {
			_outTable.clear();
			_outTable = _inTable;
			
			if (IsFadeInWindowingConfigured()) {
				for (auto it = _outTable.begin(); it != _outTable.end(); it++) {					
					it->data.IR.left = std::move(Common::CIRWindowing::Proccess(it->data.IR.left, Common::CIRWindowing::fadein, fadeInBegin, riseTime, globalParameters.GetSampleRate()));
					it->data.IR.right = std::move(Common::CIRWindowing::Proccess(it->data.IR.right, Common::CIRWindowing::fadein, fadeInBegin, riseTime, globalParameters.GetSampleRate()));					
				}
			}
			if (IsFadeOutWindowingConfigured()) {
				for (auto it = _outTable.begin(); it != _outTable.end(); it++) {
					it->data.IR.left = std::move(Common::CIRWindowing::Proccess(it->data.IR.left, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, globalParameters.GetSampleRate()));
					it->data.IR.right = std::move(Common::CIRWindowing::Proccess(it->data.IR.right, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, globalParameters.GetSampleRate()));
				}

				// Update impulseResponseLength and the number of subfilters
				IRLength = _outTable.begin()->data.IR.left.size();
				partitionedFRNumberOfSubfilters = CalculateNumberOPartitions(IRLength);
			}
		}

		/**
		 * @brief Create a new table with the same data as the input table but with the IRs windowed
		 * @param _inTable 
		 * @param _outTable 
		 */
		//void CalculateWindowingIRTable(const TSphericalFIRTable & _inTable, TSphericalFIRTable & _outTable) {
		//	_outTable.clear();
		//	_outTable = _inTable;

		//	if (IsFadeInWindowingConfigured()) {
		//		for (auto it = _outTable.begin(); it != _outTable.end(); it++) {					
		//			it->second.IR.left = std::move(Common::CIRWindowing::Proccess(it->second.IR.left, Common::CIRWindowing::fadein, fadeInBegin, riseTime, globalParameters.GetSampleRate()));
		//			it->second.IR.right = std::move(Common::CIRWindowing::Proccess(it->second.IR.right, Common::CIRWindowing::fadein, fadeInBegin, riseTime, globalParameters.GetSampleRate()));
		//		}
		//	}

		//	if (IsFadeOutWindowingConfigured()) {
		//		for (auto it = _outTable.begin(); it != _outTable.end(); it++) {
		//			it->second.IR.left = std::move(Common::CIRWindowing::Proccess(it->second.IR.left, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, globalParameters.GetSampleRate()));
		//			it->second.IR.right = std::move(Common::CIRWindowing::Proccess(it->second.IR.right, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, globalParameters.GetSampleRate()));
		//		}

		//		// Update impulseResponseLength and the number of subfilters
		//		IRLength = _outTable.begin()->second.IR.left.size();				
		//		IR_TFpartitioned_NumberOfSubfilters = CalculateNumberOPartitions(IRLength);
		//	}
		//}

		///////////////
		// ATTRIBUTES
		///////////////		
		mutable std::mutex mutex;								// Thread management
		
		Common::CGlobalParameters globalParameters; // Global parameters of the service

		int32_t IRLength;								// HRIR vector length	
		int32_t partitionedFRNumberOfSubfilters;	// Number of subfilters (blocks) for the UPC algorithm
		int32_t partitionedFRSubfilterLength;		// Size of one HRIR subfilter
		//float distanceOfMeasurement;					// Distance where the HRIR have been measurement		
		Common::CCranialGeometry cranialGeometry;					// Cranial geometry of the listener
		Common::CCranialGeometry originalCranialGeometry;		// Cranial geometry of the listener
		//TEXTRAPOLATION_METHOD extrapolationMethod;		// Methods that is going to be used to extrapolate

		//float sphereBorder;								// Define spheere "sewing"
		//float epsilon_sewing;

		//float azimuthMin, azimuthMax, elevationMin, elevationMax, elevationNorth, elevationSouth;	// Variables that define limits of work area

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		//bool dataReady;							// Variable that indicates if the HRTF has been loaded correctly
		//bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		//int gridSamplingStep; 						// HRTF Resample table step (azimuth and elevation)
		bool customITD;					// Indicate the use of a customized delay
		//int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap
		
		
		//int samplingRate;							// Sampling rate of the IR
		int numberOfEars;							// Number of ears
		float fadeInBegin;							// Variable to be used in the windowing IR process
		float riseTime;								// Variable to be used in the windowing IR process
		float fadeOutCutoff;						// Variable to be used in the windowing IR process
		float fallTime;								// Variable to be used in the windowing IR process 

		// Tables									
		TRawSofaData sofaIRDataBase;					// Time domain database - orginal data from SOFA file		
		TReferenceBucketMap partitionedFRDataBase;		// Frequency domain partitioned database
		TReferenceEntryList referencePositionSearchList; // List of reference positions for nearest search (built from the map keys)					
	};
}
#endif

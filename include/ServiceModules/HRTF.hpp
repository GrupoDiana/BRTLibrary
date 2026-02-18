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
#include <Common/IRWindowing.hpp>
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
			:enableWoodworthITD{ false }
			, gridSamplingStep{ DEFAULT_GRIDSAMPLING_STEP }
			, gapThreshold{ DEFAULT_GAP_THRESHOLD }			
			, HRTFLoaded{ false }
			, setupInProgress{ false }			
			, azimuthMin{ DEFAULT_MIN_AZIMUTH }
			, azimuthMax{ DEFAULT_MAX_AZIMUTH }
			, elevationMin{ DEFAULT_MIN_ELEVATION }
			, elevationMax{ DEFAULT_MAX_ELEVATION }
			, sphereBorder{ SPHERE_BORDER }
			, epsilon_sewing{ EPSILON_SEWING }			
			, elevationNorth{ 0 }
			, elevationSouth{ 0 }
			, fadeInBegin { 0 }
			, riseTime { 0 }
			, fadeOutCutoff { 0 }
			, fallTime { 0 }
			, extrapolationMethod{ TEXTRAPOLATION_METHOD::nearest_point }
		{ }

		/** \brief Get size of each HRIR buffer
		*	\retval size number of samples of each HRIR buffer for one ear
		*   \eh Nothing is reported to the error handler.
		*/
		/*int32_t GetIRLength() const
		{
			return impulseResponseLength;
		}*/

		/**
		 * @brief Set sampling step for HRIR resampling
		 * @param _resamplingStep 
		 */
		void SetGridSamplingStep(int _samplingStep) override {
			gridSamplingStep = _samplingStep;
		}
		/**
		 * @brief Get sampling step defined for HRIR resampling
		 * @return 
		 */
		int GetGridSamplingStep() const override {
			return gridSamplingStep;
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
				
		/** \brief	Get the number of subfilters (blocks) in which the HRIR has been partitioned
		*	\retval n Number of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetNumberOfSubfiltersFR() const override {
			return partitionedFRNumberOfSubfilters;
		}

		/** \brief	Get the size of subfilters (blocks) in which the HRIR has been partitioned, every subfilter has the same size
		*	\retval size Size of HRIR subfilters
		*   \eh Nothing is reported to the error handler.
		*/		
		const int32_t GetSubfilterLengthFR() const override {
			return partitionedFRSubfilterLength;
		}


		/** \brief	Get the distance where the HRTF has been measured
		*   \return distance of the speakers structure to calculate the HRTF
		*   \eh Nothing is reported to the error handler.
		*/		
		double GetDistanceOfMeasurement(const Common::CTransform & _referenceLocation, const double & _azimuth, const double & _elevation, const double & _distance) const override {			
			
			if (!dataReady) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "GetDistanceOfMeasurement: HRTF data not ready");
				return 0;
			}
			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetDistanceOfMeasurement: Distance Bucket find error");
				return 0;
			}
			if (distanceBucket->table.size() == 0) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetDistanceOfMeasurement: Distance Bucket empty");
				return 0;
			}
			float distance = distanceBucket->table.begin()->second.orientation.distance; // All the IRs in the same distance bucket have the same distance, so we can take the distance of the first one.
			return distance; 	
		}

				

		/** \brief	Set the radius of the listener head
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHeadRadius(float _headRadius) override
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

		/** \brief	Get the radius of the listener head in meters, truncated and rounded to 4 decimals to avoid precision errors
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
		void SetCranialGeometryAsDefault() override {
			originalCranialGeometry = cranialGeometry;
		}

		/** \brief	Get the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		Common::CVector3 GetEarLocalPosition(Common::T_ear _ear) override {			
			if (_ear == Common::T_ear::LEFT)		{ return cranialGeometry.GetLeftEarLocalPosition(); }
			else if (_ear == Common::T_ear::RIGHT) { return cranialGeometry.GetRightEarLocalPosition(); }
			else 
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}
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
				distanceFRTable.clear();
				stepVector.clear();
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
		*	\param [in] _HRIRLength buffer size of the HRIR to be added
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		bool BeginSetup(const int32_t & _HRIRLength, const BRTServices::TEXTRAPOLATION_METHOD & _extrapolationMethod) override {
			std::lock_guard<std::mutex> l(mutex);
			// Change class state
			setupInProgress = true;
			HRTFLoaded = false;

			// Clear every table
			stepVector.clear();
			distanceFRTable.clear();
			sofaIRDataBase2.clear();

			//Update parameters
			impulseResponseLength = _HRIRLength;
			extrapolationMethod = _extrapolationMethod;			
			partitionedFRNumberOfSubfilters = CalculateNumberOPartitions(impulseResponseLength);
			elevationNorth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			elevationSouth = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

			SET_RESULT(RESULT_OK, "nonInterpolatedHRTF Setup started");
			return true;
		}

		void AddIR(const Common::CVector3 & _referencePosition, const double & _azimuth, const double & _elevation, const double & _distance, THRIRStruct && _newIR) override {
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Cannot add IR - Setup not in progress");
				return;
			}

			const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
			const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);

			TIRStruct newIRData(TOrientation(_azimuthInRage, _elevationInRange, _distance), std::forward<THRIRStruct>(_newIR));

			sofaIRDataBase2.push_back(std::forward<TIRStruct>(newIRData));
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

			if (sofaIRDataBase2.empty()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "ERROR SphericalFIRTable::EndSetup - No data to be processed");
				return false;
			}
			if (sofaIRDataBase2.size() > 1) spatiallyOriented = true;

			std::vector<BRTServices::TIRStruct> windowingIRTable;
			CalculateWindowingIRTable(sofaIRDataBase2, windowingIRTable);
			if (serviceType == TServiceType::hrir_database_interpolated) {
				RemoveCommonDelayFromTable(windowingIRTable);
			}

			TRawSofaTableByDistances sofaIRDatabaseByDistances;
			SlipRawDataByDistances(sofaIRDataBase2, sofaIRDatabaseByDistances);

			distanceFRTable.clear();
			for (auto & distBucketIt : sofaIRDatabaseByDistances) {
				std::vector<TOrientation> _orientationList = offlineInterpolation.CalculateListOfOrientations(distBucketIt.table);
				CalculateExtrapolation(distBucketIt.table, _orientationList); // Make the extrapolation if it's needed
				offlineInterpolation.CalculateTF_InPoles<TRawSofaTable, BRTServices::TIRStruct>(distBucketIt.table, impulseResponseLength, gridSamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromHemisphereParts());
				offlineInterpolation.CalculateTF_SphericalCaps<TRawSofaTable, BRTServices::TIRStruct>(distBucketIt.table, impulseResponseLength, gapThreshold, gridSamplingStep, CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());

				TDistanceBucket aux;
				aux.distance_mm = distBucketIt.distance_mm;

				//Creation and filling of resampling HRTF table
				CQuasiUniformSphereDistribution::CreateGrid<TSphericalFIRTablePartitioned, TFRPartitionedStruct>(aux.table, stepVector, gridSamplingStep, distBucketIt.distance);
				offlineInterpolation.FillResampledTable<TRawSofaTable, TSphericalFIRTablePartitioned, BRTServices::TIRStruct, BRTServices::TFRPartitionedStruct>(distBucketIt.table, aux.table, globalParameters.GetBufferSize(), impulseResponseLength, partitionedFRNumberOfSubfilters, CHRTFAuxiliarMethods::SplitAndGetFFT_HRTFData(), CHRTFAuxiliarMethods::CalculateHRIRFromBarycentrics_OfflineInterpolation());

				// Add to vector of tables
				distanceFRTable.push_back(std::move(aux));
			}

			//Setup values
			if (!distanceFRTable.empty()) {
				auto it = distanceFRTable.begin();
				if (!it->table.empty()) {
					partitionedFRSubfilterLength = it->table.begin()->second.IR.left[0].size();
					//partitionedFRSubfilterLength = it->table> second.IR.left[0].size();
					setupInProgress = false;
					HRTFLoaded = true;

					SET_RESULT(RESULT_OK, "Non-Interpolated HRTF Matrix resample completed succesfully");
					return true;
				}
			}
			return false;
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
		const std::vector<CMonoBuffer<float>> GetHRIRPartitioned(Common::T_ear _ear, float _azimuth, float _elevation, bool _runTimeInterpolation, const Common::CTransform &  _referenceLocation) const override {						
			return GetFR_SpatiallyOriented(_azimuth, _elevation, 0, Common::CTransform(), _ear, _runTimeInterpolation);
			/*return CHRTFAuxiliarMethods::GetHRIRFromPartitionedTable(t_HRTF_Resampled_partitioned, ear, _azimuth, _elevation, runTimeInterpolation,
				partitionedFRNumberOfSubfilters, partitionedFRSubfilterLength, stepVector);*/
		}


		const TFRPartitions GetFR_SpatiallyOriented(const float & _azimuth, const float & _elevation, const float & _distance, const Common::CTransform & _referenceLocation, const Common::T_ear & ear, bool _runTimeInterpolation) const override { 
			
			std::lock_guard<std::mutex> l(mutex);
			TFRPartitions _foundData;

			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get HRIR for a wrong ear (BOTH or NONE)");
				return _foundData;
			}

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: nonInterpolatedHRTF Setup in progress return empty");
				return _foundData;
			}
						
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get IR for a wrong ear (BOTH or NONE)");
				return _foundData;
			}
			
			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return _foundData;
			}		

			TFRPartitions foundData;
			foundData = CHRTFAuxiliarMethods::GetHRIRFromPartitionedTable(distanceBucket->table, ear, _azimuth, _elevation, _runTimeInterpolation,
				partitionedFRNumberOfSubfilters, partitionedFRSubfilterLength, stepVector);
			return foundData;										
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
		THRIRPartitionedStruct GetHRIRDelay(Common::T_ear ear, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation, Common::CTransform & _referenceLocation) {
			
			Common::CEarPair<uint64_t> data = GetFR_Delay(_azimuthCenter, _elevationCenter, 0, _referenceLocation, runTimeInterpolation);
			
			THRIRPartitionedStruct result;
			result.leftDelay = data.left;
			result.rightDelay = data.right;
			return result;
			//std::lock_guard<std::mutex> l(mutex);
			//THRIRPartitionedStruct data;

			//if (setupInProgress) {
			//	SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIRDelay: nonInterpolatedHRTF Setup in progress return empty");
			//	return data;
			//}

			//// Modify delay if customized delay is activate
			//if (enableWoodworthITD) {
			//	data.leftDelay = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::LEFT);
			//	data.rightDelay = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::RIGHT);
			//	return data;
			//}

			//TFRPartitionedStruct aux = CHRTFAuxiliarMethods::GetHRIRDelayFromPartitioned(t_HRTF_Resampled_partitioned, ear, _azimuthCenter, _elevationCenter, runTimeInterpolation,
			//	partitionedFRNumberOfSubfilters, partitionedFRSubfilterLength, stepVector);

			//data.leftDelay = aux.delay.left;
			//data.rightDelay = aux.delay.right;
			//return data;
		}

		const Common::CEarPair<uint64_t> GetFR_Delay(const float & _azimuthCenter, const float & _elevationCenter, const float & _distance, const Common::CTransform & _referenceLocation, bool _runTimeInterpolation) const override { 
			
			std::lock_guard<std::mutex> l(mutex);			
			Common::CEarPair<uint64_t> foundData { 0, 0 };

			if (setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTSET, "GetHRIR_partitioned: nonInterpolatedHRTF Setup in progress return empty");
				return foundData;
			}
			
			// Modify delay if customized delay is activate
			if (enableWoodworthITD) {
				foundData.left = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::LEFT);
				foundData.right = CHRTFAuxiliarMethods::CalculateCustomizedDelay(_azimuthCenter, _elevationCenter, cranialGeometry, Common::T_ear::RIGHT);
				return foundData;
			}

			// Find Table to use if exists
			const TDistanceBucket * distanceBucket = FindDistanceBucket(_distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return foundData;
			}		

			
			foundData = CHRTFAuxiliarMethods::GetHRIRDelayFromPartitioned_2Ears(distanceBucket->table, _azimuthCenter, _elevationCenter, _runTimeInterpolation,
				partitionedFRNumberOfSubfilters, partitionedFRSubfilterLength, stepVector);
			
			return foundData;
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
		/**
		 * @brief Slip raw IR data into different tables according to the distance of the measurement. 
		 * Every table will be used to create a resampled and partitioned HRTF table for that distance.
		 * @param _inputData Raw IR data read from the sofa file
		 * @param _outpuTable vector of tables sliped by distance
		 */
		void SlipRawDataByDistances(const std::vector<BRTServices::TIRStruct> & _inputData, TRawSofaTableByDistances & _outpuTable) {
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Cannot add IR - Setup not in progress");
				return;
			}

			for (auto & dataIt : _inputData) {
				const double & _azimuth = dataIt.orientation.azimuth;
				const double & _elevation = dataIt.orientation.elevation;
				const double & _distance = dataIt.orientation.distance;
				TIRStruct newIRData = dataIt;
				// Find o create orientation bucket
				TDistanceBucketRawSofa * distanceBucket = nullptr;

				const int32_t distance_mm = quantise_dist_mm(_distance);
				for (auto & distBucketIt : _outpuTable) {
					if (distBucketIt.distance_mm == distance_mm) {
						distanceBucket = &distBucketIt;
						break;
					}
				}

				if (!distanceBucket) {
					// Create new distance bucket
					TDistanceBucketRawSofa newDist;	
					newDist.distance = _distance;
					newDist.distance_mm = distance_mm;
					_outpuTable.push_back(std::move(newDist));
					distanceBucket = &_outpuTable.back();
				}

				// Emplace new IR into orientation table
				const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
				const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);

				auto returnValue = distanceBucket->table.emplace(TOrientation(_azimuth, _elevation, _distance), std::forward<TIRStruct>(newIRData));
				//Error handler
				if (!returnValue.second) {
					if (distanceBucket->table.find(TOrientation(_azimuth, _elevation, _distance)) != distanceBucket->table.end()) {
						SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in sofaIRDataBase map, already exists a HRIR in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
					} else {
						SET_RESULT(RESULT_WARNING, "Error emplacing HRIR in sofaIRDataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
					}
				}
			}
			// Sort distance buckets by distance_mm
			std::sort(_outpuTable.begin(), _outpuTable.end(),
				[](const TDistanceBucketRawSofa & a, const TDistanceBucketRawSofa & b) {
					return a.distance_mm < b.distance_mm;
				});
		}		
																
		/**
		 * @brief Call the extrapolation method. Fill the gaps of the table with the selected extrapolation method.		 
		*/
		void CalculateExtrapolation(TRawSofaTable & _table, std::vector<TOrientation> & _orientationList) {
			// Select the one that extrapolates with zeros or the one that extrapolates based on the nearest point according to some parameter.			
			if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::zero_insertion) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded nonInterpolatedHRTF sofa file, an extrapolation with zeros will be performed to fill it.");
				extrapolation.Process<TRawSofaTable, BRTServices::TIRStruct>(_table, _orientationList, impulseResponseLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetZerosHRIR());
			}
			else if (extrapolationMethod == BRTServices::TEXTRAPOLATION_METHOD::nearest_point) {
				SET_RESULT(RESULT_WARNING, "At least one large gap has been found in the loaded nonInterpolatedHRTF sofa file, an extrapolation will be made to the nearest point to fill it.");
				extrapolation.Process<TRawSofaTable, BRTServices::TIRStruct>(_table, _orientationList, impulseResponseLength, DEFAULT_EXTRAPOLATION_STEP, CHRTFAuxiliarMethods::GetNearestPointHRIR());
			}
			else {
				SET_RESULT(RESULT_ERROR_NOTSET, "Extrapolation Method not set up.");
				// Do nothing
			}									
		}


		///////////////////
		// Common Delay
		///////////////////		
		/**
		 * @brief Calculate and remove the common delay of every IR functions of the DataBase Table. 
		 */
		void RemoveCommonDelayFromTable(std::vector<BRTServices::TIRStruct> & table) {
			//1. Init the minumun value with the fist value of the table
			auto it0 = table.begin();
			uint64_t minimumDelayLeft = it0->delay.left; //Vrbl to store the minumun delay value for left ear
			uint64_t minimumDelayRight = it0->delay.right; //Vrbl to store the minumun delay value for right ear

			//2. Find the common delay
			//Scan the whole table looking for the minimum delay for left and right ears
			for (auto it = table.begin(); it != table.end(); it++) {
				//Left ear
				if (it->delay.left < minimumDelayLeft) {
					minimumDelayLeft = it->delay.left;
				}
				//Right ear
				if (it->delay.right < minimumDelayRight) {
					minimumDelayRight = it->delay.right;
				}
			}
			//3. Delete the common delay
			//Scan the whole table substracting the common delay to every delays for both ears separately
			//The common delay of each canal have been calculated and subtracted separately in order to correct the asymmetry of the measurement
			if (minimumDelayRight != 0 || minimumDelayLeft != 0) {
				for (auto it = table.begin(); it != table.end(); it++) {
					it->delay.left -= minimumDelayLeft; //Left ear
					it->delay.right -= minimumDelayRight; //Right ear
				}
			}
			SET_RESULT(RESULT_OK, "Common delay deleted (" + std::to_string(minimumDelayLeft) + "," + std::to_string(minimumDelayRight) + ") from nonInterpolatedHRTF table succesfully");
		}

		///////////////////
		// Truncate IRs
		///////////////////
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

		void CalculateWindowingIRTable(const std::vector<BRTServices::TIRStruct> & _inTable, std::vector<BRTServices::TIRStruct> & _outTable) {
			_outTable.clear();
			_outTable = _inTable;

			if (IsFadeInWindowingConfigured()) {
				for (auto it = _outTable.begin(); it != _outTable.end(); it++) {					
					it->IR.left = Common::CIRWindowing::Process(it->IR.left, Common::CIRWindowing::fadein, fadeInBegin, riseTime, globalParameters.GetSampleRate());
					it->IR.right = Common::CIRWindowing::Process(it->IR.right, Common::CIRWindowing::fadein, fadeInBegin, riseTime, globalParameters.GetSampleRate());
				}
			}
			if (IsFadeOutWindowingConfigured()) {
				for (auto it = _outTable.begin(); it != _outTable.end(); it++) {
					it->IR.left = Common::CIRWindowing::Process(it->IR.left, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, globalParameters.GetSampleRate());
					it->IR.right = Common::CIRWindowing::Process(it->IR.right, Common::CIRWindowing::fadeout, fadeOutCutoff, fallTime, globalParameters.GetSampleRate());
				}

				// Update impulseResponseLength and the number of subfilters
				impulseResponseLength = _outTable.begin()->IR.left.size();
				partitionedFRNumberOfSubfilters = CalculateNumberOPartitions(impulseResponseLength);
			}
		}

		//////////////////////////////////
		// FIND METHODS
		//////////////////////////////////

		/**
		 * @brief Find distance bucket for a given reference location and distance
		 * @param _referenceLocation reference location
		 * @param _distance_m distance in meters
		 * @return 
		 */
		const TDistanceBucket * FindDistanceBucket(const float & _distance_m) const {
			const TDistanceBucket * distanceBucket = nullptr;

			// Pick distance bucket (exact or nearest)
			const int32_t qDistance_mm = quantise_dist_mm(_distance_m);
			distanceBucket = FindNearestDistanceBucket(qDistance_mm);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return distanceBucket;
			}
			return distanceBucket;
		}

		const TDistanceBucket * FindNearestDistanceBucket(int32_t queryDistanceMm) const {

			// refBucket.distances MUST be sorted by distance_mm (done in finalizeBuild()).
			auto it = std::lower_bound(distanceFRTable.begin(), distanceFRTable.end(), queryDistanceMm,
				[](const TDistanceBucket & b, int32_t key) { return b.distance_mm < key; });

			// Nearest: choose closest between it and previous
			if (it == distanceFRTable.begin())
				return &(*it);

			if (it == distanceFRTable.end())
				return &distanceFRTable.back();

			const auto & hi = *it;
			const auto & lo = *(it - 1);

			const int32_t dLo = queryDistanceMm - lo.distance_mm;
			const int32_t dHi = hi.distance_mm - queryDistanceMm;

			return (dLo <= dHi) ? &lo : &hi; // tie: pick any (lo)
		}
		// Reset HRTF		
		void Reset() {

			//Change class state
			setupInProgress = false;
			HRTFLoaded = false;

			//Clear every table						
			distanceFRTable.clear();
			sofaIRDataBase2.clear();
			stepVector.clear();

			//Update parameters			
			impulseResponseLength = 0;			
			gridSamplingStep = DEFAULT_GRIDSAMPLING_STEP;
		}	

		///////////////
		// ATTRIBUTES
		///////////////

		mutable std::mutex mutex; // Thread management

		//int32_t impulseResponseLength; // HRIR vector length
		//int32_t bufferSize;								// Input signal buffer size
		int32_t partitionedFRNumberOfSubfilters; // Number of subfilters (blocks) for the UPC algorithm
		int32_t partitionedFRSubfilterLength; // Size of one HRIR subfilter
		//float distanceOfMeasurement; // Distance where the HRIR have been measurement
		Common::CCranialGeometry cranialGeometry; // Cranial geometry of the listener
		Common::CCranialGeometry originalCranialGeometry; // Cranial geometry of the listener
		TEXTRAPOLATION_METHOD extrapolationMethod; // Methods that is going to be used to extrapolate

		float sphereBorder; // Define spheere "sewing"
		float epsilon_sewing;

		float azimuthMin, azimuthMax, elevationMin, elevationMax, elevationNorth, elevationSouth; // Variables that define limits of work area

		bool setupInProgress; // Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded; // Variable that indicates if the HRTF has been loaded correctly
		//bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int gridSamplingStep; // HRTF Resample table step (azimuth and elevation)
		bool enableWoodworthITD; // Indicate the use of a customized delay
		int gapThreshold; // Max distance between pole and next elevation to be consider as a gap

		float fadeInBegin; // Variable to be used in the windowing IR process
		float riseTime; // Variable to be used in the windowing IR process
		float fadeOutCutoff; // Variable to be used in the windowing IR process
		float fallTime; // Variable to be used in the windowing IR process 

		// Tables				
		std::vector<BRTServices::TIRStruct> sofaIRDataBase2;// Time domain database - original data from SOFA file
		TDistanceTable distanceFRTable;						// Data in our grid, interpolated, by distance buckets				
		std::unordered_map<TOrientation, float> stepVector; // Store hrtf interpolated grids steps

		// Empty object to return in some methods
		THRIRStruct emptyHRIR;
		TFRPartitionedStruct emptyHRIR_partitioned;
		CMonoBuffer<float> emptyMonoBuffer;
		Common::CGlobalParameters globalParameters;

		// Processors
		//CQuasiUniformSphereDistribution quasiUniformSphereDistribution;
		COfflineInterpolation offlineInterpolation;
		CExtrapolation extrapolation;

		friend class CHRTFTester;
	};
}
#endif

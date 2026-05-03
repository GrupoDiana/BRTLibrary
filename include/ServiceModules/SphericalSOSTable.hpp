/**
* \class CSphericalSOSTable
*
* \brief Declaration of CSphericalSOSTable class
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

#ifndef _C_SPHERICAL_SOS_TABLE_HPP_
#define _C_SPHERICAL_SOS_TABLE_HPP_

#include <unordered_map>
#include <Common/Buffer.hpp>
#include <Common/CommonDefinitions.hpp>
#include <ServiceModules/ServicesBase.hpp>
#include <ServiceModules/SphericalSearchKDTree.hpp>


/** \brief Hash table that contains a set of coefficients for two biquads filters that are indexed through a pair of distance
	and azimuth values (interaural coordinates). */	
//typedef std::unordered_map<CSOSFilter_Key, BRTServices::TSOSFilterStruct> T_SOSCoefficients_HashTable;
using TSphericalSOSTable = std::unordered_map<TOrientation_key, BRTServices::TSOSFilterStruct>;

// One distance bucket: one sphere (same distance), many directions
struct TSOSDistanceBucket {
	int32_t distance_mm = 0;
	BRTServices::CSphericalSearchKDTree<TOrientation> searchTree;
	TSphericalSOSTable table;
};
using TSOSDistanceTable = std::vector<TSOSDistanceBucket>;



namespace BRTServices {

	/** \details This class models the effect of frequency-dependent Interaural Level Differences when the sound source is close to the listener
	*/
	class CSphericalSOSTable : public CServicesBase {

	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor.
		*	\details Leaves SOS Filter Table empty. Use SetSOSFilterTable to load.
		*   \eh Nothing is reported to the error handler.
		*/
		CSphericalSOSTable()
			: setupInProgress { false }
		{
			serviceType = TServiceType::sos_filter_database;
		}

		/** \brief	Set the relative position of one ear (to the listener head center)
		* 	\param [in]	_ear			ear type
		*   \param [in]	_earPosition	ear local position
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) override {
			if (_ear == Common::T_ear::LEFT) {
				leftEarLocalPosition = _earPosition;
			} else if (_ear == Common::T_ear::RIGHT) {
				rightEarLocalPosition = _earPosition;
			} else if (_ear == Common::T_ear::BOTH || _ear == Common::T_ear::NONE) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to set listener ear transform for BOTH or NONE ears");
			}
		}
		
		bool BeginSetup() override {
			
			std::lock_guard<std::mutex> l(mutex);
			Reset();
			setupInProgress = true;						

			SET_RESULT(RESULT_OK, "SOS Coefficients Setup started");
			return true;
		}

		void AddCoefficients(float _azimuth, float _elevation, float _distance, Common::CEarPair<CMonoBuffer<float>> && newCoefs) override {
			std::lock_guard<std::mutex> l(mutex);
			if (!setupInProgress) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Cannot add SOS Filter coefficients - Setup not in progress");
				return;
			}

			// Find o create orientation bucket
			TSOSDistanceBucket * distanceBucket = nullptr;

			const int32_t distance_mm = quantise_dist_mm(_distance);
			for (auto & distBucketIt : sosDistanceTable) {
				if (distBucketIt.distance_mm == distance_mm) {
					distanceBucket = &distBucketIt;
					break;
				}
			}

			if (!distanceBucket) {
				TSOSDistanceBucket newDist; // Create new distance bucket
				//newDist.distance = _distance;
				newDist.distance_mm = distance_mm;
				sosDistanceTable.push_back(std::move(newDist));
				distanceBucket = &sosDistanceTable.back();
			}

			// Emplace new IR into orientation table
			const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
			const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);
			
			TSOSFilterStruct newData(TOrientation(_azimuthInRage, _elevationInRange, _distance), newCoefs);
			//newData.orientation = TOrientation(_azimuthInRage, _elevationInRange, _distance);
			auto returnValue = distanceBucket->table.emplace(TOrientation(_azimuthInRage, _elevationInRange, _distance), newData);
			//Error handler
			if (!returnValue.second) {
				if (distanceBucket->table.find(TOrientation(_azimuthInRage, _elevationInRange, _distance)) != distanceBucket->table.end()) {
					SET_RESULT(RESULT_WARNING, "Error emplacing SOS coefficients in SOSFilterDataBase map, already exists a coefs in this position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
				} else {
					SET_RESULT(RESULT_WARNING, "Error emplacing SOS coefficients in SOSFilterDataBase map in position [" + std::to_string(_azimuth) + ", " + std::to_string(_elevation) + "]");
				}
			}
		}

		bool EndSetup() override {
			std::lock_guard<std::mutex> l(mutex);
			if (!setupInProgress) { 
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "SOS Filter Setup was not started in BRTServices::CSOSCoefficients::EndSetup()");
				return false;
			}
			
			if (sosDistanceTable.empty()) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "SOS Filter Setup was not started in BRTServices::CSOSCoefficients::EndSetup() - No data was added");
				return false;
			}

			if (sosDistanceTable.size() > 0 && sosDistanceTable.front().table.size() > 1) spatiallyOriented = true;

			// Sort distance buckets by distance_mm
			std::sort(sosDistanceTable.begin(), sosDistanceTable.end(),
				[](const TSOSDistanceBucket & a, const TSOSDistanceBucket & b) {
				return a.distance_mm < b.distance_mm;
			});
			
			// Build search trees for each distance bucket
			for (auto & distBucketIt : sosDistanceTable) {
					std::vector<TOrientation> orientations;
					orientations.reserve(distBucketIt.table.size());
					for (const auto & tableIt : distBucketIt.table) {
						orientations.push_back(tableIt.second.orientation);
					}
					distBucketIt.searchTree.build(std::move(orientations));
			}			
		
			if (numberOfEars != -1) {				
				dataReady = true;
				SET_RESULT(RESULT_OK, "SOS Filter Setup finished");				
				setupInProgress = false;
				return true;
			}
			
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Some parameter is missing in order to finish the data upload in BRTServices::CSphericalSOSTable");
			return false;
		}
		
		/**
		 * @brief Get IIR filter coefficients for SOS Filter, for one ear.
		 * @detail This method is for spatially oriented SOS tables, as it takes into account the azimuth and elevation parameters.
		 * @param _azimuth azimuth angle in degrees
		 * @param _elevation elevation angle in degrees
		 * @param _distance distance in meters
		 * @param ear ear for which we want to get the coefficients
		 * @return std::vector<float> contains the coefficients following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		 */
		const std::vector<float> GetSOSCoefficients_SpatiallyOriented(float _azimuth, float _elevation, float _distance, Common::T_ear ear) const override {
					
			std::vector<float> foundData;
			if (!dataReady) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Spherical SOS table was not initialized in BRTServices::CSphericalSOSTable::GetSOSFilterCoefficients()");
				return foundData;
			}

			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get SOS Filter coefficients for a wrong ear (BOTH or NONE)");
				return foundData;
			}

			if (!spatiallyOriented) { 
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get SOS Filter coefficients with spatial orientation parameters, but SOS table is not spatially oriented");
				return foundData;
			}

			if ((ear == Common::T_ear::RIGHT) && numberOfEars == 1) {
				// This is a workaround to make our near-field SOS SOFA files work; we should correct it those SOFA.
				return GetSOSCoefficients_SpatiallyOriented(-_azimuth, _elevation, _distance, Common::T_ear::LEFT);
			}

			//ASSERT(_distance > 0, RESULT_ERROR_OUTOFRANGE, "Distance must be greater than zero when", "");			

			// Find Table to use if exists
			const TSOSDistanceBucket * distanceBucket = FindDistanceBucket(_distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return foundData;
			}		
			
			Common::CEarPair<CMonoBuffer<float>> data = GetDataFromSphericalSOSTable(distanceBucket, _azimuth, _elevation, true);
			
			if (ear == Common::T_ear::LEFT) {
				return data.left;
            } else { // (ear == Common::T_ear::RIGHT) 
				return data.right;
			}			
		}

		/**
		 * @brief Get IIR filter coefficients for SOS Filter, for both ears. This method is for not spatially oriented SOS tables, as it does not take into account the azimuth and elevation parameters. For spatially oriented SOS tables.		 
		 * @return Pair of CMonoBuffer<float> containing the coefficients for left and right ears, following this order [f1_b0, f1_b1, f1_b2, f1_a1, f1_a2, f2_b0, f2_b1, f2_b2, f2_a1, f2_a2]
		 */
		const Common::CEarPair<CMonoBuffer<float>> GetSOSCoefficients_2Ears() const override {
			
			Common::CEarPair<CMonoBuffer<float>> foundData;

			if (!dataReady) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Spherical SOS table was not initialized in BRTServices::CSphericalSOSTable::GetSOSFilterCoefficients()");
				return foundData;
			}
			
			if (spatiallyOriented) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get SOS Filter coefficients with NOT spatial orientation parameters, but SOS table is spatially oriented");
				return foundData;
			}
			
			float _azimuth = 0;
			float _elevation = 0;
			float _distance = 0;
			bool _findNearest = true;
			// Find Table to use if exists
			const TSOSDistanceBucket * distanceBucket = FindDistanceBucket(_distance);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetFRPartitioned_SpatiallyOriented_2Ears: Distance Bucket find error");
				return foundData;
			}
			
			return GetDataFromSphericalSOSTable(distanceBucket, _azimuth, _elevation, true);
		}

	private:
		
		/**
		 * @brief Find and return the coefficients for a given distance bucket and orientation parameters. If _findNearest is true, it will return the coefficients of the nearest orientation if an exact match is not found. If _findNearest is false, it will return an empty data if an exact match is not found.
		 * @param distanceBucket 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _findNearest 
		 * @return 
		 */
		const Common::CEarPair<CMonoBuffer<float>> GetDataFromSphericalSOSTable(const TSOSDistanceBucket * distanceBucket, const float & _azimuth, const float & _elevation, bool _findNearest) const {

			std::lock_guard<std::mutex> l(mutex);
			const double _azimuthInRage = CInterpolationAuxiliarMethods::NormalizeAzimuth0_360(_azimuth);
			const double _elevationInRange = CInterpolationAuxiliarMethods::NormalizeElevation_0_90_270_360(_elevation);

			Common::CEarPair<CMonoBuffer<float>> foundData;
			auto it = distanceBucket->table.find(TOrientation_key(_azimuthInRage, _elevationInRange));
			if (it != distanceBucket->table.end()) {
				// Exact match found
				foundData = it->second.coefs;
			} else {
				// No exact match
				if (!_findNearest) {
					SET_RESULT(RESULT_ERROR_OUTOFRANGE, "GetDataFromSphericalSOSTable: Requested azimuth and elevation not found in SOS table");
					return foundData;
				}
				// Find nearest
				TOrientation nearest = distanceBucket->searchTree.nearest(_azimuthInRage, _elevationInRange);
				auto it = distanceBucket->table.find(TOrientation_key(nearest));
				if (it != distanceBucket->table.end()) {
					foundData = it->second.coefs;
				} else {
					// ERROR: This should not happen
					SET_RESULT(RESULT_ERROR_NOTALLOWED, "GetDataFromSphericalSOSTable: SearchTree returned an orientation not present in SOS table");
				}
			}
			return foundData;
		}
		
		/**
		 * @brief Find distance bucket for a given reference location and distance
		 * @param _referenceLocation reference location
		 * @param _distance_m distance in meters
		 * @return 
		 */
		const TSOSDistanceBucket * FindDistanceBucket(const float & _distance_m) const {
			const TSOSDistanceBucket * distanceBucket = nullptr;

			// Pick distance bucket (exact or nearest)
			const int32_t qDistance_mm = quantise_dist_mm(_distance_m);
			distanceBucket = FindNearestDistanceBucket(qDistance_mm);
			if (!distanceBucket) {
				SET_RESULT(RESULT_ERROR_UNKNOWN, "GetSOSFilterCoefficients: Distance not found error");
				return distanceBucket;
			}
			return distanceBucket;
		}

		/**
		 * @brief Find the nearest distance bucket for a given quantised distance in millimeters. It assumes that sosDistanceTable is sorted by distance_mm.
		 * @param queryDistanceMm 
		 * @return 
		 */
		const TSOSDistanceBucket * FindNearestDistanceBucket(int32_t queryDistanceMm) const {

			// refBucket.distances MUST be sorted by distance_mm (done in finalizeBuild()).
			auto it = std::lower_bound(sosDistanceTable.begin(), sosDistanceTable.end(), queryDistanceMm,
				[](const TSOSDistanceBucket & b, int32_t key) { return b.distance_mm < key; });

			// Nearest: choose closest between it and previous
			if (it == sosDistanceTable.begin())
				return &(*it);

			if (it == sosDistanceTable.end())
				return &sosDistanceTable.back();

			const auto & hi = *it;
			const auto & lo = *(it - 1);

			const int32_t dLo = queryDistanceMm - lo.distance_mm;
			const int32_t dHi = hi.distance_mm - queryDistanceMm;

			return (dLo <= dHi) ? &lo : &hi; // tie: pick any (lo)
		}

		void Reset() {			
			setupInProgress = false;
			dataReady = false;
			sosDistanceTable.clear();			
			numberOfEars = -1;	
		}

		//int CalculateTableAzimuthStep() {			
		//	// Order azimuth and remove duplicates
		//	std::sort(azimuthList.begin(), azimuthList.end());
		//	azimuthList.erase(unique(azimuthList.begin(), azimuthList.end()), azimuthList.end());

		//	// Calculate the minimum azimuth
		//	int azimuthStep = 999999;	//TODO Why this number?
		//	for (int i = 0; i < azimuthList.size() - 1; i++) {
		//		if (azimuthList[i + 1] - azimuthList[i] < azimuthStep) {
		//			azimuthStep = azimuthList[i + 1] - azimuthList[i];
		//		}
		//	}
		//	return azimuthStep;
		//}

		/// Calculate the TABLE Distance STEP from the table
		//float CalculateTableDistanceStep() {			
		//	// Order distances and remove duplicates
		//	std::sort(distanceList.begin(), distanceList.end());
		//	distanceList.erase(unique(distanceList.begin(), distanceList.end()), distanceList.end());

		//	// Calculate the minimum d
		//	double distanceStep = 999999; //TODO Why this number?
		//	for (int i = 0; i < distanceList.size() - 1; i++) {
		//		if (distanceList[i + 1] - distanceList[i] < distanceStep) {
		//			distanceStep = distanceList[i + 1] - distanceList[i];
		//		}
		//	}
		//				
		//	return distanceStep;
		//}

		/**
		 * @brief Transform distance in meters to distance in milimetres
		 * @param _distanceInMetres distance in meters
		 * @return 
		 */
		float GetDistanceInMM(float _distanceInMetres) {
			return _distanceInMetres * 1000.0f;
		}
		
		/**
		 * @brief Transform distance in milimetres to distance in meters
		 * @param _distanceInMilimetres distance in milimetres
		 * @return 
		 */
		float GetDistanceInMetres(float _distanceInMilimetres) {
			return _distanceInMilimetres * 0.001f;
		}


		///////////////
		// ATTRIBUTES
		///////////////	
		mutable std::mutex mutex;			// Thread management
		bool setupInProgress;				// Variable that indicates the SOS Filter load is in process		
		
		TSOSDistanceTable sosDistanceTable; // SOS Filter table indexed by distance buckets, each containing a search tree and a hash table indexed by orientation
		
		Common::CVector3 leftEarLocalPosition;		// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;		// Listener right ear relative position
	};
}
#endif

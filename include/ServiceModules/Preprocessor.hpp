/**
* \class CPreprocessor
*
* \brief Declaration of CPreprocessor
* \date	October 2023
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
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CPREPROCESSOR_HPP
#define _CPREPROCESSOR_HPP

#include <unordered_map>
#include <vector>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/OfflineInterpolation.hpp>


namespace BRTServices
{
	/**
	 * @brief Base class for extrapolation processors
	*/
	class CPreprocessor {
	public:

		CPreprocessor()
			:resamplingStep{DEFAULT_RESAMPLING_STEP}, gapThreshold{DEFAULT_GAP_THRESHOLD}, HRTFLoaded{ false }, setupInProgress{ false },
			epsilon_sewing{ EPSILON_SEWING }
		{}

		//	Fill out the HRTF for every azimuth and two specific elevations: 90 and 270 degrees	
		template<typename T, typename U, typename Functor>
		void CalculateTF_InPoles(T& _t_TF_DataBase, int _TFlength, int _resamplingStep, Functor f)
		{
			U precalculatedTF_270, precalculatedTF_90;
			bool found = false;
			//Clasify every HRIR of the HRTF into the two hemispheres by their t_HRTF_DataBase_ListOfOrientations
			std::vector<orientation> keys_southernHemisphere, keys_northenHemisphere;
			int iAzimuthPoles = DEFAULT_MIN_AZIMUTH;
			int iElevationNorthPole = CHRTFAuxiliarMethods::GetPoleElevation(TPole::north);
			int iElevationSouthPole = CHRTFAuxiliarMethods::GetPoleElevation(TPole::south);


			//	NORTHERN HEMOSPHERE POLES (90 degrees elevation ) ____________________________________________________________________________
			auto it90 = _t_TF_DataBase.find(orientation(iAzimuthPoles, iElevationNorthPole));

			if (it90 != _t_TF_DataBase.end())
			{
				precalculatedTF_90 = it90->second;
			}
			else
			{
				keys_northenHemisphere.reserve(_t_TF_DataBase.size());

				for (auto& it : _t_TF_DataBase)
				{
					if (it.first.elevation < iElevationNorthPole) { keys_northenHemisphere.push_back(it.first); }
				}
				// sort using a custom function object
				struct { bool operator()(orientation a, orientation b) const { return a.elevation > b.elevation; } } customLess;
				std::sort(keys_northenHemisphere.begin(), keys_northenHemisphere.end(), customLess);

				precalculatedTF_90 = CalculateTF_InOneHemispherePole<T, U, Functor>(_t_TF_DataBase, _TFlength, keys_northenHemisphere, f);

				SET_RESULT(RESULT_WARNING, "Transfer Function interpolated in the pole [ " + std::to_string(iAzimuthPoles) + ", " + std::to_string(iElevationNorthPole) + "]");
			}

			//	SOURTHERN HEMOSPHERE POLES (270 degrees elevation) ____________________________________________________________________________
			auto it270 = _t_TF_DataBase.find(orientation(iAzimuthPoles, iElevationSouthPole));

			if (it270 != _t_TF_DataBase.end())
			{
				precalculatedTF_270 = it270->second;
			}
			else
			{
				keys_southernHemisphere.reserve(_t_TF_DataBase.size());
				for (auto& it : _t_TF_DataBase)
				{
					if (it.first.elevation > iElevationSouthPole) { keys_southernHemisphere.push_back(it.first); }
				}
				//Get a vector of iterators ordered from highest to lowest elevation.		
				struct {
					bool operator()(orientation a, orientation b) const { return a.elevation < b.elevation; }
				} customLess;
				std::sort(keys_southernHemisphere.begin(), keys_southernHemisphere.end(), customLess);

				precalculatedTF_270 = CalculateTF_InOneHemispherePole<T,U, Functor>(_t_TF_DataBase, _TFlength, keys_southernHemisphere, f);

				SET_RESULT(RESULT_WARNING, "Transfer Function interpolated in the pole [ " + std::to_string(iAzimuthPoles) + ", " + std::to_string(iElevationSouthPole) + "]");
			}
			// Fill out the table for "every azimuth in the pole" ____________________________________________________________________________
			for (int az = DEFAULT_MIN_AZIMUTH; az < DEFAULT_MAX_AZIMUTH; az = az + _resamplingStep)
			{
				//Elevation 90 degrees
				_t_TF_DataBase.emplace(orientation(az, iElevationNorthPole), precalculatedTF_90);
				//Elevation 270 degrees
				_t_TF_DataBase.emplace(orientation(az, iElevationSouthPole), precalculatedTF_270);
			}
		}

		//	Calculate the HRIR in the pole of one of the hemispheres
		//param hemisphereParts	vector of the HRTF t_HRTF_DataBase_ListOfOrientations of the hemisphere		
		template<typename T, typename U, typename Functor>
		U CalculateTF_InOneHemispherePole(T& _t_TF_DataBase, int _TFlength, std::vector<orientation> keys_hemisphere, Functor f)
		{
			U calculatedHRIR;
			std::vector < std::vector <orientation>> hemisphereParts;
			hemisphereParts.resize(NUMBER_OF_PARTS);
			int border = std::ceil(SPHERE_BORDER / NUMBER_OF_PARTS);
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

			//calculatedHRIR = CHRTFAuxiliarMethods::CalculateHRIRFromHemisphereParts(_t_TF_DataBase, _TFlength, hemisphereParts);
			calculatedHRIR = f(_t_TF_DataBase, _TFlength, hemisphereParts);

			// //Calculate the delay and the HRIR of each hemisphere part
			//float totalDelay_left = 0.0f;
			//float totalDelay_right = 0.0f;

			//std::vector< THRIRStruct> newHRIR;
			//newHRIR.resize(hemisphereParts.size());

			//for (int q = 0; q < hemisphereParts.size(); q++)
			//{
			//	newHRIR[q].leftHRIR.resize(_HRIRLength, 0.0f);
			//	newHRIR[q].rightHRIR.resize(_HRIRLength, 0.0f);

			//	float scaleFactor;
			//	if (hemisphereParts[q].size())
			//	{
			//		scaleFactor = 1.0f / hemisphereParts[q].size();
			//	}
			//	else
			//	{
			//		scaleFactor = 0.0f;
			//	}

			//	for (auto it = hemisphereParts[q].begin(); it != hemisphereParts[q].end(); it++)
			//	{
			//		auto itHRIR = _t_HRTF_DataBase.find(orientation(it->azimuth, it->elevation));

			//		//Get the delay
			//		newHRIR[q].leftDelay = (newHRIR[q].leftDelay + itHRIR->second.leftDelay);
			//		newHRIR[q].rightDelay = (newHRIR[q].rightDelay + itHRIR->second.rightDelay);

			//		//Get the HRIR
			//		for (int i = 0; i < _HRIRLength; i++) {
			//			newHRIR[q].leftHRIR[i] = (newHRIR[q].leftHRIR[i] + itHRIR->second.leftHRIR[i]);
			//			newHRIR[q].rightHRIR[i] = (newHRIR[q].rightHRIR[i] + itHRIR->second.rightHRIR[i]);
			//		}
			//	}//END loop hemisphere part

			//	 //Multiply by the factor (weighted sum)
			//	 //Delay 
			//	totalDelay_left = totalDelay_left + (scaleFactor * newHRIR[q].leftDelay);
			//	totalDelay_right = totalDelay_right + (scaleFactor * newHRIR[q].rightDelay);
			//	//HRIR
			//	for (int i = 0; i < _HRIRLength; i++)
			//	{
			//		newHRIR[q].leftHRIR[i] = newHRIR[q].leftHRIR[i] * scaleFactor;
			//		newHRIR[q].rightHRIR[i] = newHRIR[q].rightHRIR[i] * scaleFactor;
			//	}
			//}

			////Get the FINAL values
			//float scaleFactor_final = 1.0f / hemisphereParts.size();

			////Calculate Final delay
			//calculatedHRIR.leftDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_left));
			//calculatedHRIR.rightDelay = static_cast <unsigned long> (round(scaleFactor_final * totalDelay_right));

			////calculate Final HRIR
			//calculatedHRIR.leftHRIR.resize(_HRIRLength, 0.0f);
			//calculatedHRIR.rightHRIR.resize(_HRIRLength, 0.0f);

			//for (int i = 0; i < _HRIRLength; i++)
			//{
			//	for (int q = 0; q < hemisphereParts.size(); q++)
			//	{
			//		calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] + newHRIR[q].leftHRIR[i];
			//		calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] + newHRIR[q].rightHRIR[i];
			//	}
			//}
			//for (int i = 0; i < _HRIRLength; i++)
			//{
			//	calculatedHRIR.leftHRIR[i] = calculatedHRIR.leftHRIR[i] * scaleFactor_final;
			//	calculatedHRIR.rightHRIR[i] = calculatedHRIR.rightHRIR[i] * scaleFactor_final;
			//}

			return calculatedHRIR;
		}

		/// <summary>
		/// Get HRIR of azimith 0 and emplace again with azimuth 360 in the HRTF database table for every elevations
		/// </summary>
		/// <param name="_resamplingStep"></param>
		template <typename T>
		void FillOutTableInAzimuth360(T& _t_TF_DataBase, int _resamplingStep) {

			int iElevationNorthPole = CHRTFAuxiliarMethods::GetPoleElevation(TPole::north);
			int iElevationSouthPole = CHRTFAuxiliarMethods::GetPoleElevation(TPole::south);

			for (int el = DEFAULT_MIN_ELEVATION; el <= iElevationNorthPole; el = el + _resamplingStep)
			{
				GetAndEmplaceTF_inAzimuth360(_t_TF_DataBase, el);
			}
			for (int el = iElevationSouthPole; el < DEFAULT_MAX_ELEVATION; el = el + _resamplingStep)
			{
				GetAndEmplaceTF_inAzimuth360(_t_TF_DataBase, el);
			}
		}

		/// <summary>
		/// Get HRIR of azimith 0 and emplace again with azimuth 360 in the HRTF database table for an specific elevation
		/// </summary>
		/// <param name="_elevation"></param>
		template <typename T>
		void GetAndEmplaceTF_inAzimuth360(T& _t_TF_DataBase, float _elevation) {
			auto it = _t_TF_DataBase.find(orientation(DEFAULT_MIN_AZIMUTH, _elevation));
			if (it != _t_TF_DataBase.end()) {
				_t_TF_DataBase.emplace(orientation(DEFAULT_MAX_AZIMUTH, _elevation), it->second);
			}
		}

		/// <summary>
		/// Fill Spherical Cap Gap of an HRTF making Interpolation between pole and the 2 nearest points.
		/// </summary>
		/// <param name="_gapThreshold"></param>
		/// <param name="_resamplingStep"></param>
		template <typename T, typename U, typename Functor>
		void CalculateTF_SphericalCaps(T& _t_TF_DataBase, int _TFlength, int _gapThreshold, int _resamplingStep, Functor f_CalculateHRIR_Offline)
		{
			// Initialize some variables
			float max_dist_elev = 0;
			int iElevationNorthPole = CHRTFAuxiliarMethods::GetPoleElevation(TPole::north);
			int iElevationSouthPole = CHRTFAuxiliarMethods::GetPoleElevation(TPole::south);
			int elev_Step = _resamplingStep;
			int pole;
			float elev_south, elev_north, distance;
			std::vector<orientation> orientations, north_hemisphere, south_hemisphere;
			orientation bufferOrientation;

			// Create a vector with all the t_HRTF_DataBase_ListOfOrientations of the Database
			orientations.reserve(_t_TF_DataBase.size());
			for (auto& itr : _t_TF_DataBase)
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
				Calculate_and_EmplaceTF<T, U, Functor>(_t_TF_DataBase, _TFlength, iElevationSouthPole, south_hemisphere, elev_south, elev_Step, f_CalculateHRIR_Offline);
			}
			// Reset var to use it in north hemisphere
			max_dist_elev = 0;

			// NORTH HEMISPHERE
			CalculateDistanceBetweenPoleandLastRing(north_hemisphere, max_dist_elev, elev_north);

			if (max_dist_elev > _gapThreshold)
			{
				Calculate_and_EmplaceTF<T, U, Functor>(_t_TF_DataBase,_TFlength, iElevationNorthPole, north_hemisphere, elev_north, elev_Step, f_CalculateHRIR_Offline);
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
		template <typename T, typename U, typename Functor>
		void Calculate_and_EmplaceTF(T& _t_Table,int _TFLength, int _pole, std::vector<orientation> _hemisphere, float _elevationLastRing, int _fillStep, Functor f_CalculateHRIR_Offline)
		{
			std::vector<orientation> lastRingOrientationList;
			std::vector<T_PairDistanceOrientation> sortedList;
			int azimuth_Step = _fillStep;
			U TF_interpolated;

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
					for (float azim = DEFAULT_MIN_AZIMUTH; azim < DEFAULT_MAX_AZIMUTH; azim = azim + azimuth_Step)
					{
						//sortedList = distanceBasedInterpolator.GetSortedDistancesList(lastRingOrientationList, azim, elevat);
						TF_interpolated = distanceBasedInterpolator.CalculateHRIR_offlineMethod<T,U, Functor>(_t_Table, f_CalculateHRIR_Offline, lastRingOrientationList, azim, elevat, _TFLength, _pole);
						_t_Table.emplace(orientation(azim, elevat), TF_interpolated);
					}
				}
			}	// NORTH HEMISPHERE
			else if (_pole == ELEVATION_NORTH_POLE)
			{
				for (float elevat = _elevationLastRing + _fillStep; elevat < _pole; elevat = elevat + _fillStep)
				{
					for (float azim = DEFAULT_MIN_AZIMUTH; azim < DEFAULT_MAX_AZIMUTH; azim = azim + azimuth_Step)
					{
						//sortedList = distanceBasedInterpolator.GetSortedDistancesList(lastRingOrientationList, azim, elevat);
						TF_interpolated = distanceBasedInterpolator.CalculateHRIR_offlineMethod<T, U, Functor>(_t_Table, f_CalculateHRIR_Offline, lastRingOrientationList, azim, elevat, _TFLength, _pole);
						_t_Table.emplace(orientation(azim, elevat), TF_interpolated);
					}
				}
			}
		}


		/**
		 * @brief Fill vector with the list of orientations of the T_HRTF_DataBase table
		 * @tparam T Type of the table 
		 * @param table Table of data 
		 * @return List of orientations
		*/
		template <typename T>
		std::vector<orientation> CalculateListOfOrientations(T& table) {
			std::vector<orientation> table_ListOfOrientations;
			table_ListOfOrientations.reserve(table.size());
			for (auto& kv : table)
			{
				table_ListOfOrientations.push_back(kv.first);
			}
			return table_ListOfOrientations;
		}

		
		template <typename T, typename U, typename Functor>
		void FillResampledTable(T& table_dataBase, U& t_HRTF_Resampled_partitioned, int _bufferSize, int _HRIRLength, int _HRIRPartitioned_NumberOfSubfilters, Functor f) {
			int numOfInterpolatedHRIRs = 0;
			for (auto& it : t_HRTF_Resampled_partitioned)
			{
				if (CalculateAndEmplaceNewPartitionedHRIR(table_dataBase, t_HRTF_Resampled_partitioned, it.first.azimuth, it.first.elevation, _bufferSize, _HRIRLength, _HRIRPartitioned_NumberOfSubfilters, f)) { numOfInterpolatedHRIRs++; }
			}
			SET_RESULT(RESULT_WARNING, "Number of interpolated HRIRs: " + std::to_string(numOfInterpolatedHRIRs));
		}

		template <typename T, typename U, typename Functor>
		bool CalculateAndEmplaceNewPartitionedHRIR(T& t_HRTF_DataBase, U& t_HRTF_Resampled_partitioned, double _azimuth, double _elevation, int _bufferSize, int _HRIRLength, int _HRIRPartitioned_NumberOfSubfilters, Functor f) {
			//THRIRStruct interpolatedHRIR;
			bool bHRIRInterpolated = false;

			auto it = t_HRTF_DataBase.find(orientation(_azimuth, _elevation));
			if (it != t_HRTF_DataBase.end())
			{
				//Fill out HRTF partitioned table.IR in frequency domain
				//THRIRPartitionedStruct newHRIR_partitioned;
				auto newHRIR_partitioned = f(it->second, _bufferSize, _HRIRPartitioned_NumberOfSubfilters);
				t_HRTF_Resampled_partitioned[orientation(_azimuth, _elevation)] = std::forward<THRIRPartitionedStruct>(newHRIR_partitioned);
			}
			else
			{
				// Get a list sorted by distances to the orientation of interest
				//std::vector<T_PairDistanceOrientation> sortedList = distanceBasedInterpolator.GetSortedDistancesList(t_HRTF_DataBase_ListOfOrientations, _azimuth, _elevation);
				//Get the interpolated HRIR 
				//auto interpolatedHRIR = distanceBasedInterpolator.CalculateHRIR_offlineMethod(t_HRTF_DataBase, t_HRTF_DataBase_ListOfOrientations, _azimuth, _elevation, HRIRLength);
				auto t_HRTF_DataBase_ListOfOrientations = CalculateListOfOrientations(t_HRTF_DataBase);
				auto interpolatedHRIR = quadrantBasedInterpolator.CalculateHRIR_offlineMethod(t_HRTF_DataBase, t_HRTF_DataBase_ListOfOrientations, _HRIRLength, _azimuth, _elevation);
				bHRIRInterpolated = true;

				//Fill out HRTF partitioned table.IR in frequency domain
				//THRIRPartitionedStruct newHRIR_partitioned;
				auto newHRIR_partitioned = f(interpolatedHRIR, _bufferSize, _HRIRPartitioned_NumberOfSubfilters);
				t_HRTF_Resampled_partitioned[orientation(_azimuth, _elevation)] = std::forward<THRIRPartitionedStruct>(newHRIR_partitioned);
			}
			return bHRIRInterpolated;

		}



	private:

		float epsilon_sewing;
		int gapThreshold;							// Max distance between pole and next elevation to be consider as a gap

		bool setupInProgress;						// Variable that indicates the HRTF add and resample algorithm are in process
		bool HRTFLoaded;							// Variable that indicates if the HRTF has been loaded correctly
		bool bInterpolatedResampleTable;			// If true: calculate the HRTF resample matrix with interpolation
		int resamplingStep; 						// HRTF Resample table step (azimuth and elevation)

		CDistanceBasedInterpolator distanceBasedInterpolator;
		CQuadrantBasedInterpolator quadrantBasedInterpolator;
		
	};
	
}
#endif
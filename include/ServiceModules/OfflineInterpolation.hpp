/**
* \class COfflineInterpolation
*
* \brief Declaration of COfflineInterpolation
* \date	October 2023
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


#ifndef _COFFLINEINTERPOLATION_HPP
#define _COFFLINEINTERPOLATION_HPP

#include <unordered_map>
#include <vector>
#include <ServiceModules/OfflineInterpolationAuxiliarMethods.hpp>


namespace BRTServices
{
	/**
	 * @brief Base class for extrapolation processors
	*/
	class COfflineInterpolation {
	public:

		COfflineInterpolation(){}		
		/**
		 * @brief Fill out the TFs or IRs tables for every azimuth and two specific elevations: 90 and 270 degrees	
		 * @tparam T type of table
		 * @tparam U type of data contained in the tables
		 * @tparam Functor function used to get the new TF
		 * @param _t_TF_DataBase table to get the data used in the calculation and to emplace the calculated data
		 * @param _TFlength lenght of the calculated data
		 * @param _resamplingStep configured resampling step
		 * @param f function used to get the new TF
		*/
		template<typename T, typename U, typename Functor>
		void CalculateTF_InPoles(T& _t_TF_DataBase, int _TFlength, int _resamplingStep, Functor f)
		{
			U precalculatedTF_270, precalculatedTF_90;
			bool found = false;
			//Clasify every HRIR of the HRTF into the two hemispheres by their t_HRTF_DataBase_ListOfOrientations
			std::vector<orientation> keys_southernHemisphere, keys_northenHemisphere;
			int iAzimuthPoles = DEFAULT_MIN_AZIMUTH;
			int iElevationNorthPole = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			int iElevationSouthPole = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);

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
			for (int az = DEFAULT_MIN_AZIMUTH; az <= DEFAULT_MAX_AZIMUTH; az = az + _resamplingStep)
			{
				//Elevation 90 degrees
				_t_TF_DataBase.emplace(orientation(az, iElevationNorthPole), precalculatedTF_90);
				//Elevation 270 degrees
				_t_TF_DataBase.emplace(orientation(az, iElevationSouthPole), precalculatedTF_270);
			}
		}

		/**
		 * @brief Calculate the IR of TF in the pole of one of the hemispheres
		 * @tparam T type of table
		 * @tparam U type of data contained in the tables
		 * @tparam Functor function used to get the new TF
		 * @param _t_TF_DataBase table to get the data used in the calculation and to emplace the calculated data
		 * @param _TFlength lenght of the calculated data
		 * @param keys_hemisphere vector with the orientations of one of the hemispheres
		 * @param f function used to get the new TF
		 * @return the calculated IR or TF for one of the hemispheres
		*/
		template<typename T, typename U, typename Functor>
		U CalculateTF_InOneHemispherePole(T& _t_TF_DataBase, int _TFlength, std::vector<orientation> keys_hemisphere, Functor f)
		{
			U calculatedTF;
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
					} 
				}
			}

			//Calculate the interpolated IR or TF
			calculatedTF = f(_t_TF_DataBase, _TFlength, hemisphereParts);
			return calculatedTF;
		}


		/**
		 * @brief Get IR or TF of azimith 0 and emplace again with azimuth 360 in the database table for an specific elevation
		 * @tparam T type of used table
		 * @param _t_TF_DataBase table to get the data in azimuth 0 and emplace in azimuth 360
		 * @param _elevation 
		*/
		template <typename T>
		void GetAndEmplaceTF_inAzimuth360(T& _t_TF_DataBase, float _elevation) {
			auto it = _t_TF_DataBase.find(orientation(DEFAULT_MIN_AZIMUTH, _elevation));
			if (it != _t_TF_DataBase.end()) {
				_t_TF_DataBase.emplace(orientation(DEFAULT_MAX_AZIMUTH, _elevation), it->second);
			}
		}

		/**
		 * @brief Fill Spherical Cap Gap of an IR os TFs table, making interpolation between pole and the 2 nearest points.
		 * @tparam T type of table
		 * @tparam U type of data contained in the tables
		 * @tparam Functor function used to get the new IR or TF
		 * @param _t_TF_DataBase table to get the data used in the calculation and to emplace the calculated data
		 * @param _TFlength lenght of the calculated data
		 * @param _gapThreshold maximum size of the gap
		 * @param _resamplingStep resampling step used to fill the gap
		 * @param f_CalculateHRIR_Offline function used to get the new IR or TF
		*/
		template <typename T, typename U, typename Functor>
		void CalculateTF_SphericalCaps(T& _t_TF_DataBase, int _TFlength, int _gapThreshold, int _resamplingStep, Functor f_CalculateHRIR_Offline)
		{
			// Initialize some variables
			float max_dist_elev = 0;
			int iElevationNorthPole = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::north);
			int iElevationSouthPole = CInterpolationAuxiliarMethods::GetPoleElevation(TPole::south);
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
				Calculate_and_EmplaceTF_InCaps<T, U, Functor>(_t_TF_DataBase, _TFlength, iElevationSouthPole, south_hemisphere, elev_south, elev_Step, f_CalculateHRIR_Offline);
			}
			// Reset var to use it in north hemisphere
			max_dist_elev = 0;

			// NORTH HEMISPHERE
			CalculateDistanceBetweenPoleandLastRing(north_hemisphere, max_dist_elev, elev_north);

			if (max_dist_elev > _gapThreshold)
			{
				Calculate_and_EmplaceTF_InCaps<T, U, Functor>(_t_TF_DataBase,_TFlength, iElevationNorthPole, north_hemisphere, elev_north, elev_Step, f_CalculateHRIR_Offline);
			}
		}
		
		/**
		 * @brief Calculate the max distance between pole and the nearest ring, to know if there is a gap in any spherical cap
		 * @param [in] _hemisphere vector that contains all the oritations of the hemisphere
		 * @param [out] max_dist_elev maximum elevation distance between rings
		 * @param [out] elevationLastRing elevation in degrees of the last ring
		*/
		void CalculateDistanceBetweenPoleandLastRing(std::vector<orientation>& _hemisphere, float& max_dist_elev, float& elevationLastRing)
		{
			for (int jj = 1; jj < _hemisphere.size(); jj++)
			{
				// distance between 2 t_HRTF_DataBase_ListOfOrientations, always positive
				if (_hemisphere[jj].elevation != _hemisphere[0].elevation)
				{
					max_dist_elev = abs(_hemisphere[jj].elevation - _hemisphere[0].elevation); // Distance positive for all poles
					elevationLastRing = _hemisphere[jj].elevation;
					break;
				}
			}
		}
		
		/**
		 * @brief Off-line calculation of the IR or TF in the spherical caps, by distance-based and emplace it in the original table
		 * @tparam T type of table
		 * @tparam U type of data contained in the tables
		 * @tparam Functor function used to get the new IR or TF
		 * @param _t_TF_DataBase table to get the data used in the calculation and to emplace the calculated data
		 * @param _TFlength lenght of the calculated data
		 * @param _pole elevation (ELEVATION_SOUTH_POLE or ELEVATION_NORTH_POLE) of the pole of the corresponding cap
		 * @param _hemisphere hemishere of the cap (north or south)
		 * @param _elevationLastRing elevation in degrees of the last ring of the original table
		 * @param _fillStep azimuth step used to fill the spherial cap
		 * @param f_CalculateHRIR_Offline function used to get the new IR or TF
		*/
		template <typename T, typename U, typename Functor>
		void Calculate_and_EmplaceTF_InCaps(T& _t_Table,int _TFLength, int _pole, std::vector<orientation> _hemisphere, float _elevationLastRing, int _fillStep, Functor f_CalculateHRIR_Offline)
		{
			std::vector<orientation> lastRingOrientationList;
			std::vector<T_PairDistanceOrientation> sortedList;
			int azimuth_Step = _fillStep;
			U TF_interpolated;

			// Get a list with only the points of the nearest known ring
			for (auto& itr : _hemisphere)
			{
				if (Common::AreSame(itr.elevation, _elevationLastRing, 0.0001f) )
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
						TF_interpolated = distanceBasedInterpolator.CalculateHRIR_offlineMethod<T,U, Functor>(_t_Table, f_CalculateHRIR_Offline, lastRingOrientationList, azim, elevat, _TFLength, _pole);
						_t_Table.emplace(orientation(azim, elevat), TF_interpolated);
					}
				}
			}	
			// NORTH HEMISPHERE
			else if (_pole == ELEVATION_NORTH_POLE)
			{
				for (float elevat = _elevationLastRing + _fillStep; elevat < _pole; elevat = elevat + _fillStep)
				{
					for (float azim = DEFAULT_MIN_AZIMUTH; azim < DEFAULT_MAX_AZIMUTH; azim = azim + azimuth_Step)
					{
						TF_interpolated = distanceBasedInterpolator.CalculateHRIR_offlineMethod<T, U, Functor>(_t_Table, f_CalculateHRIR_Offline, lastRingOrientationList, azim, elevat, _TFLength, _pole);
						_t_Table.emplace(orientation(azim, elevat), TF_interpolated);
					}
				}
			}
		}


		/**
		 * @brief Fill vector with the list of orientations of the original database table
		 * @tparam T type of table
		 * @param table type of data contained in the table
		 * @return vector with a list of orientations of the original table
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

		/**
		 * @brief 
		 * @tparam T 
		 * @tparam U 
		 * @tparam W_TFStruct 
		 * @tparam X_TFPartitionedStruct 
		 * @tparam Functor 
		 * @tparam Functor2 
		 * @param table_dataBase 
		 * @param t_HRTF_Resampled_partitioned 
		 * @param _bufferSize 
		 * @param _HRIRLength 
		 * @param _HRIRPartitioned_NumberOfSubfilters 
		 * @param f 
		 * @param f2 
		*/
		template <typename T, typename U, typename W_TFStruct, typename X_TFPartitionedStruct, typename Functor, typename Functor2>
		void FillResampledTable(T& table_dataBase, U& t_HRTF_Resampled_partitioned, int _bufferSize, int _HRIRLength, int _HRIRPartitioned_NumberOfSubfilters, Functor f, Functor2 f2) {
			int numOfInterpolatedHRIRs = 0;
			for (auto& it : t_HRTF_Resampled_partitioned)
			{
				if (CalculateAndEmplace_NewPartitionedTF<T,U, W_TFStruct, X_TFPartitionedStruct, Functor, Functor2>(table_dataBase, t_HRTF_Resampled_partitioned, it.first.azimuth, it.first.elevation, _bufferSize, _HRIRLength, _HRIRPartitioned_NumberOfSubfilters, f, f2)) { numOfInterpolatedHRIRs++; }
			}
			SET_RESULT(RESULT_WARNING, "Number of interpolated HRIRs: " + std::to_string(numOfInterpolatedHRIRs));
		}

		/**
		 * @brief 
		 * @tparam T 
		 * @tparam U 
		 * @tparam W_TFStruct 
		 * @tparam X_TFPartitionedStruct 
		 * @tparam Functor 
		 * @tparam Functor2 
		 * @param table 
		 * @param resampledTable 
		 * @param _azimuth 
		 * @param _elevation 
		 * @param _bufferSize 
		 * @param _TFLength 
		 * @param _TFPartitioned_NumberOfSubfilters 
		 * @param f 
		 * @param f2 
		 * @return 
		*/
		template <typename T, typename U, typename W_TFStruct, typename X_TFPartitionedStruct, typename Functor, typename Functor2>
		bool CalculateAndEmplace_NewPartitionedTF(T& table, U& resampledTable, double _azimuth, double _elevation, int _bufferSize, int _TFLength, int _TFPartitioned_NumberOfSubfilters, Functor f, Functor2 f2) {
			W_TFStruct newTF;
			bool bHRIRInterpolated = false;

			auto it = table.find(orientation(_azimuth, _elevation));
			if (it != table.end())
			{
				newTF = it->second;
			}
			else
			{				
				auto t_HRTF_DataBase_ListOfOrientations = CalculateListOfOrientations(table);
				newTF = quadrantBasedInterpolator.CalculateHRIR_offlineMethod<T, W_TFStruct, Functor2>(table, f2, t_HRTF_DataBase_ListOfOrientations, _TFLength, _azimuth, _elevation);
				bHRIRInterpolated = true;
			}
			//Fill out HRTF partitioned table.IR in frequency domain
			X_TFPartitionedStruct newTF_partitioned = f(newTF, _bufferSize, _TFPartitioned_NumberOfSubfilters);
			resampledTable[orientation(_azimuth, _elevation)] = std::forward<X_TFPartitionedStruct>(newTF_partitioned);
			return bHRIRInterpolated;
		}


	private:

		CDistanceBased_OfflineInterpolator distanceBasedInterpolator;
		CQuadrantBased_OfflineInterpolator quadrantBasedInterpolator;
		
	};
	
}
#endif
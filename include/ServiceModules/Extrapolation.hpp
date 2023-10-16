/**
* \class CExtrapolationInterface, CExtrapolation
*
* \brief Declaration of CExtrapolationInterface and CExtrapolationBasedNearestPoint classes interface
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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CEXTRAPOLATION_HPP
#define _CEXTRAPOLATION_HPP

#include <unordered_map>
#include <vector>
#include <ServiceModules/HRTFDefinitions.hpp>

namespace BRTServices
{
	/**
	 * @brief Base class for extrapolation processors
	*/
	class CExtrapolation {
	public:
		

		/**
		 * @brief Perform, if necessary, the extrapolation process by padding with zeros.
		 * @param table Table of data to be extrapolated
		 * @param extrapolationStep 
		*/
		void ProcessZeroInsertionBasedExtrapolation(T_HRTFTable& table, int extrapolationStep) {
			// Look for gaps and their borders
			TAzimuthElevationBorders borders;
			TGapsFound gapsFound = AreGapsInIRGrid(table, borders);

			std::vector<orientation> orientationsList;		// Just and empty list
			FillGaps(table, orientationsList, extrapolationStep, gapsFound, borders, GetZerosHRIR());
		};
		
		/**
		 * @brief Perform, if necessary, the extrapolation process by filling to the nearest point
		 * @param table 
		 * @param orientationsList 
		 * @param extrapolationStep 
		*/
		void ProcessNearestPointBasedExtrapolation(T_HRTFTable& table, const std::vector<orientation>& orientationsList, int extrapolationStep) {
			// Look for gaps and their borders
			TAzimuthElevationBorders borders;
			TGapsFound gapsFound = AreGapsInIRGrid(table, borders);

			FillGaps(table, orientationsList, extrapolationStep, gapsFound, borders, GetNearestPointHRIR());
		};

	private:
		
		/**
		 * @brief Returns an HRIR filled with zeros in all cases.
		*/
		struct GetZerosHRIR {

			/**
			 * @brief Returns an HRIR filled with zeros in all cases.
			 * @param table data table
			 * @param orientations List Orientations of the data table. This data is not used
			 * @param _azimuth This data is not used
			 * @param _elevation This data is not used
			 * @return HRIR struct filled with zeros
			*/
			THRIRStruct operator() (const T_HRTFTable& table, const std::vector<orientation>& orientationsList, double _azimuth, double _elevation) {
				// Initialization
				int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one
				THRIRStruct HRIRZeros;
				HRIRZeros.leftHRIR.resize(HRIRSize, 0);
				HRIRZeros.rightHRIR.resize(HRIRSize, 0);
				return HRIRZeros;
			}
		};
		
		/**
		 * @brief Given any point returns the HRIR of the closest point to that point.
		*/
		struct GetNearestPointHRIR {
			/**
			 * @brief Given any point returns the HRIR of the closest point to that point.
			 * @param table data table
			 * @param orientationsList List Orientations of the data table
			 * @param _azimuth point of interest azimuth
			 * @param _elevation point of interest elevation
			 * @return HRIR struct filled with the nearest point data
			*/
			THRIRStruct operator() (const T_HRTFTable& table, const std::vector<orientation>& orientationsList, double _azimuth, double _elevation) {
				// Order list of orientation
				std::vector<T_PairDistanceOrientation> pointsOrderedByDistance = CHRTFAuxiliarMethods::GetListOrderedDistancesToPoint(orientationsList, _azimuth, _elevation);
				// Get nearest
				double nearestAzimuth = pointsOrderedByDistance.begin()->second.azimuth;
				double nearestElevation = pointsOrderedByDistance.begin()->second.elevation;
				// Find nearest HRIR and copy
				THRIRStruct nearestHRIR;

				auto it = table.find(orientation(nearestAzimuth, nearestElevation));
				if (it != table.end()) {
					nearestHRIR = it->second;
				}
				else {
					SET_RESULT(RESULT_WARNING, "No point close enough to make the extrapolation has been found, this must not happen.");

					int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one					
					nearestHRIR.leftHRIR.resize(HRIRSize, 0);
					nearestHRIR.rightHRIR.resize(HRIRSize, 0);
				}

				return nearestHRIR;
			}
		};


		/**
		 * @brief Struct to store azimtuh and elevation gap borders
		*/
		struct TAzimuthElevationBorders {
			double maxAzimuth;
			double minAzimuth;
			double maxElevation;
			double minElevation;

			TAzimuthElevationBorders() : maxAzimuth{ 0 }, minAzimuth{ 0 }, maxElevation{ 0 }, minElevation{ 0 } {};
			TAzimuthElevationBorders(double _maxAzimuth, double _minAzimuth, double _maxElevation, double _minElevation) {
				maxAzimuth		= _maxAzimuth;
				minAzimuth		= _minAzimuth;
				maxElevation	= _maxElevation;
				minElevation	= _minElevation;
			}
		};

		/**
		 * @brief Struct to store the found gaps
		*/
		struct TGapsFound {
			bool gapMaxElevation;
			bool gapMinElevation;
			bool gapMaxAzimuth;
			bool gapMinAzimuth;
			
			TGapsFound(){
				gapMaxElevation = false;
				gapMinElevation = false;
				gapMaxAzimuth = false;
				gapMinAzimuth = false;
			}
		};

		
		/**
		 * @brief Look for large gaps in the grid
		 * @param table HRIR table where to look for gaps
		 * @param _borders Borders of gaps found, if found at all
		 * @return Set of booleans indicating whether a gap has been found and which one.
		*/
		TGapsFound AreGapsInIRGrid(const T_HRTFTable& table, TAzimuthElevationBorders& _borders) {
			int totalSourcePositions = table.size();
			double averageStep = 360 / std::sqrt(totalSourcePositions * M_PI);
			
			_borders = Find_AzimuthAndElevationBorders(table);

			TGapsFound gapsFound;			
			if (( 90 - _borders.maxElevation) > (2 * averageStep)) { gapsFound.gapMaxElevation = true; }
			if (( 90 + _borders.minElevation) > (2 * averageStep)) { gapsFound.gapMinElevation = true; }
			if ((180 - _borders.maxAzimuth) > (2 * averageStep)) { gapsFound.gapMaxAzimuth = true; }
			if ((180 + _borders.minAzimuth) > (2 * averageStep)) { gapsFound.gapMinAzimuth = true; }

			// Transforrm back to library ranges
			_borders.maxAzimuth = CHRTFAuxiliarMethods::CalculateAzimuthIn0_360Range(_borders.maxAzimuth);
			_borders.minAzimuth = CHRTFAuxiliarMethods::CalculateAzimuthIn0_360Range(_borders.minAzimuth);
			_borders.maxElevation = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_borders.maxElevation);
			_borders.minElevation = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_borders.minElevation);
			
			return gapsFound;
		}

		/**
		 * @brief Find the maximum and minimum elevation and azimuth values.
		 * @param tableH RIR table where to look for gaps
		 * @return azimuth and elevation values
		*/
		TAzimuthElevationBorders Find_AzimuthAndElevationBorders(const T_HRTFTable& table) {
			// Init values with the opposite
			TAzimuthElevationBorders borders(-18, 180, -90, 90);
			// Process
			for (auto it = table.begin(); it != table.end(); it++) {
				double _azimuthTemp = CHRTFAuxiliarMethods::CalculateAzimuthIn180Range(it->first.azimuth);
				double _elevationTemp = CHRTFAuxiliarMethods::CalculateElevationIn90Range(it->first.elevation);

				borders.maxAzimuth		= (_azimuthTemp		> borders.maxAzimuth) && (_azimuthTemp != 180)	? _azimuthTemp : borders.maxAzimuth;
				borders.minAzimuth		= (_azimuthTemp		< borders.minAzimuth) && (_azimuthTemp != -180)	? _azimuthTemp	: borders.minAzimuth;
				borders.maxElevation	= (_elevationTemp	> borders.maxElevation) && (_elevationTemp != 90) ? _elevationTemp : borders.maxElevation;
				borders.minElevation	= (_elevationTemp	< borders.minElevation)	&& (_elevationTemp !=-90)	? _elevationTemp : borders.minElevation;
			}		
			return borders;
		}

		/**
		 * @brief Perform the extrapolation
		*/
		template <typename Functor>
		void FillGaps(T_HRTFTable& table, const std::vector<orientation>& orientationsList, int extrapolationStep, TGapsFound gapsFound, TAzimuthElevationBorders borders, Functor f) {

			//THRIRStruct HRIRZeros = f(table);
			T_HRTFTable originalTable = table;

			if (gapsFound.gapMaxElevation) {
				for (double _elevation = 90; _elevation >= (borders.maxElevation + extrapolationStep); _elevation -= extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					int cont=0;
					for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {
						THRIRStruct newHRIR = f(originalTable, orientationsList, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<THRIRStruct>(newHRIR));						
					}
				}
			}
			if (gapsFound.gapMinElevation) {
				for (double _elevation = 270; _elevation <= (borders.minElevation - extrapolationStep); _elevation += extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {						
						THRIRStruct newHRIR = f(originalTable, orientationsList, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<THRIRStruct>(newHRIR));
					}
				}
			}

			if (gapsFound.gapMaxAzimuth) {
				// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
				double _minElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
				double _maxElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);

				for (double _elevation = _minElevationIn90Range; _elevation <= _maxElevationIn90Range; _elevation += extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = borders.maxAzimuth + extrapolationStep; _azimuth <= 180; _azimuth += extrapolationStep) {						
						THRIRStruct newHRIR = f(originalTable, orientationsList, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<THRIRStruct>(newHRIR));						
					}
				}
			}
			
			if (gapsFound.gapMinAzimuth) {
				// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
				double _minElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
				double _maxElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);				
				
				for (double _elevation = _minElevationIn90Range; _elevation <= _maxElevationIn90Range; _elevation += extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
										
					for (double _azimuth = borders.minAzimuth - extrapolationStep; _azimuth >= 180; _azimuth -= extrapolationStep) {					
						THRIRStruct newHRIR = f(originalTable, orientationsList, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<THRIRStruct>(newHRIR));												
					}
				}
				
			}
		}

	};
}
#endif
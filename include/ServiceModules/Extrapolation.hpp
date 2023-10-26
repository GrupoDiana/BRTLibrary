/**
* \class CExtrapolation
*
* \brief Declaration of CExtrapolationBasedNearestPoint classes interface
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
#include <ServiceModules/InterpolationAuxiliarMethods.hpp>


namespace BRTServices
{
	/**
	 * @brief Base class for extrapolation processors
	*/
	class CExtrapolation {
	public:
				
		/**
		 * @brief Look in the table to see if there are any large gaps and if there are, extrapolate to fill them
		 * @tparam T Type of the table 
		 * @tparam U Type of the table Value data
		 * @tparam Functor Structure containing an operator () function that calculates the extrapolation for a point.
		 * @param table Table of data to be extrapolated
		 * @param orientationsList List of guidelines contained in the table
		 * @param extrapolationStep  Extrapolation step 
		 * @param f Structure containing an operator () function that calculates the extrapolation for a point.
		*/
		template <typename T, typename U, typename Functor>
		void Process(T& table, const std::vector<orientation>& orientationsList, int _TFSize, int extrapolationStep, Functor f) {
			// Look for gaps and their borders
			TAzimuthElevationBorders borders;
			TGapsFound gapsFound = AreGapsInIRGrid(table, borders);
			
			FillGaps<T, U>(table, orientationsList, _TFSize, extrapolationStep, gapsFound, borders, f);
		};
		

	private:
				
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
		template <typename T>
		TGapsFound AreGapsInIRGrid(const T& table, TAzimuthElevationBorders& _borders) {
			int totalSourcePositions = table.size();
			double averageStep = 360 / std::sqrt(totalSourcePositions * M_PI);
			
			_borders = Find_AzimuthAndElevationBorders(table);

			TGapsFound gapsFound;			
			if (( 90 - _borders.maxElevation) > (2 * averageStep)) { gapsFound.gapMaxElevation = true; }
			if (( 90 + _borders.minElevation) > (2 * averageStep)) { gapsFound.gapMinElevation = true; }
			if ((180 - _borders.maxAzimuth) > (2 * averageStep)) { gapsFound.gapMaxAzimuth = true; }
			if ((180 + _borders.minAzimuth) > (2 * averageStep)) { gapsFound.gapMinAzimuth = true; }

			// Transforrm back to library ranges
			_borders.maxAzimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_borders.maxAzimuth);
			_borders.minAzimuth = CInterpolationAuxiliarMethods::CalculateAzimuthIn0_360Range(_borders.minAzimuth);
			_borders.maxElevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_borders.maxElevation);
			_borders.minElevation = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_borders.minElevation);
			
			return gapsFound;
		}

		/**
		 * @brief Find the maximum and minimum elevation and azimuth values.
		 * @param tableH RIR table where to look for gaps
		 * @return azimuth and elevation values
		*/
		template <typename T>
		TAzimuthElevationBorders Find_AzimuthAndElevationBorders(const T& table) {
			// Init values with the opposite
			TAzimuthElevationBorders borders(-18, 180, -90, 90);
			// Process
			for (auto it = table.begin(); it != table.end(); it++) {
				double _azimuthTemp = CInterpolationAuxiliarMethods::CalculateAzimuthIn180Range(it->first.azimuth);
				double _elevationTemp = CInterpolationAuxiliarMethods::CalculateElevationIn90Range(it->first.elevation);

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
		template <typename T, typename U, typename Functor>
		void FillGaps(T& table, const std::vector<orientation>& orientationsList, int _TFSize, int extrapolationStep, TGapsFound gapsFound, TAzimuthElevationBorders borders, Functor f) {
			
			T originalTable = table;

			if (gapsFound.gapMaxElevation) {
				for (double _elevation = 90; _elevation >= (borders.maxElevation + extrapolationStep); _elevation -= extrapolationStep) {
					double _elevationInRage = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					int cont=0;
					for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {
						U newTF = f(originalTable, orientationsList, _TFSize, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<U>(newTF));
					}
				}
			}
			if (gapsFound.gapMinElevation) {
				for (double _elevation = 270; _elevation <= (borders.minElevation - extrapolationStep); _elevation += extrapolationStep) {
					double _elevationInRage = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {						
						U newTF = f(originalTable, orientationsList, _TFSize, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<U>(newTF));
					}
				}
			}

			if (gapsFound.gapMaxAzimuth) {
				// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
				double _minElevationIn90Range = CInterpolationAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
				double _maxElevationIn90Range = CInterpolationAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);

				for (double _elevation = _minElevationIn90Range; _elevation <= _maxElevationIn90Range; _elevation += extrapolationStep) {
					double _elevationInRage = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = borders.maxAzimuth + extrapolationStep; _azimuth <= 180; _azimuth += extrapolationStep) {						
						U newTF = f(originalTable, orientationsList, _TFSize, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<U>(newTF));
					}
				}
			}
			
			if (gapsFound.gapMinAzimuth) {
				// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
				double _minElevationIn90Range = CInterpolationAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
				double _maxElevationIn90Range = CInterpolationAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);
				
				for (double _elevation = _minElevationIn90Range; _elevation <= _maxElevationIn90Range; _elevation += extrapolationStep) {
					double _elevationInRage = CInterpolationAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
										
					for (double _azimuth = borders.minAzimuth - extrapolationStep; _azimuth >= 180; _azimuth -= extrapolationStep) {					
						U newTF = f(originalTable, orientationsList, _TFSize, _azimuth, _elevationInRage);
						table.emplace(orientation(_azimuth, _elevationInRage), std::forward<U>(newTF));
					}
				}
				
			}
		}

	};
}
#endif
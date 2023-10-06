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
	class CExtrapolationInterface {
	public:
		virtual void Process(T_HRTFTable& table, int extrapolationStep) = 0;


	protected:
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

		

		TGapsFound AreGapsInIRGrid(const T_HRTFTable& table, TAzimuthElevationBorders& _borders) {
			int totalSourcePositions = table.size();
			double averageStep = 360 / std::sqrt(totalSourcePositions * M_PI);
			
			_borders = Find_AzimuthAndElevationBorders(table);

			TGapsFound gapsFound;			
			if (( 90 - _borders.maxElevation) > (2 * averageStep)) { gapsFound.gapMaxElevation = true; }
			if (( 90 + _borders.minElevation) > (2 * averageStep)) { gapsFound.gapMinElevation = true; }
			if ((180 - _borders.maxAzimuth) > (2 * averageStep)) { gapsFound.gapMaxAzimuth = true; }
			if ((180 + _borders.minAzimuth) > (2 * averageStep)) { gapsFound.gapMinAzimuth = true; }

			return gapsFound;
		}


		
		TAzimuthElevationBorders Find_AzimuthAndElevationBorders(const T_HRTFTable& table) {
			// Init values with the opposite
			TAzimuthElevationBorders borders(-18, 180, -90, 90);

			// Process
			for (auto it = table.begin(); it != table.end(); it++) {
				double _azimuthTemp = CHRTFAuxiliarMethods::CalculateAzimuthIn180Range(it->first.azimuth);
				double _elevationTemp = CHRTFAuxiliarMethods::CalculateElevationIn90Range(it->first.elevation);

				borders.maxAzimuth	= (_azimuthTemp		> borders.maxAzimuth)		? _azimuthTemp	: borders.maxAzimuth;
				borders.minAzimuth	= (_azimuthTemp		< borders.minAzimuth)		? _azimuthTemp	: borders.minAzimuth;
				borders.maxElevation = (_elevationTemp	> borders.maxElevation)		? _elevationTemp : borders.maxElevation;
				borders.minElevation = (_elevationTemp	< borders.minElevation)		? _elevationTemp : borders.minElevation;
			}

			borders.maxAzimuth	= CHRTFAuxiliarMethods::CalculateAzimuthIn0_360Range(borders.maxAzimuth);
			borders.minAzimuth	= CHRTFAuxiliarMethods::CalculateAzimuthIn0_360Range(borders.minAzimuth);
			borders.maxElevation = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(borders.maxElevation);
			borders.minElevation = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(borders.minElevation);

			return borders;
		}

		template <typename F>
		void FillGaps(T_HRTFTable& table, int extrapolationStep, TGapsFound gapsFound, TAzimuthElevationBorders borders, F f) {


			THRIRStruct HRIRZeros = f(table);


			if (gapsFound.gapMaxElevation) {
				for (double _elevation = 90; _elevation > (borders.maxElevation + extrapolationStep); _elevation -= extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {
						table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
					}
				}
			}
			if (gapsFound.gapMinElevation) {
				for (double _elevation = 270; _elevation < (borders.minElevation - extrapolationStep); _elevation += extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {
						table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
					}
				}
			}

			if (gapsFound.gapMaxAzimuth) {
				// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
				double _minElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
				double _maxElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);

				for (double _elevation = _minElevationIn90Range; _elevation < _maxElevationIn90Range; _elevation += extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = borders.maxAzimuth + extrapolationStep; _azimuth <= 180; _azimuth += extrapolationStep) {
						table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
					}
				}
			}
			if (gapsFound.gapMinAzimuth) {
				// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
				double _minElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
				double _maxElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);

				for (double _elevation = _minElevationIn90Range; _elevation < _maxElevationIn90Range; _elevation += extrapolationStep) {
					double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
					for (double _azimuth = borders.minAzimuth - extrapolationStep; _azimuth >= 180; _azimuth -= extrapolationStep) {
						table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
					}
				}
			}
		}

	};

	
	/**
	 * @brief
	*/
	class CZeroInsertionBasedExtrapolation : CExtrapolationInterface {		
	public:
		
		void Process(T_HRTFTable& table, int extrapolationStep) {
			
			// Initialization
			//int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one
			/*THRIRStruct HRIRZeros;
			HRIRZeros.leftHRIR.resize(HRIRSize, 0);
			HRIRZeros.rightHRIR.resize(HRIRSize, 0);*/

			// Look for gaps and their borders
			TAzimuthElevationBorders borders;			
			TGapsFound gapsFound = AreGapsInIRGrid(table, borders);
			
			FillGaps(table, extrapolationStep, gapsFound, borders, GetZerosHRIR());



			//// Fill gaps
			//if (gapsFound.gapMaxElevation) {
			//	for (double _elevation = 90; _elevation > (borders.maxElevation + extrapolationStep); _elevation-= extrapolationStep) {
			//		double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
			//		for (double _azimuth = 0; _azimuth < 360; _azimuth+= extrapolationStep) {							
			//			table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
			//		}
			//	}
			//}
			//if (gapsFound.gapMinElevation) {
			//	for (double _elevation = 270; _elevation < (borders.minElevation - extrapolationStep); _elevation += extrapolationStep) {
			//		double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
			//		for (double _azimuth = 0; _azimuth < 360; _azimuth += extrapolationStep) {					
			//			table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);						
			//		}
			//	}
			//}

			//if (gapsFound.gapMaxAzimuth) {
			//	// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
			//	double _minElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);				
			//	double _maxElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);
			//	
			//	for (double _elevation = _minElevationIn90Range; _elevation < _maxElevationIn90Range; _elevation += extrapolationStep) {					
			//		double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);					
			//		for (double _azimuth = borders.maxAzimuth + extrapolationStep; _azimuth <= 180; _azimuth += extrapolationStep) {
			//			table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
			//		}
			//	}
			//}
			//if (gapsFound.gapMinAzimuth) {
			//	// We need to loop from minimun to maximum posible elevation, so we change the elevation to [-90 to 90] range
			//	double _minElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.minElevation);
			//	double _maxElevationIn90Range = CHRTFAuxiliarMethods::CalculateElevationIn90Range(borders.maxElevation);

			//	for (double _elevation = _minElevationIn90Range; _elevation < _maxElevationIn90Range; _elevation += extrapolationStep) {
			//		double _elevationInRage = CHRTFAuxiliarMethods::CalculateElevationIn0_90_270_360Range(_elevation);
			//		for (double _azimuth = borders.minAzimuth - extrapolationStep; _azimuth >= 180; _azimuth -= extrapolationStep) {
			//			table.emplace(orientation(_azimuth, _elevationInRage), HRIRZeros);
			//		}
			//	}
			//}
		}

	private:
				
		struct GetZerosHRIR {
			THRIRStruct operator() (const T_HRTFTable& table) {				
				// Initialization
				int HRIRSize = table.begin()->second.leftHRIR.size();	// Justa took the first one
				THRIRStruct HRIRZeros;
				HRIRZeros.leftHRIR.resize(HRIRSize, 0);
				HRIRZeros.rightHRIR.resize(HRIRSize, 0);
				return HRIRZeros;
			}
		};

	};

	/**
	 * @brief
	*/
	class CNearestPointBasedExtrapolation : CExtrapolationInterface {
	public:
		void Process(T_HRTFTable& table, int extrapolationStep) {

		}
	};



}
#endif
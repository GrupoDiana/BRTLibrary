/**
* \class CAmbisonicEncoder
*
* \brief Declaration of CAmbisonicEncoder class interface.
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

#ifndef _CAMBISONIC_ENCODER_HPP_
#define _CAMBISONIC_ENCODER_HPP_

#include <Common/Buffer.hpp>

namespace BRTProcessing {

	enum TAmbisonicNormalization { none, N3D, SN3D, maxN };

	class CAmbisonicEncoder {
	public:
		
		/**
			\brief Default constructor
		*/
		CAmbisonicEncoder() : initialized{ false }, ambisonicOrder { 1 }, normalization{ TAmbisonicNormalization::N3D }/*, bufferSize{ 0 }*/ {
			numberOfChannels = CalculateNumberOfChannels(ambisonicOrder);
		}

		/**
		 * @brief Initialises the class according to the order and renames it to use
		 * @param _ambisonicOrder The orders allowed are 1, 2 or 3.
		 * @param _ambisonicNormalization Possible normalisations are :N3D, SN3D, maxN
		 * @param _bufferSize The buffer size
		*/
		void Setup(int _ambisonicOrder, TAmbisonicNormalization _ambisonicNormalization/*, int _bufferSize*/) {			
			
			if (_ambisonicOrder >= 1 && _ambisonicOrder <= 3) {
				//bufferSize = _bufferSize;
				ambisonicOrder = _ambisonicOrder;
				normalization = _ambisonicNormalization;
				numberOfChannels = CalculateNumberOfChannels(ambisonicOrder);
				initialized = true;
			}
			
		}

		/**
		 * @brief Set to initial state
		*/
		void Reset() {
			initialized = false;
			//bufferSize = 0;
			ambisonicOrder = 1;
			normalization = TAmbisonicNormalization::N3D;
			numberOfChannels = CalculateNumberOfChannels(ambisonicOrder);			
		}

		/**
		 * @brief Calculate the number of ambisonic channels from the ambisonic order
		 * @return Number of ambisonic channels
		*/
		static int CalculateNumberOfChannels(int _ambisonicOrder) { return pow((_ambisonicOrder + 1), 2); }

		/**
		 * @brief Get the set ambisonic order
		 * @return ambisonic order
		*/
		int GetOrder() { return ambisonicOrder; }

		/**
		 * @brief Get the number of ambisonic channels. It has been set according to the established order
		 * @return number of ambisonic channels
		*/
		int GetTotalChannels() { return numberOfChannels; }

		
		/**
		 * @brief Init ambisonic channels 
		 * @return vector of as many CMonoBuffers as ambisonic channels
		*/
		void InitAmbisonicChannels(std::vector< CMonoBuffer<float>>& channelsBuffers, int bufferSize) {
			if (!initialized) { 
				SET_RESULT(RESULT_ERROR_NOTSET, "AmbisonicEncoder class not initialised");
				channelsBuffers = std::vector< CMonoBuffer<float> >();
			}

			channelsBuffers = std::vector< CMonoBuffer<float>>(GetTotalChannels(), CMonoBuffer<float>(bufferSize, 0.0f));			
		}

		/**
		 * @brief Performs coding of all ambisonic channels as a function of azimuth and elevation.
		 * @param inBuffer Input samples
		 * @param outVectorOfBuffers vector of as many CMonoBuffers as ambisonic channels
		 * @param azimuth source azimuth
		 * @param elevation source elevetaion
		*/		
		void EncodedIR(const CMonoBuffer<float>& inBuffer, std::vector< CMonoBuffer<float> >& channelsOutBuffers, float _azimuthDegress, float _elevationDegress) {

			if (!initialized) { 
				SET_RESULT(RESULT_ERROR_NOTSET, "AmbisonicEncoder class not initialised");
				return; 
			}			
			
			std::vector<double> ambisonicFactors = GetRealSphericalHarmonics(DegreesToRadians(_azimuthDegress), DegreesToRadians(_elevationDegress));
			
			for (int nChannel = 0; nChannel < GetTotalChannels(); nChannel++) {				
				for (int nSample = 0; nSample < inBuffer.size(); nSample++)	{							
					channelsOutBuffers[nChannel][nSample] += inBuffer[nSample] * ambisonicFactors[nChannel];					
				}
			}
		}
		
		void EncodedPartitionedIR(const std::vector<CMonoBuffer<float>>& inPartitionedBuffer, std::vector<std::vector< CMonoBuffer<float>>>& partitionedChannelsOutBuffers, float _azimuthDegress, float _elevationDegress) {

			if (!initialized) {
				SET_RESULT(RESULT_ERROR_NOTSET, "AmbisonicEncoder class not initialised");
				return;
			}
			int numberOfChannels = GetTotalChannels();
			int numberOfPartitions = inPartitionedBuffer.size();
			int partitionsSize = inPartitionedBuffer[0].size();		// They must all be the same
						

			std::vector<double> ambisonicFactors = GetRealSphericalHarmonics(DegreesToRadians(_azimuthDegress), DegreesToRadians(_elevationDegress));

			for (int nChannel = 0; nChannel < numberOfChannels; nChannel++) {
				for (int nPartition = 0; nPartition < numberOfPartitions; nPartition++) {
					for (int nSample = 0; nSample < partitionsSize; nSample++) {
						partitionedChannelsOutBuffers[nChannel][nPartition][nSample] += inPartitionedBuffer[nPartition][nSample] * ambisonicFactors[nChannel];
					}
				}
			}
		}
		

	private:

		// Attributes
		bool initialized;
		//int bufferSize;
		int ambisonicOrder;
		int numberOfChannels;
		TAmbisonicNormalization normalization;
	

		/////////////////////
		// PRIVATE METHODS
		/////////////////////
		
		/**
		 * @brief Get the ambisonic factors from the configured ambisonic order and normalisation and from the given azimuth and elevation.
		 * @param _ambisonicAzimut azimuth to calculate the factors
		 * @param _ambisonicElevation elevation to calculate the factors
		 * @return Vector of floats containing the factors in order, the size of the vector will depend on the order [4, 9, 16].
		*/
		std::vector<double> GetRealSphericalHarmonics(double _ambisonicAzimut, double _ambisonicElevation) {
			if (!initialized) { return std::vector<double>(); }

			std::vector<double> _factors(GetTotalChannels());	// Init
			
			switch (GetOrder())
			{
			case 3:
				_factors[9] = sqrt(35 / 8) * pow(cos(_ambisonicElevation), 3) * sin(3 * _ambisonicAzimut);
				_factors[10] = (sqrt(105) / 2) * sin(_ambisonicElevation) * pow(cos(_ambisonicElevation), 2) * sin(2 * _ambisonicAzimut);
				_factors[11] = sqrt(21 / 8) * cos(_ambisonicElevation) * (5 * pow(sin(_ambisonicElevation), 2) - 1) * sin(_ambisonicAzimut);
				_factors[12] = (sqrt(7) / 2) * sin(_ambisonicElevation) * (5 * pow(sin(_ambisonicElevation), 2) - 3);
				_factors[13] = sqrt(21 / 8) * cos(_ambisonicElevation) * (5 * pow(sin(_ambisonicElevation), 2) - 1) * cos(_ambisonicAzimut);
				_factors[14] = (sqrt(105) / 2) * sin(_ambisonicElevation) * pow(cos(_ambisonicElevation), 2) * cos(2 * _ambisonicAzimut);
				_factors[15] = sqrt(35 / 8) * pow(cos(_ambisonicElevation), 3) * cos(3 * _ambisonicAzimut);

			case 2:
				_factors[4] = (sqrt(15) / 2) * pow(cos(_ambisonicElevation), 2) * sin(2 * _ambisonicAzimut);
				_factors[5] = (sqrt(15) / 2) * sin(2 * _ambisonicElevation) * sin(_ambisonicAzimut);
				_factors[6] = (sqrt(5) / 2) * (3 * pow(sin(_ambisonicElevation), 2) - 1);
				_factors[7] = (sqrt(15) / 2) * sin(2 * _ambisonicElevation) * cos(_ambisonicAzimut);
				_factors[8] = (sqrt(15) / 2) * pow(cos(_ambisonicElevation), 2) * cos(2 * _ambisonicAzimut);


			case 1:
				_factors[0] = 1;
				_factors[1] = sqrt(3) * cos(_ambisonicElevation) * sin(_ambisonicAzimut);
				_factors[2] = sqrt(3) * sin(_ambisonicElevation);
				_factors[3] = sqrt(3) * cos(_ambisonicElevation) * cos(_ambisonicAzimut);
			default:
				break;

			}

			if (normalization == TAmbisonicNormalization::SN3D) { ConvertN3DtoSN3D(_factors); }
			else if (normalization == TAmbisonicNormalization::maxN) { ConvertN3DtoMaxN(_factors); }
			
			return _factors;
		}
		/**
		 * @brief Apply a normalisation to the ambisonic factors
		 * @param _factors 
		*/
		void ConvertN3DtoSN3D(std::vector<double>& _factors) {
			for (int i = 1; i < _factors.size(); i++) {
				if (i < 4) { _factors[i] *= (1 / sqrt(3)); }
				else if (i < 9) { _factors[i] *= (1 / sqrt(5)); }
				else if (i < 16) { _factors[i] *= (1 / sqrt(7)); }
			}
		}
		/**
		 * @brief Apply a normalisation to the ambisonic factors
		 * @param _factors 
		*/
		void ConvertN3DtoMaxN(std::vector<double>& _factors) {
			switch (GetOrder())
			{
			case 3:
				_factors[9] *= sqrt(8 / 35);
				_factors[10] *= 3 / sqrt(35);
				_factors[11] *= sqrt(45 / 224);
				_factors[12] *= 1 / sqrt(7);
				_factors[13] *= sqrt(45 / 224);
				_factors[14] *= 3 / sqrt(35);
				_factors[15] *= sqrt(8 / 35);

			case 2:
				_factors[4] *= 2 / sqrt(15);
				_factors[5] *= 2 / sqrt(15);
				_factors[6] *= 1 / sqrt(5);
				_factors[7] *= 2 / sqrt(15);
				_factors[8] *= 2 / sqrt(15);


			case 1:
				_factors[0] *= 1 / sqrt(2);
				_factors[1] *= 1 / sqrt(3);
				_factors[2] *= 1 / sqrt(3);
				_factors[3] *= 1 / sqrt(3);
			default:
				break;

			}
		}

		double DegreesToRadians(double _degrees) {
			return (_degrees / 180.0) * ((double)M_PI);
		}		
	};
}
#endif
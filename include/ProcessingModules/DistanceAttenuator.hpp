/**
* \class CDistanceAttenuator
*
* \brief Declaration of CDistanceAttenuation class
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
#ifndef _DISTANCE_ATTENUATION_
#define _DISTANCE_ATTENUATION_

#include <Common/Buffer.hpp>
#include <Common/CommonDefinitions.hpp>
#include <algorithm>
#include <memory>
#include <vector>

#define EPSILON_ATT 0.0001
#define EPSILON_DISTANCE 0.0001f
#define FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB -6.0206f ///< std::log10(0.5f) * 20.0f

namespace BRTProcessing {

class CDistanceAttenuator {
public:
	CDistanceAttenuator()
		: enableProcessor { true }
		, setupDone { false }
		, distanceAttenuationFactorDB { 0 }
		, referenceAttenuationDistance { 0 }
		, previousAttenuation_Channel { 0 } {
	}

	/**
         * @brief Set up the distance attenuation processor
         * @param _distanceAttenuationFactorDB Distance attenuation factor in decibels
         * @param _attenuationReferenceDistance Distance at which the attenuation is 0 dB, in meters.
         */
	void Setup(float _distanceAttenuationFactorDB, float _attenuationReferenceDistance) {
		std::lock_guard<std::mutex> l(mutex);
		if (_distanceAttenuationFactorDB > 0) {
			SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation factor in decibels must be a negative value");
			return;
		}
		if (_attenuationReferenceDistance <= 0) {
			SET_RESULT(RESULT_ERROR_PHYSICS, "Reference distance must be a positive value");
			return;
		}
		distanceAttenuationFactorDB = _distanceAttenuationFactorDB;
		referenceAttenuationDistance = _attenuationReferenceDistance;
		setupDone = true;
	}

	/**
         * @brief Enable distance attenuation process
         */
	void EnableProcessor() { enableProcessor = true; }

	/**
         * @brief Disable distance attenuation process
         */
	void DisableProcessor() { enableProcessor = false; }

	/**
         * @brief Check if distance attenuation process is enabled
         * @return True if the processor is enabled
         */
	bool IsProcessorEnabled() { return enableProcessor; }

	/**
         * @brief Set the distance attenuation factor in decibels
         * @param _distanceAttenuationFactorDB distance attenuation factor in decibels
         */
	void SetDistanceAttenuationFactor(float _distanceAttenuationFactorDB) {
		std::lock_guard<std::mutex> l(mutex);
		if (_distanceAttenuationFactorDB > 0) {
			SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation factor in decibels must be a negative value");
			return;
		}
		distanceAttenuationFactorDB = _distanceAttenuationFactorDB;
	}

	/**
         * @brief Get the distance attenuation factor in decibels
         * @return distance attenuation factor in decibels
         */
	float GetDistanceAttenuationFactor() {
		return distanceAttenuationFactorDB;
	}

	/**
         * @brief Set the distance attenuation reference distance. This is the distance at which the attenuation is 0 dB
         * @param _referenceAttenuationDistance Distance at which the attenuation is 0 dB, in meters.
         */
	void SetReferenceAttenuationDistance(float _referenceAttenuationDistance) {
		std::lock_guard<std::mutex> l(mutex);
		if (_referenceAttenuationDistance <= 0) {
			SET_RESULT(RESULT_ERROR_PHYSICS, "Reference distance must be a positive value");
			return;
		}
		referenceAttenuationDistance = _referenceAttenuationDistance;
	}

	/**
         * @brief Get the distance attenuation reference distance
         * @return attenuation reference distance
         */
	float GetReferenceAttenuationDistance() {
		return referenceAttenuationDistance;
	}

	/**
         * @brief Apply distance attenuation to the input buffer
         * @param _inBuffer input buffer
         * @param outBuffer output buffer attenuated by distance between source and listener
         * @param sourceTransform source transform
         * @param listenerTransform listener transform
         */
	void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, Common::CTransform sourceTransform, Common::CTransform listenerTransform) {
		std::lock_guard<std::mutex> l(mutex);
		if (!setupDone) {
			SET_RESULT(RESULT_ERROR_NOTSET, "DistanceAttenuator has not been set up", "");
			return;
		}

		ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");

		outBuffer = _inBuffer;

		if (!enableProcessor) {
			return;
		}

		// Calculate distance
		float distance = CalculateDistance(sourceTransform, listenerTransform);

		// Attenuation is computed	
		float attenuation = GetDistanceAttenuation(distanceAttenuationFactorDB, distance, 0);

		//Apply attenuation gradually using Exponential Moving Average method
		float unnecessary_fixme;
		if (outBuffer.size() != 0)
			outBuffer.ApplyGainExponentially(previousAttenuation_Channel, unnecessary_fixme, attenuation, globalParameters.GetBufferSize(), globalParameters.GetSampleRate());
	}

private:
	mutable std::mutex mutex; // Thread management

	bool enableProcessor;
	bool setupDone;
	float distanceAttenuationFactorDB; // Attenuation factor for distance attenuation
	float referenceAttenuationDistance; // Distance at which the attenuation is 0 dB, in meters.
	float previousAttenuation_Channel;
	Common::CGlobalParameters globalParameters;

	/////////////////////
	/// PRIVATE Methods
	/////////////////////

	float GetDistanceAttenuation(float attenuationForDuplicateDistance, float distance, float extraAttennuation_dB) const {
		// Error handler:
		if (distance <= 0.0f) {
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Attempt to compute distance attenuation for a negative or zero distance");
			return 1.0f;
		}
		//else
		//	SET_RESULT(RESULT_OK, "Distance attenuation returned succesfully for single source");

		// Compute gain
		if (distance > EPSILON_DISTANCE && std::fabs(attenuationForDuplicateDistance) > EPSILON_ATT) {
			// Compute attenuation factor
			float attenuationFactor = attenuationForDuplicateDistance / FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB;

			return std::pow(10.0f, extraAttennuation_dB + attenuationFactor * std::log10(referenceAttenuationDistance / distance));
		} else
			return 1.0;
	}

	float CalculateDistance(Common::CTransform _sourceTransform, Common::CTransform _listenerTransform) {

		//Get azimuth and elevation between listener and source
		Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
		float _distanceToListener = _vectorToListener.GetDistance();
		return _distanceToListener;
	}
};
}
#endif
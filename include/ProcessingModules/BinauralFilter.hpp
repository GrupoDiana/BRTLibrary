/**
* \class CBinauralFilter
*
* \brief Declaration of CBinauralFilter class. It implements binaural filtering from second-order stages.
* \date	Nov 2024
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
#ifndef _BINAURAL_FILTER_HPP
#define _BINAURAL_FILTER_HPP

#define EPSILON 0.001f


#include <Common/Buffer.hpp>
#include <ServiceModules/SOSFilters.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>

#define NUMBER_OF_COEFFICIENTS_IN_STAGE_SOS 6

namespace BRTProcessing {

	class CBinauralFilter {
	public:
		CBinauralFilter()
			: enableProcessor { true }
			, initialized { false }
			, numberOfCoefficientsPerEar { 0 }
		{
					
		}

		/**
		 * @brief Configure the filter, according to the number of second order stages you tell it to set.
		 * @param _numberOfFilterStages number of second order stages to set
		 */
		void Setup(int _numberOfFilterStages) { 
			if (initialized) return;
			
			if (_numberOfFilterStages < 0) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "The number of filter stages has to be greater than 0 in Common::CBinauralFilter");				
				return;
			}

			for (int i = 0; i < _numberOfFilterStages; i++) {
				filtersChain.left.AddFilter();	//Initialize the filter 
				filtersChain.right.AddFilter();	//Initialize the filter
			}			

			numberOfCoefficientsPerEar = _numberOfFilterStages * NUMBER_OF_COEFFICIENTS_IN_STAGE_SOS;
			initialized = true;
		}
		
		/**
		 * @brief Store the coefficients of the filter. For a filter independent of the source and listener position.
		 * @param _coefficientsLeft vector of coefficients for the left ear
		 * @param _coefficientsRight vector of coefficients for the right ear
		 */
		void SetCoefficients(std::vector<float>& _coefficientsLeft, std::vector<float>& _coefficientsRight) {
			if (!initialized) return;

			if ((_coefficientsLeft.size() != numberOfCoefficientsPerEar) || (_coefficientsRight.size() != numberOfCoefficientsPerEar)) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "The number of coefficients has to be equal to the number of filter stages times 6 in Common::CBinauralFilter");
				return;
			}
			SetCoefficients(filtersChain.left, _coefficientsLeft);		//Set LEFT coefficients
			SetCoefficients(filtersChain.right, _coefficientsRight);	//Set RIGHT coefficients			
		}
		
		/**
		 * @brief Enable processor
		 */
		void EnableProcessor() { enableProcessor = true; }
		/**
		 * @brief Disable processor
		 */
		void DisableProcessor() { enableProcessor = false; }
		/**
		 * @brief Get the flag to know if the processor is enabled.
		 * @return true if the processor is enabled, false otherwise
		 */
		bool IsProcessorEnabled() { return enableProcessor; }
		

		/**
		 * @brief Filter the input signal with the binaural filter taking into account the source and listener position
		 * @param _inLeftBuffer left ear input buffer
		 * @param _inRightBuffer right ear input buffer
		 * @param outLeftBuffer out left ear buffer
		 * @param outRightBuffer out right ear buffer
		 * @param sourceTransform source position and orientation
		 * @param listenerTransform listener position and orientation
		 * @param _SOSFilterWeakPtr pointer to SOS filter 
		 */
		void Process(CMonoBuffer<float>& _inLeftBuffer, CMonoBuffer<float>& _inRightBuffer, CMonoBuffer<float>& outLeftBuffer, CMonoBuffer<float>& outRightBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CSOSFilters>& _SOSFilterWeakPtr)
		{
			outLeftBuffer = _inLeftBuffer;
			outRightBuffer = _inRightBuffer;
			
			if (!initialized) return;

			// Check process flag
			if (!enableProcessor) { return;	}
			
			float distance = Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);			
			
			if (distance > DISTANCE_MODEL_THRESHOLD_NEAR) {
				return;
			}
			if (Common::AreSame(distance, 0, MINIMUM_DISTANCE_SOURCE_LISTENER)) {
				SET_RESULT(RESULT_WARNING, "The source is inside the listener's head.");
				outLeftBuffer = _inLeftBuffer;
				outRightBuffer = _inRightBuffer;
				return;
			}

			ASSERT(_inLeftBuffer.size() == globalParameters.GetBufferSize() || _inRightBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");			
			
			// Check listener ILD
			std::shared_ptr<BRTServices::CSOSFilters> _listenerILD = _SOSFilterWeakPtr.lock();
			if (!_listenerILD) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "ILD listener pointer is null when trying to use in BRTProcessing::CNearFieldEffect");
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			float interauralAzimuth = CalculateInterauralAzimuth(sourceTransform, listenerTransform);

			//Get coefficients from the ILD table
			std::vector<float> coefficientsLeft = _listenerILD->GetSOSFilterCoefficients(Common::T_ear::LEFT, distance, interauralAzimuth);
			std::vector<float> coefficientsRight = _listenerILD->GetSOSFilterCoefficients(Common::T_ear::RIGHT, distance, interauralAzimuth);
			
			/*if (coefficientsLeft.size() != 12 || coefficientsRight.size() != 12) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "Twelve coefficients were expected in order to be able to set up the filters in BRTProcessing::CNearFieldEffect");
				return;
			}*/						
			
			SetCoefficients(filtersChain.left, coefficientsLeft);		//Set LEFT coefficients 			 
			SetCoefficients(filtersChain.right, coefficientsRight);	//Set RIGHT coefficients

			// Process the signal
			filtersChain.left.Process(outLeftBuffer);
			filtersChain.right.Process(outRightBuffer);									
		}


		/**
		 * @brief Filter the input signal with the binaural filter
		 * @param _inLeftBuffer left ear input buffer
		 * @param _inRightBuffer right ear input buffer
		 * @param outLeftBuffer out left ear buffer
		 * @param outRightBuffer out right ear buffer
		 */
		void Process(CMonoBuffer<float> & _inLeftBuffer, CMonoBuffer<float> & _inRightBuffer, CMonoBuffer<float> & outLeftBuffer, CMonoBuffer<float> & outRightBuffer)
		{
			outLeftBuffer = _inLeftBuffer;
			outRightBuffer = _inRightBuffer;
			
			if (!initialized) return;						
			if (!enableProcessor) return;			
			
			ASSERT(_inLeftBuffer.size() == globalParameters.GetBufferSize() || _inRightBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
							
			// Process the signal
			filtersChain.left.Process(outLeftBuffer);
			filtersChain.right.Process(outRightBuffer);
		}

		/**
		 * @brief Reset the buffers of the process
		 */
		void ResetProcessBuffers() {
			filtersChain.left.ResetBuffers();
			filtersChain.right.ResetBuffers();			
		}

	
	private:
		///////////////////////
		// Private Methods
		///////////////////////		

		/// Calculates the parameters derived from the source and listener position
		float CalculateInterauralAzimuth(Common::CTransform& _sourceTransform, Common::CTransform& _listenerTransform)
		{

			//Get azimuth and elevation between listener and source
			Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
			float _distanceToListener = _vectorToListener.GetDistance();

			//Check listener and source are in the same position
			if (_distanceToListener <= MINIMUM_DISTANCE_SOURCE_LISTENER) {
				SET_RESULT(RESULT_WARNING, "The sound source is too close to the centre of the listener's head in BRTProcessing::CNearFieldEffect");
				return MINIMUM_DISTANCE_SOURCE_LISTENER;
			}						
			return _vectorToListener.GetInterauralAzimuthDegrees();			
		}

		
		/**
		 * @brief Set the coefficients of the filter
		 * @param _filter filter to set the coefficients
		 * @param cofficients vector of coefficients
		 */	
		void SetCoefficients(Common::CFiltersChain & _filter, std::vector<float> & cofficients) {
			
			Common::TFiltersChainCoefficients filterCoeficientsVector;
			for (int i = 0; i < numberOfCoefficientsPerEar; i += NUMBER_OF_COEFFICIENTS_IN_STAGE_SOS) {
				std::vector<float> stage(cofficients.begin() + i, cofficients.begin() + i + NUMBER_OF_COEFFICIENTS_IN_STAGE_SOS);
				filterCoeficientsVector.push_back(stage);
			}
			_filter.SetFromCoefficientsVector(filterCoeficientsVector);			
		}

		///////////////////////
		// Private Attributes
		///////////////////////		
		Common::CGlobalParameters globalParameters;
		Common::CEarPair<Common::CFiltersChain> filtersChain;		// Computes the Near field effects
		bool enableProcessor;										// Flag to enable the processor		
		bool initialized;
		int numberOfCoefficientsPerEar;
	};
}
#endif
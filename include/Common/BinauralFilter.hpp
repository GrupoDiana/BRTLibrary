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
#ifndef _BINAURAL_FILTER_HPP
#define _BINAURAL_FILTER_HPP


#include <Common/Buffer.hpp>
#include <ServiceModules/NFCFilters.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>


namespace Common {

	class CBinauralFilter {
	public:
		CBinauralFilter()
			: enableProcessor { true }
			, initialized { false }
		{
		
			//nearFieldEffectFilters.left.AddFilter();		//Initialize the filter to ILD simulation 
			//nearFieldEffectFilters.left.AddFilter();		//Initialize the filter to ILD simulation
			//nearFieldEffectFilters.right.AddFilter();		//Initialize the filter to ILD simulation
			//nearFieldEffectFilters.right.AddFilter();		//Initialize the filter to ILD simulation
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

			numberOfCoefficientsPerChannel = _numberOfFilterStages * 6;
			initialized = true;
		}
		
		void SetCoefficients(std::vector<float>& _coefficientsLeft, std::vector<float>& _coefficientsRight) {
			if (!initialized) return;

			if ((_coefficientsLeft.size() != numberOfCoefficientsPerChannel) || (_coefficientsRight.size() != numberOfCoefficientsPerChannel)) {
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
		 * @brief 
		 * @param _inLeftBuffer 
		 * @param _inRightBuffer 
		 * @param outLeftBuffer 
		 * @param outRightBuffer 
		 * @param sourceTransform 
		 * @param listenerTransform 
		 * @param _listenerILDWeak 
		 */
		void Process(CMonoBuffer<float>& _inLeftBuffer, CMonoBuffer<float>& _inRightBuffer, CMonoBuffer<float>& outLeftBuffer, CMonoBuffer<float>& outRightBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CNearFieldCompensationFilters>& _listenerILDWeak)

		{
			outLeftBuffer = _inLeftBuffer;
			outRightBuffer = _inRightBuffer;

			// Check process flag
			if (!enableProcessor) { return;	}
			
			float distance = CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);
			if (distance > DISTANCE_MODEL_THRESHOLD_NEAR) {	return; }
						
			ASSERT(_inLeftBuffer.size() == globalParameters.GetBufferSize() || _inRightBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");			
			
			// Check listener ILD
			std::shared_ptr<BRTServices::CNearFieldCompensationFilters> _listenerILD = _listenerILDWeak.lock();
			if (!_listenerILD) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "ILD listener pointer is null when trying to use in BRTProcessing::CNearFieldEffect");
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			float interauralAzimuth = CalculateInterauralAzimuth(sourceTransform, listenerTransform);

			//Get coefficients from the ILD table
			std::vector<float> coefficientsLeft = _listenerILD->GetILDNearFieldEffectCoefficients(Common::T_ear::LEFT, distance, interauralAzimuth);
			std::vector<float> coefficientsRight = _listenerILD->GetILDNearFieldEffectCoefficients(Common::T_ear::RIGHT, distance, interauralAzimuth);
			
			if (coefficientsLeft.size() != 12 || coefficientsRight.size() != 12) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "Twelve coefficients were expected in order to be able to set up the filters in BRTProcessing::CNearFieldEffect");
				return;
			}						
			
			SetCoefficients(filtersChain.left, coefficientsLeft);		//Set LEFT coefficients 			 
			SetCoefficients(filtersChain.right, coefficientsRight);	//Set RIGHT coefficients

			// Process the signal
			filtersChain.left.Process(outLeftBuffer);
			filtersChain.right.Process(outRightBuffer);									
		}


		
		void Process(CMonoBuffer<float> & _inLeftBuffer, CMonoBuffer<float> & _inRightBuffer, CMonoBuffer<float> & outLeftBuffer, CMonoBuffer<float> & outRightBuffer)
		{
			outLeftBuffer = _inLeftBuffer;
			outRightBuffer = _inRightBuffer;

			// Check process flag
			if (!enableProcessor) {
				return;
			}
			
			ASSERT(_inLeftBuffer.size() == globalParameters.GetBufferSize() || _inRightBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
							
			// Process the signal
			filtersChain.left.Process(outLeftBuffer);
			filtersChain.right.Process(outRightBuffer);
		}


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

		/*void SetCoefficients(Common::CFiltersChain& _filter, std::vector<float>& cofficients) {
			Common::TFiltersChainCoefficients filterCoeficientsVector;
			std::vector<float> firstStage(cofficients.begin(), cofficients.begin() + 6);
			std::vector<float> secondStage(cofficients.begin() + 6, cofficients.end());

			filterCoeficientsVector.push_back(firstStage);
			filterCoeficientsVector.push_back(secondStage);

			_filter.SetFromCoefficientsVector(filterCoeficientsVector);		
		}*/

		void SetCoefficients(Common::CFiltersChain & _filter, std::vector<float> & cofficients) {
			
			Common::TFiltersChainCoefficients filterCoeficientsVector;
			for (int i = 0; i < numberOfCoefficientsPerChannel; i += 6) {
				std::vector<float> stage(cofficients.begin() + i, cofficients.begin() + i + 6);
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
		int numberOfCoefficientsPerChannel;
	};
}
#endif
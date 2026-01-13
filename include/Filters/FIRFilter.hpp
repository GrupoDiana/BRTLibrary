/**
* \class CFIRFilter
*
* \brief Declaration of CFIRFilter class
* \date	Nov 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/


#ifndef _FIR_FILTER_HPP
#define _FIR_FILTER_HPP

#define EPSILON 0.001f


#include <Common/Buffer.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>
#include <ServiceModules/GeneralFIR.hpp>
#include <Filters/FilterBase.hpp>
#include <ProcessingModules/FIRConvolver.hpp>


namespace BRTFilters {

	class CFIRFilter : public CFilterBase, private BRTProcessing::CFIRConvolver {
	public:
		CFIRFilter()
			: CFilterBase { TFilterType::FIR }
			, firTable { nullptr }
			/*, initialized { false }
			, numberOfChannels { 0 }
			, numberOfBiquadSectionsPerChannel { 0 }
			, numberOfCoefficientsPerChannel { 0 }*/
		{
					
		}

										
		/**
		 * @brief Enable processor
		 */
		void Enable() override {
			enable = true;
			BRTProcessing::CFIRConvolver::EnableProcessor();
		};

		///**
		// * @brief Disable processor
		// */
		void Disable() override {
			enable = false;
			BRTProcessing::CFIRConvolver::DisableProcessor();
		};
				

		/**
		 * @brief Set the FIR table to be used in the convolution
		 * @param _firTable fir table pointer
		 * @return 
		 */
		bool SetFIRTable(std::shared_ptr<BRTServices::CGeneralFIR> _firTable) override {

			if (_firTable->GetSamplingRate() != globalParameters.GetSampleRate()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This FIR has not been assigned. The sample rate of the FIR does not match the one set in the library Global Parameters.");
				return false;
			}

			if (firTable == _firTable) {
				// Same table, do nothing
				return true;
			}

			firTable = _firTable;
			int numberOfChannels = firTable->GetNumberOfEars();
			BRTProcessing::CFIRConvolver::Setup(numberOfChannels);
			//ResetBuffers();
			return true;
		}

		std::shared_ptr<BRTServices::CGeneralFIR> GetFIRTable() const {
			return firTable;
		}

		void RemoveFIRTable() {
			firTable = nullptr;
			ResetBuffers();
		}

		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const int & _channel) override
		{			
			outBuffer = _inBuffer;
			if (firTable == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "FIR Table has not been set in BRTFilters::CFIRFilter::Process");				
				return;
			}
			
			if (enable == false) {				
				return;
			}

			BRTProcessing::CFIRConvolver::Process(_inBuffer, outBuffer, _channel, firTable);
		}

		void Process(const CMonoBuffer<float> & _inLeftBuffer, CMonoBuffer<float> & _outLeftBuffer, const CMonoBuffer<float> & _inRightBuffer, CMonoBuffer<float> & _outRightBuffer) override {
			_outLeftBuffer = _inLeftBuffer;
			_outRightBuffer = _inRightBuffer;
			if (firTable == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "FIR Table has not been set in BRTFilters::CFIRFilter::Process");
				return;
			}
			if (enable == false) {
				return;
			}

			BRTProcessing::CFIRConvolver::Process(_inLeftBuffer, _outLeftBuffer, _inRightBuffer, _outRightBuffer, firTable);
		}


		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const int & _channel, const Common::CTransform & _sourceTransform, const Common::CTransform & _listenerTransform) override
		{			
			outBuffer = _inBuffer;
			
			if (firTable == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "FIR Table has not been set in BRTFilters::CFIRFilter::Process");
				return;
			}
			if (enable == false) {
				return;
			}
																										
			BRTProcessing::CFIRConvolver::Process(_inBuffer, outBuffer, _channel, _sourceTransform, _listenerTransform, firTable);										
		}

		

		/**
		 * @brief Reset the buffers of the process
		 */
		void ResetBuffers() override{
			BRTProcessing::CFIRConvolver::ResetConvolutionBuffers();	
		}

	
	private:
		///////////////////////
		// Private Methods
		///////////////////////		
		
			
		///////////////////////
		// Private Attributes
		///////////////////////		
		Common::CGlobalParameters globalParameters;
		
		std::shared_ptr<BRTServices::CGeneralFIR> firTable; // Table of FIRs		
	};
}
#endif
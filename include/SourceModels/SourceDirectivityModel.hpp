/**
* \class CSourceDirectivityModel
*
* \brief Declaration of CSourceDirectivityModel class
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
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP
#define _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP

#include <vector>
#include <memory>
#include <Connectivity/CommandList.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <ServiceModules/ServicesBase.hpp>
#include <ProcessingModules/DirectivityConvolver.hpp>

namespace BRTSourceModel {
	class CSourceDirectivityModel : public CSourceModelBase, BRTProcessing::CDirectivityConvolver {

	public:			
		CSourceDirectivityModel(std::string _sourceID)
			: CSourceModelBase(_sourceID, TSourceType::Directivity) {			
			CreatePositionEntryPoint("listenerPosition");
		}

	
		/** \brief SET DirectivityTF of source
		*	\param[in] pointer to DirectivityTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetDirectivity(std::shared_ptr< BRTServices::CServicesBase > _sourceDirectivity) override {						
			std::lock_guard<std::mutex> lock(mutex);
			return SetDirectivityWithoutLock(_sourceDirectivity, BRTProcessing::CDirectivityConvolver::IsSourceDirectionalityEnabled());
		}

		bool SetDirectivity(std::shared_ptr<BRTServices::CServicesBase> _directivity, bool _enabled) override {
			std::lock_guard<std::mutex> lock(mutex);
			return SetDirectivityWithoutLock(_directivity, _enabled);
		}

		/**
		 * @brief Get the source directivity transfer function
		 * @return shered pointer to the directivity of the source model
		*/		
		std::shared_ptr<BRTServices::CServicesBase> GetDirectivity() override {
			std::lock_guard<std::mutex> lock(mutex);
			return sourceDirectivity;
		}
		
		/**
		 * @brief Remove the shared pointer of the directivity TF
		*/
		bool RemoveDirectivity(std::shared_ptr<BRTServices::CServicesBase> _expected = nullptr) override {
			
			std::lock_guard<std::mutex> lock(mutex);
			if (_expected && sourceDirectivity != _expected) {
				return false;
			}
			BRTProcessing::CDirectivityConvolver::DisableSourceDirectionality();
			sourceDirectivity.reset();
			BRTProcessing::CDirectivityConvolver::ResetSourceConvolutionBuffers();
			return true;
		}

		/**
		 * @brief Enable or disable directivity for the source model
		 * @param _enabled boolean true if you want to enable the directivity, false if disable
		*/				
		bool SetDirectivityEnable(bool _enabled) override {
			std::lock_guard<std::mutex> lock(mutex);
			return SetDirectivityEnableWithoutLock(_enabled);
		}

		
		/**
		 * @brief Get the status of the directivity of the source model
		 * @return true if it is enabled, false if it is disabled
		 */		
		bool IsDirectivityEnabled() override {
			std::lock_guard<std::mutex> lock(mutex);
			return IsSourceDirectionalityEnabled();
		}

	private:	
		/**
		 * @brief Set the directivity of the source model without locking the mutex. 
		 * This method is intended to be called from within a locked context.		 
		 * @param _directivity 
		 * @param _enabled 
		 * @return 
		 */
		bool SetDirectivityWithoutLock(const std::shared_ptr<BRTServices::CServicesBase> & _directivity, bool _enabled) {
			if (!_directivity || !_directivity->IsDataReady()) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Directivity resource must be non-null and ready.");
				return false;
			}

			sourceDirectivity = _directivity;
			ResetSourceConvolutionBuffers();

			if (_enabled) {
				EnableSourceDirectionality();
			} else {
				DisableSourceDirectionality();
			}

			return true;
		}
		
		/**
		 * @brief Set the directivity enable state without locking the mutex. 
		 * This method is intended to be called from within a locked context.
		 * @param _enabled 
		 * @return 
		 */
		bool SetDirectivityEnableWithoutLock(bool _enabled) {
			if (_enabled && (!sourceDirectivity || !sourceDirectivity->IsDataReady())) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED,	"Cannot enable directivity without a ready resource.");
				return false;
			}

			if (BRTProcessing::CDirectivityConvolver::IsSourceDirectionalityEnabled() == _enabled) {
				return true;
			}

			// Discard history belonging to the previous processing state.
			BRTProcessing::CDirectivityConvolver::ResetSourceConvolutionBuffers();
			if (_enabled) {
				BRTProcessing::CDirectivityConvolver::EnableSourceDirectionality();
			} else {
				BRTProcessing::CDirectivityConvolver::DisableSourceDirectionality();
			}

			return true;
		}

		/**
		 * @brief Update method of the Source directivity model
		 * @param _entryPointID ID of the entry ponint to do the update
		*/
		void ProcessInputSamples() override {
			std::lock_guard<std::mutex> l(mutex);

			CMonoBuffer<float> outBuffer;
			const CMonoBuffer<float> & inBuffer = GetBuffer();
			Common::CTransform sourcePosition = GetSourceTransform();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
			if (inBuffer.size() != 0) {
				Process(inBuffer, outBuffer, sourcePosition, listenerPosition, sourceDirectivity);
				SetOutputBuffer(outBuffer);
				PropagateBuffer();
			}
		}

		/**
		* @brief Implementation of the virtual method for processing the received commands
		* The SourceModelBase class already handles the common commands. Here you have to manage the specific ones.
		*/		
		void UpdateCommandSource(BRTConnectivity::CCommand & command) override {
			const std::string commandName = command.GetCommand();

			// Global commands apply to every source.
			if (commandName == BRTConnectivity::CCommandList::COMMAND_OVERALL_STOP) {
				ResetSourceConvolutionBuffers();
				return;
			}

			// Source-specific commands only apply to their target.
			if (!IsToMySoundSource(command.GetStringParameter(BRTConnectivity::CCommand::FIELD_CONTENT_SOURCE_ID))) {
				return;
			}

			if (commandName == BRTConnectivity::CCommandList::COMMAND_SOURCE_STOP) {
				ResetSourceConvolutionBuffers();
			
			} else if (
				commandName == BRTConnectivity::CCommandList::COMMAND_SOURCE_ENABLE_DIRECTIVITY) {
				SetDirectivityEnableWithoutLock(
					command.GetBoolParameter(BRTConnectivity::CCommand::FIELD_CONTENT_ENABLE_PARAMETER));
			}
		}

		/**
		 * @brief Reset the buffers used in the convolution of the directivity
		*/
		// TODO Move to command
		void ResetBuffers() override {
			std::lock_guard<std::mutex> lock(mutex);
			ResetSourceConvolutionBuffers();
		}

		////////////////
		// Attributes
		/////////////// 		
		std::shared_ptr<BRTServices::CServicesBase> sourceDirectivity; // Directivity of the source
		Common::CGlobalParameters globalParameters;
		
	};
}
#endif
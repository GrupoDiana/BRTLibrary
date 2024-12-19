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
#include <SourceModels/SourceModelBase.hpp>
#include <ProcessingModules/DirectivityTFConvolver.hpp>
#include <ServiceModules/DirectivityTF.hpp>

namespace BRTSourceModel {
	class CSourceDirectivityModel : public CSourceModelBase, BRTProcessing::CDirectivityTFConvolver {

	public:			
		CSourceDirectivityModel(std::string _sourceID)
			: CSourceModelBase(_sourceID, TSourceType::Directivity) {			
			CreatePositionEntryPoint("listenerPosition");
		}

	
		/** \brief SET DirectivityTF of source
		*	\param[in] pointer to DirectivityTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetDirectivityTF(std::shared_ptr< BRTServices::CDirectivityTF > _sourceDirectivityTF) override {			
			sourceDirectivityTF = _sourceDirectivityTF;						
			ResetSourceConvolutionBuffers();
			return true;
		}

		/**
		 * @brief Get the source directivity transfer function
		 * @return shered pointer to the directivity of the source model
		*/
		std::shared_ptr<BRTServices::CDirectivityTF> GetDirectivityTF() override {
			return sourceDirectivityTF;
		}

		/**
		 * @brief Remove the shared pointer of the directivity TF
		*/
		void RemoveDirectivityTF() override {
			sourceDirectivityTF = std::make_shared<BRTServices::CDirectivityTF>();			
		}

		/**
		 * @brief Enable or disable directivity for the source model
		 * @param _enabled boolean true if you want to enable the directivity, false if disable
		*/
		// TODO: Move to command
		void SetDirectivityEnable(bool _enabled) override {
			if (_enabled) { EnableSourceDirectionality(); }
			else { DisableSourceDirectionality(); }
		}
		
	private:	

		/**
		 * @brief Update method of the Source directivity model
		 * @param _entryPointID ID of the entry ponint to do the update
		*/
		void Update(std::string _entryPointID) override {
			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "samples") {

				CMonoBuffer<float> outBuffer;
				CMonoBuffer<float> inBuffer = GetBuffer();
				Common::CTransform sourcePosition = GetSourceTransform();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
				if (inBuffer.size() != 0) {
					Process(inBuffer, outBuffer, sourcePosition, listenerPosition, sourceDirectivityTF);
					SendData(outBuffer);
				}
			}
		}

		/**
		* @brief Implementation of the virtual method for processing the received commands
		* The SourceModelBase class already handles the common commands. Here you have to manage the specific ones.
		*/
		void UpdateCommandSource() override {
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/enableDirectivity") {
					if (command.GetBoolParameter("enable")) {
						EnableSourceDirectionality();
					} else {
						DisableSourceDirectionality();
					}
				} else if (command.GetCommand() == "/source/resetBuffers") {
					ResetSourceConvolutionBuffers();
				}
			}
		}

		/**
		 * @brief Reset the buffers used in the convolution of the directivity
		*/
		// TODO Move to command
		void ResetBuffers() override {
			ResetSourceConvolutionBuffers();
		}

		////////////////
		// Attributes
		/////////////// 		
		std::shared_ptr<BRTServices::CDirectivityTF> sourceDirectivityTF;			// Directivity of the source
		Common::CGlobalParameters globalParameters;
		
	};
}
#endif
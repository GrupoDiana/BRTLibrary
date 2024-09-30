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
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP
#define _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP

#include <vector>
#include <Base/SourceModelBase.hpp>
#include <ProcessingModules/DirectivityTFConvolver.hpp>
#include <ServiceModules/DirectivityTF.hpp>

namespace BRTSourceModel {
	class CSourceDirectivityModel : public BRTBase::CSourceModelBase, BRTProcessing::CDirectivityTFConvolver {

	public:			
		CSourceDirectivityModel(std::string _sourceID) : BRTBase::CSourceModelBase(_sourceID) {
			SetSourceType(TSourceType::Directivity);
			CreatePositionEntryPoint("listenerPosition");
		}

		/**
		 * @brief Update method of the Source directivity model
		 * @param _entryPointID ID of the entry ponint to do the update
		*/
		void Update(std::string _entryPointID) {
			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "samples") {

				CMonoBuffer<float> outBuffer;
				CMonoBuffer<float> inBuffer = GetBuffer();
				Common::CTransform sourcePosition = GetCurrentSourceTransform();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
				if (inBuffer.size() != 0) {
					Process(inBuffer, outBuffer, sourcePosition, listenerPosition, sourceDirectivityTF);
					SendData(outBuffer);
				}
			}			
		}
		
		/**
		 * @brief Update to check to internal OSC commands
		*/
		void UpdateCommand() {
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
			
			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/location") {
					Common::CVector3 location = command.GetVector3Parameter("location");										
					Common::CTransform sourceTransform = GetCurrentSourceTransform();
					sourceTransform.SetPosition(location);
					SetSourceTransform(sourceTransform);
				}
				else if (command.GetCommand() == "/source/orientation") {
					Common::CVector3 orientationYawPitchRoll = command.GetVector3Parameter("orientation");										
					Common::CQuaternion orientation;
					orientation = orientation.FromYawPitchRoll(orientationYawPitchRoll.x, orientationYawPitchRoll.y, orientationYawPitchRoll.z);
					
					Common::CTransform sourceTransform = GetCurrentSourceTransform();
					sourceTransform.SetOrientation(orientation);
					SetSourceTransform(sourceTransform);
				}
				else if (command.GetCommand() == "/source/orientationQuaternion") {
					Common::CQuaternion orientation = command.GetQuaternionParameter("orientation");
					Common::CTransform sourceTransform = GetCurrentSourceTransform();
					sourceTransform.SetOrientation(orientation);
					SetSourceTransform(sourceTransform);
				}
				else if(command.GetCommand() == "/source/enableDirectivity") {
					if (command.GetBoolParameter("enable")) { EnableSourceDirectionality(); }
					else { DisableSourceDirectionality(); }
				} else if (command.GetCommand() == "/source/resetBuffers") {
					ResetSourceConvolutionBuffers();
				}
			}
		}

		/** \brief SET DirectivityTF of source
		*	\param[in] pointer to DirectivityTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetDirectivityTF(std::shared_ptr< BRTServices::CDirectivityTF > _sourceDirectivityTF) {			
			sourceDirectivityTF = _sourceDirectivityTF;						
			ResetSourceConvolutionBuffers();
			return true;
		}

		/**
		 * @brief Get the source directivity transfer function
		 * @return shered pointer to the directivity of the source model
		*/
		std::shared_ptr< BRTServices::CDirectivityTF > GetDirectivityTF() {
			return sourceDirectivityTF;
		}

		/**
		 * @brief Remove the shared pointer of the directivity TF
		*/
		void RemoveDirectivityTF() {
			sourceDirectivityTF = std::make_shared<BRTServices::CDirectivityTF>();			
		}

		/**
		 * @brief Enable or disable directivity for the source model
		 * @param _enabled boolean true if you want to enable the directivity, false if disable
		*/
		// TODO: Move to command
		void SetDirectivityEnable(bool _enabled) {
			if (_enabled) { EnableSourceDirectionality(); }
			else { DisableSourceDirectionality(); }
		}
		
		/**
		 * @brief Reset the buffers used in the convolution of the directivity
		*/
		// TODO Move to command
		void ResetBuffers() {
			ResetSourceConvolutionBuffers();
		}

	private:		
		mutable std::mutex mutex;
		std::shared_ptr<BRTServices::CDirectivityTF> sourceDirectivityTF;			// Directivity of the source
		Common::CGlobalParameters globalParameters;
		
	};
}
#endif
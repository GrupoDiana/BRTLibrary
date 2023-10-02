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
#include <ProcessingModules/SRTFConvolver.hpp>
#include <ServiceModules/SRTF.hpp>

namespace BRTSourceModel {
	class CSourceDirectivityModel : public BRTBase::CSourceModelBase, BRTProcessing::CSRTFConvolver {

	public:			
		CSourceDirectivityModel(std::string _sourceID) : BRTBase::CSourceModelBase(_sourceID) {
			//CreateListenerTransformEntryPoint("listenerPosition");
			CreatePositionEntryPoint("listenerPosition");
		}

		void Update(std::string _entryPointID) {
			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "samples") {

				CMonoBuffer<float> outBuffer;
				CMonoBuffer<float> inBuffer = GetBuffer();
				Common::CTransform sourcePosition = GetCurrentSourceTransform();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
				if (inBuffer.size() != 0) {
					Process(inBuffer, outBuffer, sourcePosition, listenerPosition, sourceSRTF);
					SendData(outBuffer);
				}
			}			
		}
				
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

		/** \brief SET SRTF of source
		*	\param[in] pointer to SRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetSRTF(std::shared_ptr< BRTServices::CSRTF > _sourceSRTF) {			
			if (_sourceSRTF->GetSamplingRate() != globalParameters.GetSampleRate()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This SRTF has not been assigned to the source. The sample rate of the SRTF does not match the one set in the library Global Parameters.");
				return false;
			}			
			sourceSRTF = _sourceSRTF;						
			ResetSourceConvolutionBuffers();
		}

		std::shared_ptr< BRTServices::CSRTF > GetSRFT() {
			return sourceSRTF;
		}

		void RemoveSRTF() {
			sourceSRTF = std::make_shared<BRTServices::CSRTF>();	// empty HRTF		
		}

		// TODO Move to command
		void SetDirectivityEnable(bool _enabled) {
			if (_enabled) { EnableSourceDirectionality(); }
			else { DisableSourceDirectionality(); }
		}
		// TODO Move to command
		void ResetBuffers() {
			ResetSourceConvolutionBuffers();
		}

	private:		
		mutable std::mutex mutex;
		std::shared_ptr<BRTServices::CSRTF> sourceSRTF;			// SHRTF of source
		Common::CGlobalParameters globalParameters;
		
	};
}
#endif
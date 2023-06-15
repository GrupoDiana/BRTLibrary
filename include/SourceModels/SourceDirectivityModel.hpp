#ifndef _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP
#define _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP

//#include "ExitPoint.hpp"
//#include "EntryPoint.hpp"
//#include <Base/EntryPointManager.hpp>
#include "SourceModelBase.hpp"
#include <vector>
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
		void SetSRTF(std::shared_ptr< BRTServices::CSRTF > _sourceSRTF) {			
			sourceSRTF = _sourceSRTF;			
		}

		std::shared_ptr< BRTServices::CSRTF > GetSRFT() {
			return sourceSRTF;
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

		//// METHODS
		//bool IsToMySoundSource(std::string _sourceID) {			
		//	return GetSourceID() == _sourceID;
		//}
	};
}
#endif
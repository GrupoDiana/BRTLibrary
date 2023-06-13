#ifndef _SOUND_SOURCE_BASIC_MODEL_HPP
#define _SOUND_SOURCE_BASIC_MODEL_HPP

#include "ExitPoint.hpp"
#include "SourceModelBase.hpp"
#include <vector>

namespace BRTSourceModel {
	class CSourceSimpleModel : public BRTBase::CSourceModelBase {

	public:			
		CSourceSimpleModel(std::string _sourceID) : BRTBase::CSourceModelBase(_sourceID) {			
		}

		void Update(std::string _entryPointID) {
			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "samples") {
				CMonoBuffer<float> buffer = GetBuffer();
				SendData(buffer);
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
			}
		}

	private:		
		mutable std::mutex mutex;


	};
}
#endif
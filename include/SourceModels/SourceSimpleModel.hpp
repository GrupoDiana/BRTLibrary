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
				

	private:		
		mutable std::mutex mutex;

	};
}
#endif
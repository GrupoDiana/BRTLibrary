#ifndef _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP
#define _SOUND_SOURCE_DIRECTIVITY_MODEL_HPP

#include "ExitPoint.hpp"
#include "EntryPoint.hpp"
#include "SourceModelBase.hpp"
#include <vector>
#include <ProcessingModules/SRTFConvolver.hpp>
#include <ServiceModules/SRTF.hpp>

namespace BRTSourceModel {
	class CSourceDirectivityModel : public BRTBase::CSourceModelBase, BRTProcessing::CSRTFConvolver {

	public:			
		CSourceDirectivityModel(std::string _sourceID) : BRTBase::CSourceModelBase(_sourceID) {
			CreateListenerTransformEntryPoint("listenerPosition");
		}

		void Update(std::string _entryPointID) {
			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "samples") {

				CMonoBuffer<float> outBuffer;
				CMonoBuffer<float> inBuffer = GetBuffer();
				Common::CTransform sourcePosition = GetCurrentSourceTransform();
				Common::CTransform listenerPosition = GetListenerTransformEntryPoint()->GetData();
				if (inBuffer.size() != 0) {
					Process(inBuffer, outBuffer, sourcePosition, listenerPosition, sourceSRTF);
					SendData(outBuffer);
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

	private:		
		mutable std::mutex mutex;

		std::shared_ptr<BRTServices::CSRTF> sourceSRTF;			// SHRTF of source
	};
}
#endif
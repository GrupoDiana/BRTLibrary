#ifndef _SOUND_SOURCE_MODEL_BASE_HPP
#define _SOUND_SOURCE_MODEL_BASE_HPP


#include <Base/EntryPointManager.hpp>
#include <Base/CommandEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>

#include <vector>

namespace BRTBase {

	class CSourceModelBase : public CCommandEntryPointManager, public CExitPointManager, public CEntryPointManager {
	public:		
		virtual ~CSourceModelBase() {}						
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommand() = 0;

		CSourceModelBase(std::string _sourceID) : dataReady{ false }, sourceID{ _sourceID} {
			
			CreateSamplesExitPoint("samples");
			CreateTransformExitPoint();
			
			CreateIDExitPoint();
			GetIDExitPoint()->sendData(sourceID);

			CreateCommandEntryPoint();
		}

		void SetBuffer(CMonoBuffer<float>& _buffer) { 
			samplesBuffer = _buffer; 
			dataReady = true;
		}

		CMonoBuffer<float> GetBuffer() {
			return samplesBuffer;
			
		}
		void SetDataReady() {
			if (!dataReady) { return; }
			//samplesExitPoint->sendData(samplesBuffer); 
			Update(GetSamplesExitPoint("samples")->GetID());
			//dataReady = false;
		}

		void operator()() {
			//samplesExitPoint->sendData(samplesBuffer);
			//Update();
			//dataReady = false;
			Update(GetSamplesExitPoint("samples")->GetID());
		}

		void SendData(CMonoBuffer<float>& _buffer) {
			GetSamplesExitPoint("samples")->sendData(_buffer);
			dataReady = false;
		}

		void SetSourceTransform(Common::CTransform _transform) { 
			sourceTransform = _transform;
			//sourcePositionExitPoint->sendData(sourceTransform);
			GetTransformExitPoint()->sendData(sourceTransform);
		}
		
		const Common::CTransform& GetCurrentSourceTransform() const { return sourceTransform; };		
		
		std::string GetID() { return sourceID; }

		// Update callback
		void updateFromEntryPoint(std::string entryPointID) {
			Update(entryPointID);
		}
		void updateFromCommandEntryPoint(std::string entryPointID) {			   
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}

		
		/*void CreatePositionEntryPoint(std::string entryPointID) {
			listenerPositionEntryPoint = std::make_shared<BRTBase::CEntryPointTransform >(std::bind(&CSourceModelBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, 1);
		}
		

		std::shared_ptr<BRTBase::CEntryPointTransform > GetPositionEntryPoint(std::string entryPointID) {
			return listenerPositionEntryPoint;
		}*/
		
		bool IsToMySoundSource(std::string _sourceID) {
			return GetID() == _sourceID;
		}

	private:
		std::string sourceID;
		bool dataReady;

		Common::CTransform sourceTransform;
		CMonoBuffer<float> samplesBuffer;
						

		//std::shared_ptr<CExitPointSamplesVector> samplesExitPoint;
		/*std::shared_ptr<CExitPointTransform> sourcePositionExitPoint;*/
		//std::shared_ptr<CExitPointID> sourceIDExitPoint;
		
		//std::shared_ptr<CEntryPointTransform> listenerPositionEntryPoint;		
	};
}
#endif
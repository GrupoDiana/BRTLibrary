#ifndef _SOUND_SOURCE_MODEL_BASE_HPP
#define _SOUND_SOURCE_MODEL_BASE_HPP

#include "ExitPoint.hpp"
//#include "EntryPoint.hpp"
#include <Base/CommandEntryPointManager.hpp>
#include <vector>

namespace BRTBase {

	class CSourceModelBase: public  CCommandEntryPointManager {
	public:		
		virtual ~CSourceModelBase() {}						
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommand() = 0;

		CSourceModelBase(std::string _sourceID) : dataReady{ false }, sourceID{ _sourceID} {
			
			CreateSamplesExitPoint("samples");
			CreateTransformExitPoint("sourceTransform");
			CreateIDExitPoint("sourceID");
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
			Update(samplesExitPoint->GetID());
			//dataReady = false;
		}

		void operator()() {
			//samplesExitPoint->sendData(samplesBuffer);
			//Update();
			//dataReady = false;
			Update(samplesExitPoint->GetID());
		}

		void SendData(CMonoBuffer<float>& _buffer) {
			samplesExitPoint->sendData(_buffer);
			dataReady = false;
		}

		void SetSourceTransform(Common::CTransform _transform) { 
			sourceTransform = _transform;
			sourcePositionExitPoint->sendData(sourceTransform);			
		}
		
		const Common::CTransform& GetCurrentSourceTransform() const { return sourceTransform; };		
		
		std::string GetSourceID() { return sourceID; }

		std::shared_ptr<BRTBase::CExitPointSamplesVector> GetSamplesExitPoint(std::string exitPointID) {
			return samplesExitPoint;						
		}
	
		std::shared_ptr<BRTBase::CExitPointTransform> GetTransformExitPoint() {
			return sourcePositionExitPoint;			
		}

		std::shared_ptr<BRTBase::CExitPointID> GetIDExitPoint() {
			return sourceIDExitPoint;
		}


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




		void CreateSamplesExitPoint(std::string exitPointID) {
			samplesExitPoint = std::make_shared<CExitPointSamplesVector>(exitPointID);
		}
		void CreateTransformExitPoint(std::string exitPointID) {
			sourcePositionExitPoint = std::make_shared<CExitPointTransform>(exitPointID);
		}
		void CreateIDExitPoint(std::string exitPointID) {
			sourceIDExitPoint = std::make_shared<CExitPointID>(exitPointID);
			sourceIDExitPoint->sendData(sourceID);
		}
		void CreateListenerTransformEntryPoint(std::string entryPointID) {			
			listenerPositionEntryPoint = std::make_shared<BRTBase::CEntryPointTransform >(std::bind(&CSourceModelBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, 1);
		}
		

		std::shared_ptr<BRTBase::CEntryPointTransform >  GetListenerTransformEntryPoint() {			
			return listenerPositionEntryPoint;
		}

	private:
		std::string sourceID;
		bool dataReady;

		Common::CTransform sourceTransform;
		CMonoBuffer<float> samplesBuffer;
						

		std::shared_ptr<CExitPointSamplesVector> samplesExitPoint;
		std::shared_ptr<CExitPointTransform> sourcePositionExitPoint;
		std::shared_ptr<CExitPointID> sourceIDExitPoint;
		
		std::shared_ptr<CEntryPointTransform> listenerPositionEntryPoint;		
	};
}
#endif
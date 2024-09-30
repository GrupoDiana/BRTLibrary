/**
* \class CSourceModelBase
*
* \brief Declaration of CSourceModelBase class
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

		enum TSourceType {Simple, Directivity, Virtual};
		
		CSourceModelBase(std::string _sourceID)
			: dataReady { false }
			, sourceID { _sourceID }
			, sourceType {Simple} {
			
			CreateSamplesExitPoint("samples");
			CreateTransformExitPoint();
			
			CreateIDExitPoint();
			GetIDExitPoint()->sendData(sourceID);

			CreateCommandEntryPoint();
		}

		void SetBuffer(const CMonoBuffer<float>& _buffer) { 
			samplesBuffer = _buffer; 
			dataReady = true;
		}

		CMonoBuffer<float> GetBuffer() {
			return samplesBuffer;
			
		}
		void SetDataReady() {
			if (!dataReady) { 				
				SetBuffer(CMonoBuffer<float>(globalParameters.GetBufferSize()));			// set and empty buffer to continue
			}			
			//Update(GetSamplesExitPoint("samples")->GetID());
			Update("samples");
		}

		void operator()() {
			//samplesExitPoint->sendData(samplesBuffer);
			//Update();
			//dataReady = false;
			//Update(GetSamplesExitPoint("samples")->GetID());			
			SetDataReady();
		}

		void SendData(CMonoBuffer<float>& _buffer) {
			GetSamplesExitPoint("samples")->sendData(_buffer);
			dataReady = false;
		}

		void SetSourceTransform(Common::CTransform _transform) { 
			sourceTransform = _transform;			
			GetTransformExitPoint()->sendData(sourceTransform);
		}
		
		void SetSourceType(TSourceType _sourceType) {
			sourceType = _sourceType;
		}

		TSourceType GetSourceType() {
			return sourceType;
		}

		const Common::CTransform& GetCurrentSourceTransform() const { return sourceTransform; };		
		
		std::string GetID() { return sourceID; }

		// Update callback
		void UpdateEntryPointData(std::string entryPointID) {
			Update(entryPointID);
		}
		void updateFromCommandEntryPoint(std::string entryPointID) {			   
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}
						
		bool IsToMySoundSource(std::string _sourceID) {
			return GetID() == _sourceID;
		}

	private:
		std::string sourceID;		
		TSourceType sourceType;

		bool dataReady;
		Common::CTransform sourceTransform;
		CMonoBuffer<float> samplesBuffer;			
		Common::CGlobalParameters globalParameters;
	};
}
#endif

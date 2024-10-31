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


//#include <Base/EntryPointManager.hpp>
//#include <Base/CommandEntryPointManager.hpp>
//#include <Base/ExitPointManager.hpp>
#include <vector>
#include <Base/BRTConnectivity.hpp>

namespace BRTBase {

	class CSourceModelBase : public CBRTConnectivity /*public CCommandEntryPointManager, public CExitPointManager, public CEntryPointManager*/ {
	public:		
		virtual ~CSourceModelBase() {}						
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommandSource() = 0;

		enum TSourceType {Simple, Directivity, Virtual};
		
		CSourceModelBase(std::string _sourceID, TSourceType _sourceType)
			: dataReady { false }
			, sourceID { _sourceID }
			, sourceType { _sourceType } {
			
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




		
		/**
		* @brief Manages the reception of new data by an entry point. 
		* Only entry points that have a notification make a call to this method.
		*/
		void UpdateEntryPointData(std::string entryPointID) override {
			Update(entryPointID);
		}


		void UpdateCommand() override {
			
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/location") {
					Common::CVector3 location = command.GetVector3Parameter("location");
					Common::CTransform sourceTransform = GetCurrentSourceTransform();
					sourceTransform.SetPosition(location);
					SetSourceTransform(sourceTransform);
				} else if (command.GetCommand() == "/source/orientation") {
					Common::CVector3 orientationYawPitchRoll = command.GetVector3Parameter("orientation");
					Common::CQuaternion orientation;
					orientation = orientation.FromYawPitchRoll(orientationYawPitchRoll.x, orientationYawPitchRoll.y, orientationYawPitchRoll.z);

					Common::CTransform sourceTransform = GetCurrentSourceTransform();
					sourceTransform.SetOrientation(orientation);
					SetSourceTransform(sourceTransform);
				} else if (command.GetCommand() == "/source/orientationQuaternion") {
					Common::CQuaternion orientation = command.GetQuaternionParameter("orientation");
					Common::CTransform sourceTransform = GetCurrentSourceTransform();
					sourceTransform.SetOrientation(orientation);
					SetSourceTransform(sourceTransform);
				}
			}
			
			UpdateCommandSource();
		}

		/*void updateFromCommandEntryPoint(std::string entryPointID) {			   
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}*/
						
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

	protected:
		mutable std::mutex mutex;		// To avoid access collisions
	};
}
#endif

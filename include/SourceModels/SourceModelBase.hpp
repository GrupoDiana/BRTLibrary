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
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _SOUND_SOURCE_MODEL_BASE_HPP
#define _SOUND_SOURCE_MODEL_BASE_HPP


#include <vector>
#include <Connectivity/BRTConnectivity.hpp>

namespace BRTBase {
	class CBRTManager; // Forward declaration
}
namespace BRTSourceModel {

	enum TSourceType { Simple,	Directivity, Virtual };
	
	class CSourceModelBase : public BRTConnectivity::CBRTConnectivity {				
	public:		
		virtual ~CSourceModelBase() {}						
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommandSource() = 0;

		virtual bool SetDirectivityTF(std::shared_ptr<BRTServices::CDirectivityTF> _sourceDirectivityTF) { return false; }
		virtual std::shared_ptr<BRTServices::CDirectivityTF> GetDirectivityTF() { return nullptr; }
		virtual void RemoveDirectivityTF() {};
		virtual void SetDirectivityEnable(bool _enabled) {};
		virtual void ResetBuffers() {};

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
		
		
		/**
		 * @brief Set audio frame buffers
		 * @param _buffer samples buffer
		 */
		void SetBuffer(const CMonoBuffer<float>& _buffer) { 
			samplesBuffer = _buffer; 
			dataReady = true;
		}

		/**
		 * @brief Get the last audio frame buffer
		 * @return last samples buffer
		 */
		CMonoBuffer<float> GetBuffer() {
			return samplesBuffer;
			
		}
		
		/**
		 * @brief Set the source transform
		 * @param _transform Source transform
		 */
		void SetSourceTransform(Common::CTransform _transform) { 
			sourceTransform = _transform;			
			GetTransformExitPoint()->sendData(sourceTransform);
		}
						
		/**
		 * @brief Get the current source transform
		 * @return Source transform
		 */
		const Common::CTransform& GetSourceTransform() const { return sourceTransform; };		
		
		/**
		 * @brief Get the source ID
		 * @return Source ID
		 */ 
		std::string GetID() { return sourceID; }
		
		/**
		 * @brief Get wich type of source is
		 * @return Source type
		 */
		TSourceType GetSourceType() {
			return sourceType;
		}

		
		
	private:	

		//////////////
		// Methods
		//////////////

		/**
		 * @brief Set the data ready flag. Internal use only.
		 */
		void SetDataReady() {
			if (!dataReady) {
				SetBuffer(CMonoBuffer<float>(globalParameters.GetBufferSize())); // set and empty buffer to continue
			}
			Update("samples");
		}

		/**
		 * @brief Set the data ready flag. Internal use only.		 
		 */
		void operator()() {
			SetDataReady();
		}

		/**
		* @brief Manages the reception of new data by an entry point. 
		* Only entry points that have a notification make a call to this method.
		*/
		void UpdateEntryPointData(std::string entryPointID) override {
			Update(entryPointID);
		}

		/**
		 * @brief Manages the reception of new command by an entry point.
		 */
		void UpdateCommand() override {

			std::lock_guard<std::mutex> l(mutex);
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/location") {
					Common::CVector3 location = command.GetVector3Parameter("location");
					Common::CTransform sourceTransform = GetSourceTransform();
					sourceTransform.SetPosition(location);
					SetSourceTransform(sourceTransform);
				} else if (command.GetCommand() == "/source/orientation") {
					Common::CVector3 orientationYawPitchRoll = command.GetVector3Parameter("orientation");
					Common::CQuaternion orientation;
					orientation = orientation.FromYawPitchRoll(orientationYawPitchRoll.x, orientationYawPitchRoll.y, orientationYawPitchRoll.z);

					Common::CTransform sourceTransform = GetSourceTransform();
					sourceTransform.SetOrientation(orientation);
					SetSourceTransform(sourceTransform);
				} else if (command.GetCommand() == "/source/orientationQuaternion") {
					Common::CQuaternion orientation = command.GetQuaternionParameter("orientation");
					Common::CTransform sourceTransform = GetSourceTransform();
					sourceTransform.SetOrientation(orientation);
					SetSourceTransform(sourceTransform);
				}
			}

			UpdateCommandSource();
		}
				
		////////////////
		// Attributes
		////////////////
		std::string sourceID;		
		TSourceType sourceType;

		bool dataReady;
		Common::CTransform sourceTransform;
		CMonoBuffer<float> samplesBuffer;			
		Common::CGlobalParameters globalParameters;

		friend class BRTBase::CBRTManager; // Declare CBRTManager

	protected:
		
		/**
		 * @brief Send the data to the exit point
		 * @param _buffer Buffer to be sent
		 */
		void SendData(CMonoBuffer<float> & _buffer) {
			GetSamplesExitPoint("samples")->sendData(_buffer);
			dataReady = false;
		}
		
		/**
		 * @brief Set the source type
		 * @param _sourceType 
		 */
		void SetSourceType(TSourceType _sourceType) {
			sourceType = _sourceType;
		}

		/**
		 * @brief Check if the command is for this source
		 * @param _sourceID Source ID
		 * @return True if the command is for this source
		 */
		bool IsToMySoundSource(std::string _sourceID) {
			return GetID() == _sourceID;
		}

		mutable std::mutex mutex;		// To avoid access collisions
	};
}
#endif

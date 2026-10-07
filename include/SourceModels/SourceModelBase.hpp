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
#include <mutex>
#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Connectivity/BRTConnectivity.hpp>

namespace BRTBase {
	class CBRTManager; // Forward declaration
}
namespace BRTSourceModel {
	
	enum TSourceType { Omnidirectional, Directivity };	
	
	class CSourceModelBase : public BRTConnectivity::CBRTConnectivity {				
			
		enum TState { Waiting, inputBufferReceived, Processing, Processed };

	public:		
		virtual ~CSourceModelBase() {}						
		virtual void ProcessInputSamples() = 0;
		virtual void UpdateCommandSource() = 0;

		virtual bool SetDirectivity(std::shared_ptr<BRTServices::CServicesBase> _sourceDirectivity) { return false; }
		virtual std::shared_ptr<BRTServices::CServicesBase> GetDirectivity() { return nullptr; }
		virtual void RemoveDirectivity() {};

		virtual void SetDirectivityEnable(bool _enabled) {};
		virtual bool IsDirectivityEnabled() { return false; }

		virtual void ResetBuffers() {};

		CSourceModelBase(std::string _sourceID, TSourceType _sourceType)
			: state { TState::Waiting }			
			, sourceID { _sourceID }
			, sourceType { _sourceType }
			, virtualSource { false } {
			
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
			inputBuffer = _buffer; 
			//inputBufferReceived = true;
			state = TState::inputBufferReceived;
		}

		/**
		 * @brief Get the last audio frame buffer
		 * @return last samples buffer
		 */
		CMonoBuffer<float> & GetBuffer() {
			return inputBuffer;			
		}
		
		/**
		 * @brief Set the source transform
		 * @param _transform Source transform
		 */
		void SetSourceTransform(const Common::CTransform & _transform) { 
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
		std::string GetID() { 
			return sourceID; 
		}
		
		/**
		 * @brief Get wich type of source is
		 * @return Source type
		 */
		TSourceType GetSourceType() {
			return sourceType;
		}
		
		bool IsVirtualSource() {
			return virtualSource;
		}
				
	private:	

		//////////////
		// Methods
		//////////////

		/**
		 * @brief Set the data ready flag. Internal use only.
		 */
		void PropagateSamples() {
			if (state == TState::Waiting) {
				// Set an empty buffer to continue
				SetBuffer(CMonoBuffer<float>(globalParameters.GetBufferSize()));				
			}
			state = TState::Processing;
			ProcessInputSamples();
		}

		/**
		 * @brief Set the data ready flag. Internal use only.		 
		 */
		void operator()() {
			PropagateSamples();
		}

		/**
		* @brief Manages the reception of new data by an entry point. 
		* Only entry points that have a notification make a call to this method.
		*/
		void UpdateEntryPointData(std::string entryPointID) override {
			if (entryPointID == "samples")
				ProcessInputSamples();
		}

		/**
		 * @brief Manages the reception of new command by an entry point.
		 */
		void UpdateCommand() override {

			std::lock_guard<std::mutex> l(mutex);
			//BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			BRTConnectivity::CCommand command = GetLastReceivedCommand();

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/location") {
					Common::CVector3 location = command.GetVector3Parameter("location");
					Common::CTransform sourceTransform = GetSourceTransform();
					sourceTransform.SetPosition(location);
					SetSourceTransform(sourceTransform);
				} else if (command.GetCommand() == "/source/TOrientation") {
					Common::CVector3 orientationYawPitchRoll = command.GetVector3Parameter("TOrientation");
					Common::CQuaternion TOrientation;
					TOrientation = TOrientation.FromYawPitchRoll(orientationYawPitchRoll.x, orientationYawPitchRoll.y, orientationYawPitchRoll.z);

					Common::CTransform sourceTransform = GetSourceTransform();
					sourceTransform.SetOrientation(TOrientation);
					SetSourceTransform(sourceTransform);
				} else if (command.GetCommand() == "/source/orientationQuaternion") {
					Common::CQuaternion TOrientation = command.GetQuaternionParameter("TOrientation");
					Common::CTransform sourceTransform = GetSourceTransform();
					sourceTransform.SetOrientation(TOrientation);
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
		bool virtualSource;

		TState state;
		//bool inputBufferReceived;
		CMonoBuffer<float> inputBuffer;		
		CMonoBuffer<float> outputBuffer;	
		Common::CTransform sourceTransform;		
		Common::CGlobalParameters globalParameters;

		friend class BRTBase::CBRTManager; // Declare CBRTManager

	protected:
		
		void SetOutputBuffer(CMonoBuffer<float> & _buffer) {
			outputBuffer = _buffer;
			state = TState::Processed;
		}
		/**
		 * @brief Send the data to the exit point
		 * @param _buffer Buffer to be sent
		 */
		void PropagateBuffer() {
			if (state != TState::Processed) {			
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Trying to propagate buffer before processing it.");
				return;
			}
			GetSamplesExitPoint("samples")->sendData(outputBuffer);
			state = TState::Waiting;
		}

		void PropageteInputBuffer() {
			if (state != TState::Processing) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Trying to propagate input buffer before receiving it.");
				return;
			}
			state = TState::Processed;
			GetSamplesExitPoint("samples")->sendData(inputBuffer);
			state = TState::Waiting;
		}

		///**
		// * @brief Send the data to the exit point
		// * @param _buffer Buffer to be sent
		// */
		//void PropagateBuffer(CMonoBuffer<float> & _buffer) {
		//	GetSamplesExitPoint("samples")->sendData(_buffer);
		//	inputBufferReceived = false;
		//}
		
		/**
		 * @brief Set the source type
		 * @param _sourceType 
		 */
		void SetAsVirtualSource() {
			virtualSource = true;
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

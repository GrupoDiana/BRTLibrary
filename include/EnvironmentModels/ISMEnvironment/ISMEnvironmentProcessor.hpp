/**
* \class CISMEnvironmentModel
*
* \brief This class implements the ISM proccesor.
* \date	Oct 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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

#ifndef _ISM_ENVIRONMENT_PROCESSOR_HPP_
#define _ISM_ENVIRONMENT_PROCESSOR_HPP_

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Base/BRTManager.hpp>
#include <Connectivity/BRTConnectivity.hpp>
#include <EnvironmentModels/VirtualSourceList.hpp>
#include <EnvironmentModels/ISMEnvironment/ISMEnvironment.hpp>


namespace BRTEnvironmentModel {

	class CISMEnvironmentProcessor : public BRTConnectivity::CBRTConnectivity, public CVirtualSourceList, CISMEnvironment {
	public:

		CISMEnvironmentProcessor(BRTBase::CBRTManager * _brtManager)
			: CVirtualSourceList(_brtManager)
			, brtManager { _brtManager }
			, initialized { false }
			, setupDone { false }
			, enableProcessor { true }
			, virtualSourcesConnectedToListener { false }
			, originalSourceID { "" }
			//, muteLoS { false }
			, muteReverbPath { false }		
			, gain {1.0f}
			, numberOfImageSources { 0 }
		{

			CreateSamplesEntryPoint("inputSamples");
			CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");       
			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");						
		}

		~CISMEnvironmentProcessor() { };
				
		/**
		 * @brief Setup the environment processor
		 * @param _orinalSourceID ID of the original source
		 * @return True if the setup was successful
		 */
		bool Init(std::string & _orinalSourceID)
		{
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (initialized) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The ISM environment processor is already initialized");
				return false;
			}

			if (_orinalSourceID == "" ) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The source ID must be defined");
				return false;
			}
							
			originalSourceID = _orinalSourceID;
						
			//globalCoordinatesRoomCentre = Common::CVector3::ZERO();
			initialized = true;
			return true;
		}
		
		/**
		 * @brief Setup ISM simulation parameters
		 * @param _reflectionOrder order of reflections
		 * @param _maxDistanceSourcesToListener maximum distance from sources to listener, this is used to prune distant image sources.
		 * @param _windowSlopeDistance Transition distance for the windowing function.
		 * @param _room Room definition
		 * @param _listenerModel listener model to connect the virtual sources to
		 * @return true if the setup was successful
		 */
		bool Setup(const int & _reflectionOrder, const float & _maxDistanceSourcesToListener, const float & _windowSlopeDistance, std::shared_ptr<Common::CRoom> & _room, std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (!initialized) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The ISM environment processor is not initialized");
				return false;
			}
			if (setupDone) {				
				setupDone = false;
				ResetVirtualSources(_listenerModel);
			}			
			InitISMEnvironment(_reflectionOrder, _maxDistanceSourcesToListener, _windowSlopeDistance, _room);			
			//TODO check result before setting setupDone to true
			setupDone = true;
			return true;
		}
		

		/**
		 * @brief Enable processor
		 */
		void EnableProcessor() { enableProcessor = true; }
		/**
		 * @brief Disable processor
		 */
		void DisableProcessor() { enableProcessor = false; }
		/**
		 * @brief Get the flag to know if the processor is enabled.
		 * @return true if the processor is enabled, false otherwise
		 */
		bool IsProcessorEnabled() { return enableProcessor; }

		/**
		 * @brief Set the gain of the environment processor
		 * @param _gain New gain value
		 */
		void SetGain(float _gain) {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			gain = _gain;
		}

		/**
		 * @brief Connect the environment processor to a listener model
		 * @param _listenerModel Listener model to connect
		 * @return True if the connection was successful
		 */
		bool ConnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {			
			bool result = ConnectVirtualSourcesToListenerModel<BRTListenerModel::CListenerModelBase>(_listenerModel);
			if (result) {
				virtualSourcesConnectedToListener = true;
			} else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There was an error connecting the virtual sources to the listener model");
			}
			return result;
		}

		/**
		 * @brief Disconnect the environment processor from a listener model
		 * @param _listenerModel Listener model to disconnect
		 * @return True if the disconnection was successful
		 */
		bool DisconnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {			
			bool result=  DisconnectVirtualSourcesToListenerModel<BRTListenerModel::CListenerModelBase>(_listenerModel);
			if (result) {
				virtualSourcesConnectedToListener = false;
			} else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There was an error disconnecting the virtual sources from the listener model");
			}
			return result;
		}

		/**
		* @brief Mute or unmute line of sight component
		* @param mute True to mute the line of sight component, False to unmute
		*/
		//void MuteLOS(bool mute)
		//{
		//	muteLoS = mute;
		//}
		//
		///**
		// * @brief Return the mute status of the line of sight component
		// * @return True if the line of sight component is muted, False otherwise
		// */
		//bool GetMuteLOS() {
		//	return muteLoS;
		//}

		/**
		* @brief Mute or unmute reverb path component
		* @param mute True to mute the reverb path, False to unmute
		*/
		void MuteReverbPath(bool mute) {
			muteReverbPath = mute;
		}

		/**
		 * @brief Return the mute status of the reverb path component
		 * @return True if the reverb path component is muted, False otherwise
		 */
		bool GetMuteReverbPath() {
			return muteReverbPath;
		}				
		
		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {			
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (!initialized) {	
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "The SDN environment processor is not initialized");
				return;
			}					
			if (!setupDone) { 	
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The ISM environment processor is not setup");
				return;
			}
			if (!virtualSourcesConnectedToListener) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The virtual sources are not connected to a listener model");
				return;
			}

			// Get data from entry points
			CMonoBuffer<float> inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();
			Common::CTransform sourceLocation = GetPositionEntryPoint("sourcePosition")->GetData();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
						
			if (inBuffer.size() == 0) {				
				SET_RESULT(RESULT_ERROR_BADSIZE, "The input buffer size is 0");
				return;
			}

			ASSERT(inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");

			if (!enableProcessor) {
				virtualSourceBuffers = std::vector<CMonoBuffer<float>>(numberOfImageSources, CMonoBuffer<float>(globalParameters.GetBufferSize()));								
				virtualSourcePositions = std::vector<Common::CTransform>(numberOfImageSources, Common::CTransform());				
				SyncAllVirtualSourcesToModel();
				return;
			}	
						
			// If the source or listener position exceed the size of the room silence the output			
			if (IsInBounds(sourceLocation.GetPosition()) && IsInBounds(listenerPosition.GetPosition())) {				
				CISMEnvironment::Process(inBuffer, sourceLocation, listenerPosition, virtualSourceBuffers, virtualSourcePositions);				
			} else {		
				virtualSourceBuffers = std::vector<CMonoBuffer<float>>(numberOfImageSources, CMonoBuffer<float>(globalParameters.GetBufferSize()));			
			}
			SyncAllVirtualSourcesToModel();
		}

		void ResetProcessBuffers() {
			//TODO Implement samples buffer cleaning.
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void UpdateCommand() override {
		}
		

	private:
								
		/**
		* @brief Initialize the environment variables, required before processing. Room is always positioned
		*		 with one corner in {0, 0, 0} and the room dimensions taken as coordinates define the opposite corner,
		*		 required before calling process
		* @param roomDimensions Room dimensions in meters expressed as a CVector3 with form {x, y, z}
		*/
		void InitISMEnvironment(const int & order, const float & _maxDistanceSourcesToListener, const float & _windowSlopeDistance, std::shared_ptr<Common::CRoom> & _room) {
			if (!initialized && setupDone) return; // It is not initialized or it is already setup
			
			Common::CTransform sourceTransform = GetPositionEntryPoint("sourcePosition")->GetData();			
			Common::CTransform listenerTransform = GetPositionEntryPoint("listenerPosition")->GetData();
			
			// Set room and other parameters			
			CISMEnvironment::Setup(order, _maxDistanceSourcesToListener, _windowSlopeDistance, _room, sourceTransform, listenerTransform);		
						
			numberOfImageSources = CISMEnvironment::GetNumberOfImageSources();

			CMonoBuffer<float> inBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());			
			virtualSourceBuffers = std::vector<CMonoBuffer<float>>(numberOfImageSources, inBuffer);
			virtualSourcePositions = std::vector<Common::CTransform>(numberOfImageSources);
			
			CreateBRTVirtualSources();	
			SyncAllVirtualSourcesToModel();						
		}
			

		/**
		 * @brief Resets the virtual sources by disconnecting from the listener model, removing all virtual sources, and marking setup as incomplete.
		 * @param _listenerModel A shared pointer to the listener model to disconnect from.
		 * @return Returns a boolean value indicating the success or failure of the reset operation.
		 */
		bool ResetVirtualSources(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {
			//std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			setupDone = false;
			bool result = DisconnectToListenerModel(_listenerModel);
			if (!result && virtualSourcesConnectedToListener) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There was an error disconnecting the virtual sources from the listener model");
				return false;
			}			
			RemoveBRTVirtualSources(); // Remove all the virtual sources			
			CISMEnvironment::Reset(); // Reset the ISM environment
			numberOfImageSources = CISMEnvironment::GetNumberOfImageSources();
			virtualSourceBuffers.clear();
			virtualSourcePositions.clear();			

			if (!result) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There was an error removing the virtual sources");
				return false;
			}
			return result;
		}

		/**
		 * @brief Create the BRT virtual sources
		 */
		void CreateBRTVirtualSources() {			
			for (int i = 0; i < numberOfImageSources; i++) {
				CreateVirtualSource(GetBRTVirtualSourceID(i), originalSourceID);
			}			
		}
		
		/**
		 * @brief Removes all BRT virtual sound sources managed by the BRT manager.
		 */
		void RemoveBRTVirtualSources() {			
			for (int i = 0; i < numberOfImageSources; i++) {
				RemoveVirtualSource(GetBRTVirtualSourceID(i));
			}
		}

		/**
		 * @brief Sync the virtual sources to the model
		 */
		void SyncAllVirtualSourcesToModel() {
			
			ASSERT(virtualSourceBuffers.size() == numberOfImageSources, RESULT_ERROR_BADSIZE, "The number of virtual source buffers does not match the number of image sources", "");
			ASSERT(virtualSourcePositions.size() == numberOfImageSources, RESULT_ERROR_BADSIZE, "The number of virtual source positions does not match the number of image sources", "");
			for (int i = 0; i < numberOfImageSources; i++) {
				SyncOneVirtualSourceToModel(i);
			}
		}

		/**
		 * @brief Sync a virtual source to the model
		 * @param index Index of the virtual source
		 */
		void SyncOneVirtualSourceToModel(int index) {

			if (muteReverbPath) {
				std::fill(virtualSourceBuffers[index].begin(), virtualSourceBuffers[index].end(), 0);				
			}						
			SetVirtualSourcePosition(GetBRTVirtualSourceID(index), virtualSourcePositions[index]);						
			virtualSourceBuffers[index].ApplyGain(gain);
			SetVirtualSourceBuffer(GetBRTVirtualSourceID(index), virtualSourceBuffers[index]);
		}
		
		/**
		 * @brief Get the ID of a wall
		 * @param wallIndex Index of the wall
		 * @return ID of the wall
		 */
		std::string GetBRTVirtualSourceID(int _index) const {			
			return originalSourceID + "_ISM_VirtualSource_" + std::to_string(_index);						
		}
		
		/////////////////
		// Attributes
		/////////////////
		
		mutable std::mutex mutex;							// To avoid access collisions
		Common::CGlobalParameters globalParameters;
		std::vector<CMonoBuffer<float>> virtualSourceBuffers;
		std::vector<Common::CTransform> virtualSourcePositions;		
		BRTBase::CBRTManager * brtManager;		
		
		
		std::string originalSourceID;
		bool initialized;
		bool setupDone;
		bool enableProcessor;				
		bool virtualSourcesConnectedToListener; // Flag to know if the virtual sources are connected to a listener model
		//bool muteLoS;										// Direct Path mute
		bool muteReverbPath; // Reverb Path mute

		float gain;
		 
		
		size_t numberOfImageSources;
	};
};
#endif

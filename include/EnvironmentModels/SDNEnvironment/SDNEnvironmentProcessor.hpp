/**
* \class CSDNEnvironmentProcessor
*
* \brief  This class allows to connect the SDN-based environment renderer with the rest of the classes in the library. 
*			Performs SDN rendering for one source.
* \date Sep 2023
* 
* \authors  Developer's team (University of Milan), in alphabetical order: F. Avanzini, D. Fantini , M. Fontana, G. Presti,
* Coordinated by F. Avanzini (University of Milan) ||
*
* \b Contact: federico.avanzini@unimi.it
*
* \b Copyright: University of Milan - 2023
*
* \b Contributions: D. Gonzalez-Toledo (University of Malaga)
*
* \b Project: SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
*/
#ifndef _SDN_ENVIRONMENT_PROCESSOR_HPP_
#define _SDN_ENVIRONMENT_PROCESSOR_HPP_

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Base/BRTManager.hpp>
#include <Base/AdvancedEntryPointManager.hpp>
#include <EnvironmentModels/VirtualSourceList.hpp>
#include <EnvironmentModels/SDNEnvironment/SDNEnvironment.hpp>




namespace BRTEnvironmentModel {

	class CSDNEnvironmentProcessor : public BRTBase::CAdvancedEntryPointManager, public BRTBase::CExitPointManager, public CVirtualSourceList, SDNEnvironment {
	public:

		CSDNEnvironmentProcessor(BRTBase::CBRTManager * _brtManager)
			: CVirtualSourceList(_brtManager)
			, brtManager { _brtManager }
			, initialized { false }
			, muteLoS { false }
			, muteReverbPath { false }
			, enableProcessor { true } {

			CreateSamplesEntryPoint("inputSamples");
			CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");       
			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");						
		}

		~CSDNEnvironmentProcessor() {};
				
		/**
		 * @brief Setup the environment processor
		 * @param _orinalSourceID ID of the original source
		 * @return True if the setup was successful
		 */
		bool Setup(std::string _orinalSourceID)
		{
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (initialized) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The SDN environment processor is already initialized");
				return false;
			}

			if (_orinalSourceID == "" ) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The source ID must be defined");
				return false;
			}
							
			originalSourceID = _orinalSourceID;

			CreateBRTVirtualSources();	
			InitSDNEnvironment(Common::CVector3(1,1,1));
			globalCoordinatesRoomCentre = Common::CVector3::ZERO();
			initialized = true;
			return true;
		}

		/**
		 * @brief Setup the room
		 * @param _roomDimensionsInGlobalCoordinates Dimensions of the room in global coordinates
		 * @param _globalCoordinatesRoomCentre Centre of the room in global coordinates
		 * @return True if the setup was successful
		 */
		bool SetupRoom(Common::CVector3 _roomDimensionsInGlobalCoordinates, Common::CVector3 _globalCoordinatesRoomCentre) {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (!initialized) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The SDN environment processor is not initialized");
				return false;
			}
			if (_roomDimensionsInGlobalCoordinates == Common::CVector3::ZERO()) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The room dimensions must be defined");
				return false;
			}
			SetRoomDimensions(_roomDimensionsInGlobalCoordinates.x, AXIS_X);
			SetRoomDimensions(_roomDimensionsInGlobalCoordinates.y, AXIS_Y);
			SetRoomDimensions(_roomDimensionsInGlobalCoordinates.z, AXIS_Z);

			globalCoordinatesRoomCentre = _globalCoordinatesRoomCentre;
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
		 * @brief Connect the environment processor to a listener model
		 * @param _listenerModel Listener model to connect
		 * @return True if the connection was successful
		 */
		bool ConnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listenerModel) {
			return ConnectVirtualSourcesToListenerModel<BRTBase::CListenerModelBase>(_listenerModel);
		}

		/**
		 * @brief Disconnect the environment processor from a listener model
		 * @param _listenerModel Listener model to disconnect
		 * @return True if the disconnection was successful
		 */
		bool DisconnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listenerModel) {
			return DisconnectVirtualSourcesToListenerModel<BRTBase::CListenerModelBase>(_listenerModel);
		}

		/**
		* @brief Mute or unmute line of sight component
		* @param mute True to mute the line of sight component, False to unmute
		*/
		void MuteLOS(bool mute)
		{
			muteLoS = mute;
		}
		
		/**
		 * @brief Return the mute status of the line of sight component
		 * @return True if the line of sight component is muted, False otherwise
		 */
		bool GetMuteLOS() {
			return muteLoS;
		}

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
		* @brief Set a new absortion value for a frequency of a wall in the room
		* @param newValue New absorption value
		* @param wallIndex Index of the desired wall, the array of walls is constructed as [X0, XSize, Y0, YSize, Z0, ZSize]
		* @param freqIndex Index of the frequency to change, the array of frequencies is [125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		*/
		void SetWallFreqAbsorption(float newValue, int wallIndex, int freqIndex)
		{
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			wallNodes[wallIndex].SetFreqAbsorption(newValue, freqIndex);
		}		
		
		/**
		 * @brief Set the frequency absorption values array of a wall
		 * @param wallIndex Wall index
		 * @param newValues Absorption values vector, 8 values are expected, the centre frequencies 
			of which are as follows: [125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		 */
		void SetWallFreqAbsorption(int _wallIndex, std::vector<float> _newValues) {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (_newValues.size() != SDNParameters::NUM_FREQ) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The number of values must be equal to the number of frequencies");
			}
			wallNodes[_wallIndex].SetFreqAbsortion(_newValues);
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

			if (!enableProcessor) {
				virtualSourceBuffers = std::vector<CMonoBuffer<float>>(SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT, CMonoBuffer<float>(globalParameters.GetBufferSize()));
				SyncAllVirtualSourcesToModel();
				return;
			}			
			// Get data from entry points
			CMonoBuffer<float> inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();
			Common::CTransform sourcePosition = CalculateLocalPosition(GetPositionEntryPoint("sourcePosition")->GetData());
			Common::CTransform listenerPosition = CalculateLocalPosition(GetPositionEntryPoint("listenerPosition")->GetData());
						
			if (inBuffer.size() == 0) {
				std::cout << "Buffer Size = 0" << std::endl;
				SET_RESULT(RESULT_ERROR_BADSIZE, "The input buffer size is 0");
				return;
			}

			ASSERT(inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");

			// If the source or listener position exceed the size of the room silence the output			
			if (IsInBounds(sourcePosition.GetPosition()) && IsInBounds(listenerPosition.GetPosition())) {				
				Process(inBuffer, sourcePosition, listenerPosition, virtualSourceBuffers, virtualSourcePositions);				
			} else {		
				virtualSourceBuffers = std::vector<CMonoBuffer<float>>(SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT, CMonoBuffer<float>(inBuffer.size()));			
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
		* @brief Change the room dimensions along one axis
		* @param newValue New dimension in meters
		* @param _axis Axis whose dimension needs to be updated
		*/
		void SetRoomDimensions(float newValue, TAxis _axis) {
			//std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			switch (_axis) {
			case AXIS_X:
				dimensions.x = newValue;
				break;
			case AXIS_Y:
				dimensions.y = newValue;
				break;
			case AXIS_Z:
				dimensions.z = newValue;
				break;
			case AXIS_MINUS_X:
				dimensions.x = -newValue;
				break;
			case AXIS_MINUS_Y:
				dimensions.y = -newValue;
				break;
			case AXIS_MINUS_Z:
				dimensions.z = -newValue;
				break;
			default:
				SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Trying to set an axis which name is not defined");
			}

			hasChanged = true;
		}

		/**
		 * @brief Transform a position from global to SDN local
		 * @param _globalLocation global location
		 * @return local location
		 */
		Common::CTransform CalculateLocalPosition(const Common::CTransform& _globalPosition) {
			
			
			// Calculate parameter
			Common::CVector3 localCentre = (dimensions * 0.5f);
			Common::CVector3 transformParameter = localCentre - globalCoordinatesRoomCentre;
			// Calculate new position						
			Common::CTransform localPosition = _globalPosition;
			localPosition.SetPosition(_globalPosition.GetPosition() + transformParameter);

			return localPosition;
		}
		
		/**
		 * @brief Transform a position from SDN local to global
		 * @param _globalLocation global location
		 * @return local location
		 */
		Common::CTransform CalculateGlobalPosition(const Common::CTransform & _globalPosition) { 
			// Calculate parameter
			Common::CVector3 localCentre = (dimensions * 0.5f);
			Common::CVector3 transformParameter = globalCoordinatesRoomCentre - localCentre;
			// Calculate new position
			Common::CTransform globalPosition = _globalPosition;
			globalPosition.SetPosition(_globalPosition.GetPosition() + transformParameter);

			return globalPosition;
		}



		/**
		 * @brief Create the BRT virtual sources
		 */
		void CreateBRTVirtualSources() {

			for (int i = 0; i < SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT; i++) {
				CreateVirtualSource(GetBRTVirtualSourceID(i), originalSourceID);
			}
		}

		/**
		* @brief Initialize the environment variables, required before processing. Room is always positioned
		*		 with one corner in {0, 0, 0} and the room dimensions taken as coordinates define the opposite corner,
		*		 required before calling process
		* @param roomDimensions Room dimensions in meters expressed as a CVector3 with form {x, y, z}
		*/
		void InitSDNEnvironment(Common::CVector3 roomDimensions) {
			CMonoBuffer<float> inBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			virtualSourceBuffers = std::vector<CMonoBuffer<float>>(SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT, inBuffer);
			virtualSourcePositions = std::vector<Common::CTransform>(SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT);

			SyncAllVirtualSourcesToModel();

			Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();

			Prepare(globalParameters.GetSampleRate(), roomDimensions, sourcePosition, listenerPosition, virtualSourcePositions);
		}
				
		/**
		 * @brief Sync the virtual sources to the model
		 */
		void SyncAllVirtualSourcesToModel() {
			for (int i = 0; i < virtualSourceBuffers.size(); i++) {
				SyncVirtualSourceToModel(i);
			}
		}

		/**
		 * @brief Sync a virtual source to the model
		 * @param index Index of the virtual source
		 */
		void SyncVirtualSourceToModel(int index) {

			if ((index < 6 && muteReverbPath) || (index == 6 && muteLoS)) {
				std::fill(virtualSourceBuffers[index].begin(), virtualSourceBuffers[index].end(), 0);				
			} 			
			SetVirtualSourcePosition(GetBRTVirtualSourceID(index), CalculateGlobalPosition(virtualSourcePositions[index]));
			SetVirtualSourceBuffer(GetBRTVirtualSourceID(index), virtualSourceBuffers[index]);
		}
		
		/**
		 * @brief Get the ID of a wall
		 * @param wallIndex Index of the wall
		 * @return ID of the wall
		 */
		std::string GetBRTVirtualSourceID(int _index) {
			switch (_index) {
			case 0:
				return originalSourceID + "_SDN_WallX0";
			case 1:
				return originalSourceID + "_SDN_WallX1";
			case 2:
				return originalSourceID + "_SDN_WallY0";
			case 3:
				return originalSourceID + "_SDN_WallY1";
			case 4:
				return originalSourceID + "_SDN_WallZ0";
			case 5:
				return originalSourceID + "_SDN_WallZ1";
			case 6:
				return originalSourceID + "_SDN_DirectPath";
			default:
				return "";
			}
		}
		
		/////////////////
		// Attributes
		/////////////////
		
		mutable std::mutex mutex;							// To avoid access collisions
		Common::CGlobalParameters globalParameters;
		std::vector<CMonoBuffer<float>> virtualSourceBuffers;
		std::vector<Common::CTransform> virtualSourcePositions;		
		BRTBase::CBRTManager * brtManager;		
		
		bool muteLoS;			// Direct Path mute
		bool muteReverbPath; // Reverb Path mute

		std::string originalSourceID;
		bool initialized;
		bool enableProcessor;
		Common::CVector3 globalCoordinatesRoomCentre; 
	};



};
#endif

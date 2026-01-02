/**
* \class CEnviromentModelBase
*
* \brief Declaration of CEnviromentVirtualSourceBaseModel class
* \date	July 2023
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

#ifndef _CENVIRONMENT_MODEL_BASE_HPP_
#define _CENVIRONMENT_MODEL_BASE_HPP_

#include <memory>
#include <Base/ModelBase.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <SourceModels/VirtualSourceModel.hpp>
#include <ServiceModules/Room.hpp>

namespace BRTSourceModel {
	class CSourceOmnidirectionalModel;
	class CSourceDirectivityModel;
}

namespace BRTEnvironmentModel {
	class CEnviromentModelBase : public BRTBase::CModelBase {
	public:
		
		// Virtual Methods
		virtual ~CEnviromentModelBase() { }		
				
		virtual void UpdateGain() = 0;

		virtual void EnableDirectPath() {};
		virtual void DisableDirectPath() {};
		virtual bool IsDirectPathEnabled() { return false; };

		virtual void EnableReverbPath() {};
		virtual void DisableReverbPath() {};
		virtual bool IsReverbPathEnabled() { return false; };		

		virtual void EnableDistanceAttenuation() {};
		virtual void DisableDistanceAttenuation() {};
		virtual bool IsDistanceAttenuationEnabled() { return false; };

		virtual void EnablePropagationDelay() {};
		virtual void DisablePropagationDelay() {};
		virtual bool IsPropagationDelayEnabled() { return false; };

		virtual bool SetRoom(std::shared_ptr<BRTServices::CRoom>) { return false; };
		virtual bool UpdateRoom() { return false; };
		virtual std::shared_ptr<BRTServices::CRoom> GetRoom() const { return nullptr; }		
		virtual void RemoveRoom() { };
						
		virtual bool SetDistanceAttenuationFactor(float _distanceAttenuationDB) { return false; };
		virtual float GetDistanceAttenuationFactor() { return 0; };
		
		virtual bool SetReferenceAttenuationDistance(float _referenceAttenuationDistance) { return false; };
		virtual float GetReferenceAttenuationDistance() { return 0; };


		virtual bool SetReflectionOrder(int _reflectionOrder) { return false; }
		virtual int GetReflectionOrder() { return 0; }

		virtual bool SetMaxDistanceSourcesToListener(float _maxDistanceSourcesToListener) { return false; }
		virtual float GetMaxDistanceSourcesToListener() { return 0; }

		virtual bool SetTransitionMeters(float _transitionMeters) { return false; }
		virtual float GetTransitionMeters() { return 0; }

		virtual size_t GetNumberOfVirtualSources() { return 0; }
		virtual size_t GetNumberOfActiveVirtualSources() { return 0; }

		virtual bool ConnectSoundSource(const std::string & _sourceID) { return false; }
		virtual bool DisconnectSoundSource(const std::string & _sourceID) { return false; }	

		virtual bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) { return false; }
		virtual bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) { return false; }	
		
		CEnviromentModelBase(const std::string & _environmentModelID)
			: CModelBase(_environmentModelID) {

			
			CreateIDExitPoint();
			//CreateIDEntryPoint("listenerID");
			CreateIDEntryPoint("listenerModelID");
			GetIDExitPoint()->sendData(modelID);
			CreateCommandEntryPoint();
		}
		
		/**
		 * @brief Set the gain of the model
		 * @param _gain New gain value
		 */
		void SetGain(float _gain) override {
			gain = _gain;
			UpdateGain();
		}

		/**
		 * @brief Check if this environment is already connected to a listener model
		 * @return 
		 */
		bool IsConnectedToListenerModel() {
			std::string _listenerModelID = GetIDEntryPoint("listenerModelID")->GetData();
			return (_listenerModelID != "");			
		}


		/*void SetupRoom(Common::CRoom& _room) {
			roomDefinition = _room;
		}*/

		/**
		 * @brief Set up a shoebox room
		 * @param length extension of the room along the X axis
		 * @param width extension of the room along the Y axis.
		 * @param height extension of the room along the Z axis
		 */
		//bool SetupShoeBoxRoom(float length, float width, float height) {
		//	if (length <= 0 || width <= 0 || height <= 0) return false;
		//	
		//	Common::CVector3 currentRoomDimensions = Common::CVector3::ZERO();
		//	if (roomDefinition.IsShoeBox()) { 
		//		currentRoomDimensions = roomDefinition.GetShoeBoxRoomSize();				
		//	}
		//	if (currentRoomDimensions.x == length && currentRoomDimensions.y == width && currentRoomDimensions.z == height) {
		//		return true; 				// No need to update
		//	}
		//	if (roomDefinition.SetupShoeBox(length, width, height)) {
		//		UpdateRoomGeometry();
		//		return true;
		//	}
		//	return false;
		//}

		/**
		 * @brief Get room definition
		 * @return Room defined
		 */
		/*Common::CRoom& GetRoom() {
			return roomDefinition;
		}*/

		/**
		 * @brief Sets the absortion coeficient (frequency independent) of one wall
		 *	\details Sets the absortion coeficient (absorved energy / incident energy) of the i-th wall of the room.
		 *			Absortion coeficient of the wall (expressed as a number between 0 (no absortion) and 1 (total absortion).
		 * @param wallIndex index of the wall
		 * @param absortion absortion coeficient (frequency independent)
		 */
		/*bool SetRoomWallAbsortion(int& wallIndex, float& absortion) {
			if (roomDefinition.SetWallAbsortion(wallIndex, absortion)) {				
				UpdateRoomWallAbsortion(wallIndex);
				return true;
			} 
			return false;
		}*/

		/**
		 * @brief Sets the absorption coeficient (frequency independent) of all walls
		 *	\details Sets the absorption coeficient (absorved energy / incident energy) of each of the nine bands for the i-th wall of the room
		 * @param absortion absortion coeficient (frequency independent)
		 */ 
		/*bool SetRoomAllWallsAbsortion(float& _absortion) {
			if (roomDefinition.SetAllWallsAbsortion(_absortion)) {
				UpdateRoomAllWallsAbsortion();
				return true;
			}
			return false;			
		}*/

		/**
		 * @brief Sets the absorption coeficient (frequency dependent) of one wall
		 *	\details Sets the absorption coeficient (absorved energy / incident energy) of each of the nine bands for the i-th wall of the room
		 * @param wallIndex index of the wall
		 * @param absortionPerBand absortion coeficients for each band (frequency dependent). 9 bands are expected, 
								the centre frequencies of which are as follows:	[62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		 */
		/*bool SetRoomWallAbsortion(int wallIndex, const std::vector<float>& absortionPerBand) {
			if (roomDefinition.SetWallAbsortion(wallIndex, absortionPerBand)) {
				UpdateRoomWallAbsortion(wallIndex);
				return true;
			}
			return false;
		}*/
		
		/**
		 * @brief Sets the absortion coeficient (frequency dependent) of all walls
		 *	\details Sets the absortion coeficient (absorved energy / incident energy) of each of the nine bands for the i-th wall of the room
		 * @param absortionPerBand absortion coeficients for each band (frequency dependent). 9 bands are expected, 
								the centre frequencies of which are as follows:	[62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		 */
		/*bool SetRoomAllWallsAbsortion(const std::vector<float>& absortionPerBand) {
			if (roomDefinition.SetAllWallsAbsortion(absortionPerBand)) {
				UpdateRoomAllWallsAbsortion();
				return true;
			}
			return false;
		}*/
		
		/**
		 * @brief Enables a wall in the room by its index and updates the room geometry.
		 * @param _wallIndex Reference to the index of the wall to enable.
		 */
		/*void EnableRoomWall(int & _wallIndex) { 
			roomDefinition.EnableWall(_wallIndex);
			UpdateRoomGeometry();
		}*/
		/**
		 * @brief Disables a wall in the current room by its index and updates the room geometry.
		 * @param _wallIndex Reference to the index of the wall to disable.
		/* */
		/* void DisableRoomWall(int & _wallIndex) { 
			roomDefinition.DisableWall(_wallIndex);
			UpdateRoomGeometry();
		}*/
		/**
		 * @brief Checks if the specified room wall is enabled.
		 * @param _wallIndex Reference to the index of the wall to check.
		 * @return True if the room is defined and the specified wall is active; otherwise, false.
		 */
		/*bool IsRoomWallEnabled(int & _wallIndex) const { 
			
			if (!roomDefinition.IsRoomDefined()) {
				return false;
			}			
			return roomDefinition.IsWallActive(_wallIndex);		
		}*/


		/////////////////////
		// Update Callbacks
		/////////////////////		

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {
			// Nothing to do
		}
		
		void UpdateCommand() override {
			// Nothing to do
		}		

	private:
		
	protected:
		//Common::CRoom roomDefinition;
	};
}
#endif
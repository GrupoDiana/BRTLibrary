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
#include <Common/Room.hpp>

namespace BRTSourceModel {
	class CSourceSimpleModel;
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

		virtual void UpdateRoomGeometry() { };
		virtual void UpdateRoomWallAbsortion(int wallIndex) { };
		virtual void UpdateRoomAllWallsAbsortion() { };

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

		/**
		 * @brief Set up a shoebox room
		 * @param length extension of the room along the X axis
		 * @param width extension of the room along the Y axis.
		 * @param height extension of the room along the Z axis
		 */
		bool SetupShoeBoxRoom(float length, float width, float height) {
			if (roomDefinition.SetupShoeBox(length, width, height)) {
				UpdateRoomGeometry();
				return true;
			}
			return false;
		}

		/**
		 * @brief Get room definition
		 * @return Room defined
		 */
		Common::CRoom GetRoom() {
			return roomDefinition;
		}

		/**
		 * @brief Sets the absortion coeficient (frequency independent) of one wall
		 *	\details Sets the absortion coeficient (absorved energy / incident energy) of the i-th wall of the room.
		 *			Absortion coeficient of the wall (expressed as a number between 0 (no absortion) and 1 (total absortion).
		 * @param wallIndex index of the wall
		 * @param absortion absortion coeficient (frequency independent)
		 */
		bool SetRoomWallAbsortion(int wallIndex, float absortion) {
			if (roomDefinition.SetWallAbsortion(wallIndex, absortion)) {				
				UpdateRoomWallAbsortion(wallIndex);
				return true;
			} 
			return false;
		}

		/**
		 * @brief Sets the absorption coeficient (frequency independent) of all walls
		 *	\details Sets the absorption coeficient (absorved energy / incident energy) of each of the nine bands for the i-th wall of the room
		 * @param absortion absortion coeficient (frequency independent)
		 */ 
		bool SetRoomAllWallsAbsortion(float _absortion) {
			if (roomDefinition.SetAllWallsAbsortion(_absortion)) {
				UpdateRoomAllWallsAbsortion();
				return true;
			}
			return false;			
		}

		/**
		 * @brief Sets the absorption coeficient (frequency dependent) of one wall
		 *	\details Sets the absorption coeficient (absorved energy / incident energy) of each of the nine bands for the i-th wall of the room
		 * @param wallIndex index of the wall
		 * @param absortionPerBand absortion coeficients for each band (frequency dependent). 9 bands are expected, 
								the centre frequencies of which are as follows:	[62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		 */
		bool SetRoomWallAbsortion(int wallIndex, std::vector<float> absortionPerBand) {
			if (roomDefinition.SetWallAbsortion(wallIndex, absortionPerBand)) {
				UpdateRoomWallAbsortion(wallIndex);
				return true;
			}
			return false;
		}
		
		/**
		 * @brief Sets the absortion coeficient (frequency dependent) of all walls
		 *	\details Sets the absortion coeficient (absorved energy / incident energy) of each of the nine bands for the i-th wall of the room
		 * @param absortionPerBand absortion coeficients for each band (frequency dependent). 9 bands are expected, 
								the centre frequencies of which are as follows:	[62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		 */
		bool SetRoomAllWallsAbsortion(std::vector<float> absortionPerBand) {
			if (roomDefinition.SetAllWallsAbsortion(absortionPerBand)) {
				UpdateRoomAllWallsAbsortion();
				return true;
			}
			return false;
		}


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
		//std::string environmentModelID;															// Store unique enviroment ID	
		Common::CRoom roomDefinition;
	};
}
#endif
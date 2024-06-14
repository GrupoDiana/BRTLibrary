#ifndef _SDN_ENVIRONMENT_MODE_HPP_
#define _SDN_ENVIRONMENT_MODE_HPP_

#include <memory>
#include <Base/EnvironmentModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <EnvironmentModels/SDNenv/SDNEnvironment.hpp>

namespace BRTEnvironmentModel {

	class SDNEnvironmentModel : public BRTBase::CEnviromentVirtualSourceBaseModel, SDNEnvironment {
	public:

		SDNEnvironmentModel(BRTBase::CBRTManager* _brtManager) : BRTBase::CEnviromentVirtualSourceBaseModel(_brtManager) {


			CreateVirtualSource("WallX0");
			CreateVirtualSource("WallX1");
			CreateVirtualSource("WallY0");
			CreateVirtualSource("WallY1");
			CreateVirtualSource("WallZ0");
			CreateVirtualSource("WallZ1");
			CreateVirtualSource("DirectPath");

		}

		~SDNEnvironmentModel() {};

		void UpdateEntryPointData(std::string entryPointID) override {};

		void Update(std::string _entryPointID) override{

			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "inputSamples") {
				// Get data from entry points
				inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();
				sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();


				if (inBuffer.size() != 0) {

					ASSERT(inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
					
					// If the source or listener position exceed the size of the room silence the output
					if (IsInBounds(sourcePosition.GetPosition()) && IsInBounds(listenerPosition.GetPosition()))
					{
						Process(inBuffer, sourcePosition, listenerPosition, virtualSourceBuffers, virtualSourcePositions);
						SyncVirtualSourcesToModel();
					}
					else
					{
						for (CMonoBuffer<float>& buffer : virtualSourceBuffers)
						{
							std::fill(buffer.begin(), buffer.end(), 0);
						}
						SyncVirtualSourcesToModel();
					}

				}
			}
		}
		
		/**
		* @brief Initialize the environment variables, required before processing. Room is always positioned
		*		 with one corner in {0, 0, 0} and the room dimensions taken as coordinates define the opposite corner,
		*		 required before calling process
		* @param roomDimensions Room dimensions in meters expressed as a CVector3 with form {x, y, z}
		*/
		void Init(Common::CVector3 roomDimensions)
		{
			inBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			virtualSourceBuffers = std::vector<CMonoBuffer<float>>(SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT, inBuffer);
			virtualSourcePositions = std::vector<Common::CTransform>(SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT);

			SyncVirtualSourcesToModel();

			sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();

			Prepare(globalParameters.GetSampleRate(), roomDimensions, sourcePosition, 
				listenerPosition, virtualSourcePositions);
		}

		Common::CVector3 GetRoomDimensions() { return dimensions; }

		/**
		* @brief Change the room dimensions along one axis
		* @param newValue New dimension in meters
		* @param _axis Axis whose dimension needs to be updated
		*/
		void SetRoomDimensions(float newValue, TAxis _axis)
		{
			switch (_axis)
			{
			case AXIS_X: dimensions.x = newValue; break;
			case AXIS_Y: dimensions.y = newValue; break;
			case AXIS_Z: dimensions.z = newValue; break;
			case AXIS_MINUS_X: dimensions.x = -newValue;	break;
			case AXIS_MINUS_Y: dimensions.y = -newValue;	break;
			case AXIS_MINUS_Z: dimensions.z = -newValue;	break;
			default: SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Trying to set an axis which name is not defined");
			}

			hasChanged = true;
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
		* @brief Set a new absortion value for a frequency of a wall in the room
		* @param newValue New absorption value
		* @param wallIndex Index of the desired wall, the array of walls is constructed as [X0, XSize, Y0, YSize, Z0, ZSize]
		* @param freqIndex Index of the frequency to change, the array of frequencies is [125, 250, 500, 1000, 2000, 4000, 8000, 16000]Hz
		*/
		void setWallFreqAbsorption(float newValue, int wallIndex, int freqIndex)
		{
			wallNodes[wallIndex].SetFreqAbsorption(newValue, freqIndex);
		}

		void UpdateCommand() override {};

	private:

		void SyncVirtualSourcesToModel()
		{
			SetVirtualSourceBuffer("WallX0", virtualSourceBuffers[0]);
			SetVirtualSourcePosition("WallX0", virtualSourcePositions[0]);

			SetVirtualSourceBuffer("WallX1", virtualSourceBuffers[1]);
			SetVirtualSourcePosition("WallX1", virtualSourcePositions[1]);

			SetVirtualSourceBuffer("WallY0", virtualSourceBuffers[2]);
			SetVirtualSourcePosition("WallY0", virtualSourcePositions[2]);

			SetVirtualSourceBuffer("WallY1", virtualSourceBuffers[3]);
			SetVirtualSourcePosition("WallY1", virtualSourcePositions[3]);

			SetVirtualSourceBuffer("WallZ0", virtualSourceBuffers[4]);
			SetVirtualSourcePosition("WallZ0", virtualSourcePositions[4]);

			SetVirtualSourceBuffer("WallZ1", virtualSourceBuffers[5]);
			SetVirtualSourcePosition("WallZ1", virtualSourcePositions[5]);

			if (muteLoS)
				std::fill(virtualSourceBuffers[6].begin(), virtualSourceBuffers[6].end(), 0);
			SetVirtualSourceBuffer("DirectPath", virtualSourceBuffers[6]);
			SetVirtualSourcePosition("DirectPath", virtualSourcePositions[6]);
		}


		mutable std::mutex mutex;
		Common::CGlobalParameters globalParameters;

		std::vector<CMonoBuffer<float>> virtualSourceBuffers;
		std::vector<Common::CTransform> virtualSourcePositions;

		Common::CTransform sourcePosition, listenerPosition;

		CMonoBuffer<float> inBuffer;

		bool muteLoS = true;

	};



};
#endif
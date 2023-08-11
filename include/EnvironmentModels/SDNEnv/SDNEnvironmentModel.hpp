#ifndef _SDN_ENVIRONMENT_MODE_HPP_
#define _SDN_ENVIRONMENT_MODE_HPP_

#include <memory>
#include <Base/EnvironmentModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <EnvironmentModels/SDNenv/SDNEnvironment.hpp>

#define N_VIRTUAL_SOURCES 7

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


		void Update(std::string _entryPointID) {

			std::lock_guard<std::mutex> l(mutex);

			if (_entryPointID == "inputSamples") {
				// Get data from entry points
				// is inputSamples always a CMonoBuffer<float> ? // convert = to std::copy
				inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();  //to be fixed (better to receive a reference to data) ?
				sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();


				if (inBuffer.size() != 0) {

					ASSERT(inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
					
					if (isInBounds(sourcePosition.GetPosition()) && isInBounds(listenerPosition.GetPosition()))
					{
						Process(inBuffer, sourcePosition, listenerPosition, virtualSourceBuffers, virtualSourcePositions);
						syncVirtualSourcesToModel();
					}
					else
					{
						for (CMonoBuffer<float>& buffer : virtualSourceBuffers)
						{
							std::fill(buffer.begin(), buffer.end(), 0);
						}
						syncVirtualSourcesToModel();
					}

				}
				//this->resetUpdatingStack();
			}
		}

		void init()
		{
			inBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			virtualSourceBuffers = std::vector<CMonoBuffer<float>>(N_VIRTUAL_SOURCES, inBuffer);
			virtualSourcePositions = std::vector<Common::CTransform>(N_VIRTUAL_SOURCES);

			sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();

			prepare(globalParameters.GetSampleRate(), { 5, 5, 5 }, sourcePosition, listenerPosition, virtualSourcePositions);
		}

		void UpdateCommand() {
			
		}

		Common::CVector3 getRoomDimensions() { return dimensions; }

		void setRoomDimensions(float newValue, TAxis _axis)
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

		void muteLOS(bool mute)
		{
			muteLoS = mute;
		}

	private:

		void syncVirtualSourcesToModel()
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

		bool muteLoS = false;

	};



};
#endif
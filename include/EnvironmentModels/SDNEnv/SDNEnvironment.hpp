#ifndef _SDN_ENVIRONMENT_HPP_
#define _SDN_ENVIRONMENT_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <EnvironmentModels/SDNenv/WaveGuide.hpp>
#include <EnvironmentModels/SDNenv/ScatteringNode.hpp>

namespace BRTEnvironmentModel {


	class SDNEnvironment {
	
	public:

		SDNEnvironment() {
			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			wallNodes = std::vector<ScatteringNode>(SDNParameters::NUM_WALLS);
			sourceNode = std::vector<WaveGuide>(SDNParameters::NUM_WALLS);
			nodeListener = std::vector<WaveGuide>(SDNParameters::NUM_WALLS);
			NodeToNode = std::vector<WaveGuide>(SDNParameters::NUM_WALLS * (numConnectionsPerNode));
		};


		/**
		* @brief Process the _inBuffer with the SDN reverberator and save to _outBuffers
		* @param _inBuffer Input buffer as mono
		* @param _outBuffers Vector of output buffers, needs to have 7 mono buffers
		* @param _virtualSourcePositions Vector of transforms for each node in the model, 6 wall nodes and one source
		* @param sourcePosition Transform of current source position
		* @param listenerPosition Transform of current listener position
		*/
		void Process(CMonoBuffer<float>& _inBuffer, Common::CTransform sourcePosition, Common::CTransform listenerPosition, 
			std::vector<CMonoBuffer<float>>& _outBuffers, std::vector<Common::CTransform>& _virtualSourcePositions) 
		{
			ASSERT(_outBuffers.size() == SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT || 
				_virtualSourcePositions.size() == SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT
				, RESULT_ERROR_BADSIZE, "_outBuffers and _virtualSourcePositions size needs to be 7", "");

			// Update model state on geometry change
			if (source.GetPosition() != sourcePosition.GetPosition() || 
				receiver.GetPosition() != listenerPosition.GetPosition() ||
				hasChanged)
			{
				source.SetPosition(sourcePosition.GetPosition());
				receiver.SetPosition(listenerPosition.GetPosition());
				_virtualSourcePositions[SDNParameters::NUM_WALLS] = sourcePosition;
				hasChanged = true;
				UpdatePositions(_virtualSourcePositions);
			}

			//sourceBuffer.applyGain(source.getGain());

			int bufferDim = _inBuffer.size();

			//if (numInputChannels >= 2)
			//{
			//	for (int ch = 1; ch < numInputChannels; ch++)
			//	{
			//		sourceBuffer.addFrom(0, 0, sourceBuffer, ch, 0, bufferDim); // source to mono
			//	}
			//}

			int maxIndex = bufferDim - 1;
			//const float* currentReadPointer = sourceBuffer.getReadPointer(0);

			// Sample by sample processing of the input buffer
			for (int i = 0; i < bufferDim; i++)
			{
				// Smoothing of the source or listener movement
				if (sourceListener.IsInterpolating())
				{
					UpdateWaveguideLength();
				}
				ProcessSample(_inBuffer, _outBuffers, i, maxIndex);
			}

			hasChanged = false;

		}


		/**
		* @brief Verify if a given position is inside the current room bounds
		* @return True if position is inside the current room bounds
		*/
		bool IsInBounds(Common::CVector3 position)
		{
			return std::abs(position.x) < std::abs(dimensions.x) && ((position.x > 0) == (dimensions.x > 0)) &&
				std::abs(position.y) < std::abs(dimensions.y) && ((position.y > 0) == (dimensions.y > 0)) &&
				std::abs(position.z) < std::abs(dimensions.z) && ((position.z > 0) == (dimensions.z > 0));
		}

	protected:
		

		/**
		* @brief Initialize the SDN variables, required before calling process
		* @param samplerate Samplerate of the environment
		* @param dimensions Room dimensions in meters expressed as a CVector3 with form {x, y, z}
		* @param sourcePos Transform of current source position
		* @param playerPos Transform of current listener position
		* @param _virtualSourcePositions Vector of transforms for each node in the model, 6 wall nodes and one source
		*/
		void Prepare(double samplerate, Common::CVector3 dimensions,
			Common::CTransform sourcePos, Common::CTransform playerPos,
			std::vector<Common::CTransform>& _virtualSourcePositions)
		{
			ASSERT(_virtualSourcePositions.size() == SDNParameters::NUM_WAVEGUIDES_TO_OUTPUT
				, RESULT_ERROR_BADSIZE, "_virtualSourcePositions size needs to be 7", "");

			this->dimensions = dimensions;
			source.SetPosition(sourcePos.GetPosition());
			receiver.SetPosition(playerPos.GetPosition());

			_virtualSourcePositions[SDNParameters::NUM_WALLS] = sourcePos;

			InitWalls(samplerate, _virtualSourcePositions);
			InitWaveguides(samplerate);
		}

		Common::CVector3 dimensions = { 0.0f, 0.0f, 0.0f };
		bool hasChanged = false;


	private:

		//initialize wall nodes
		void InitWalls(double samplerate, std::vector<Common::CTransform>& _virtualSourcePositions)
		{
			dimensionsHelper[1] = dimensions.x;
			dimensionsHelper[3] = dimensions.y;
			dimensionsHelper[5] = dimensions.z;
			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				Common::CVector3 refl = ReflectionPoint(source.GetPosition(), receiver.GetPosition(), SDNParameters::axishelper[i], dimensionsHelper[i]);
				wallNodes[i].Init(samplerate, refl, numConnectionsPerNode, &sourceNode[i], &nodeListener[i]);
				_virtualSourcePositions[i].SetPosition(refl);
			}
		}

		//initialize waveguides and link to correct wall nodes
		void InitWaveguides(double samplerate)
		{

			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			float sourceListenerDist = PointToPointDistance(source.GetPosition(), receiver.GetPosition());
			sourceListener.Prepare(samplerate, &source, &receiver, sourceListenerDist);
			sourceListener.SetAttenuation(1 / sourceListenerDist);

			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				float sourceNodeDistance = PointToPointDistance(source.GetPosition(), wallNodes[i].GetPosition());
				float nodeListenerDistance = PointToPointDistance(wallNodes[i].GetPosition(), receiver.GetPosition());

				sourceNode[i].Prepare(samplerate, &source, &wallNodes[i], sourceNodeDistance);
				sourceNode[i].SetAttenuation(1 / sourceNodeDistance);

				nodeListener[i].Prepare(samplerate, &wallNodes[i], &receiver, nodeListenerDistance);
				nodeListener[i].SetAttenuation(1 / (1 + (nodeListenerDistance / sourceNodeDistance)));

				// vector construction such that inWaveguides[i]->GetStart() == outWaveguides[i]->GetEnd() is always true for each wall node
				for (int j = i + 1; j < SDNParameters::NUM_WALLS; j++)
				{
					float nodeDist = PointToPointDistance(wallNodes[j].GetPosition(), wallNodes[i].GetPosition());

					wallNodes[i].inWaveguides[(j - 1)] = &NodeToNode[(j * numConnectionsPerNode) + i]; //j node to i node
					wallNodes[j].outWaveguides[i] = wallNodes[i].inWaveguides[(j - 1)];

					wallNodes[i].outWaveguides[(j - 1)] = &NodeToNode[(i * numConnectionsPerNode) + (j - 1)]; //i node to j node
					wallNodes[j].inWaveguides[i] = wallNodes[i].outWaveguides[(j - 1)];

					wallNodes[i].inWaveguides[(j - 1)]->Prepare(samplerate, &wallNodes[j], &wallNodes[i], nodeDist);
					wallNodes[i].inWaveguides[(j - 1)]->SetAttenuation(1.0f);

					wallNodes[i].outWaveguides[(j - 1)]->Prepare(samplerate, &wallNodes[i], &wallNodes[j], nodeDist);
					wallNodes[i].outWaveguides[(j - 1)]->SetAttenuation(1.0f);

				}
			}

		}

		//recalculate position of wall nodes and update variables accordingly
		void UpdatePositions(std::vector<Common::CTransform>& _virtualSourcePositions)
		{
			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			dimensionsHelper[1] = dimensions.x;
			dimensionsHelper[3] = dimensions.y;
			dimensionsHelper[5] = dimensions.z;

			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				Common::CVector3 refl = ReflectionPoint(source.GetPosition(), receiver.GetPosition(), SDNParameters::axishelper[i], dimensionsHelper[i]);
				wallNodes[i].SetPosition(refl);
				_virtualSourcePositions[i].SetPosition(refl);
			}

			float sourceListenerDist = PointToPointDistance(source.GetPosition(), receiver.GetPosition());
			sourceListener.SetDistance(sourceListenerDist);
			sourceListener.SetAttenuation(1 / sourceListenerDist);


			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				float sourceNodeDistance = PointToPointDistance(source.GetPosition(), wallNodes[i].GetPosition());
				float nodeListenerDistance = PointToPointDistance(wallNodes[i].GetPosition(), receiver.GetPosition());

				sourceNode[i].SetDistance(sourceNodeDistance);
				sourceNode[i].SetAttenuation(1 / sourceNodeDistance);
				nodeListener[i].SetDistance(nodeListenerDistance);
				nodeListener[i].SetAttenuation(1 / (1 + (nodeListenerDistance / sourceNodeDistance)));

				for (int j = i + 1; j < SDNParameters::NUM_WALLS; j++)
				{
					float nodeDist = PointToPointDistance(wallNodes[j].GetPosition(), wallNodes[i].GetPosition());

					NodeToNode[(j * numConnectionsPerNode) + i].SetDistance(nodeDist); //j node to i node

					NodeToNode[(i * numConnectionsPerNode) + (j - 1)].SetDistance(nodeDist); //i node to j node

				}
			}
		}

		//process a sample insertion into the model
		void ProcessSample(CMonoBuffer<float>& sourceBuffer, std::vector<CMonoBuffer<float>>& _outBuffers, int sampleIndex, int maxIndex)
		{
			// inject sample into model
			sourceListener.PushNextSample(sourceBuffer[sampleIndex]);
			for (WaveGuide& guide : sourceNode)
			{
				guide.PushNextSample(sourceBuffer[sampleIndex]);
			}

			// process all scattering operations for current timestep
			ProcessNodes();

			//TODO check if distance attenuation is applied by listenermodel too
			// save output samples to the correct output buffers
			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				_outBuffers[i][sampleIndex] = nodeListener[i].GetCurrentSample();
			}
			_outBuffers[SDNParameters::NUM_WALLS][sampleIndex] = sourceListener.GetCurrentSample();
			
			TimeStep();
		}

		//process wall nodes
		void ProcessNodes()
		{
			for (ScatteringNode& node : wallNodes)
			{
				//if (node.hasNewAbsorption())
				//	node.updateFilterCoeffs(source.getSamplerate());

				node.Process();
			}
		}

		//advance simulation by one sample
		void TimeStep()
		{

			for (WaveGuide& guide : sourceNode)
			{
				guide.StepForward();
			}

			for (WaveGuide& guide : NodeToNode)
			{
				guide.StepForward();
			}

			for (WaveGuide& guide : nodeListener)
			{
				guide.StepForward();
			}

			sourceListener.StepForward();

		}

		void UpdateWaveguideLength()
		{
			sourceListener.InterpolateDistance();

			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;
			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{

				sourceNode[i].InterpolateDistance();
				nodeListener[i].InterpolateDistance();

				for (int j = i + 1; j < SDNParameters::NUM_WALLS; j++)
				{

					NodeToNode[(j * numConnectionsPerNode) + i].InterpolateDistance(); //j node to i node

					NodeToNode[(i * numConnectionsPerNode) + (j - 1)].InterpolateDistance(); //i node to j node

				}
			}
		}


		//returns vector from point b to point a
		Common::CVector3 DirVector(Common::CVector3& a, Common::CVector3& b)
		{
			return Common::CVector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		//return point of specular reflection between a and b on the wall aligned with reflAxis at distance WallPosition
		Common::CVector3 ReflectionPoint(Common::CVector3 a, Common::CVector3 b, char reflAxis, float wallPosition)
		{
			Common::CVector3 direction;
			float positionParam;

			switch (reflAxis)
			{
			case 'x':
				a.x = (2 * wallPosition) - a.x;
				direction = DirVector(a, b);
				positionParam = direction.x == 0 ? 0 : (wallPosition - a.x) / direction.x;
				return Common::CVector3( wallPosition, a.y + direction.y * positionParam, a.z + direction.z * positionParam );
				break;

			case 'y':
				a.y = (2 * wallPosition) - a.y;
				direction = DirVector(a, b);
				positionParam = direction.y == 0 ? 0 : (wallPosition - a.y) / direction.y;
				return Common::CVector3( a.x + direction.x * positionParam, wallPosition, a.z + direction.z * positionParam );
				break;

			case 'z':
				a.z = (2 * wallPosition) - a.z;
				direction = DirVector(a, b);
				positionParam = direction.z == 0 ? 0 : (wallPosition - a.z) / direction.z;
				return Common::CVector3( a.x + direction.x * positionParam, a.y + direction.y * positionParam, wallPosition );
				break;
			}

			return Common::CVector3( 0, 0, 0 );

		}

		float PointToPointDistance(Common::CVector3 startPos, Common::CVector3 endPos)
		{
			float distance = sqrtf(powf((startPos.x - endPos.x), 2) + powf((startPos.y - endPos.y), 2)
				+ powf((startPos.z - endPos.z), 2));

			if (distance < 1) distance = 1.0f;

			return distance;
		}


		WaveGuide sourceListener;
		std::vector<WaveGuide> sourceNode;
		std::vector<WaveGuide> NodeToNode;
		std::vector<WaveGuide> nodeListener;

		SDNNode source, receiver;

		std::vector<ScatteringNode> wallNodes;

		float dimensionsHelper[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	};

};
#endif
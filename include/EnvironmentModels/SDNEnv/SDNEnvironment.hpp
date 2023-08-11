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

		void Process(CMonoBuffer<float>& _inBuffer, Common::CTransform sourcePosition, Common::CTransform listenerPosition, 
			std::vector<CMonoBuffer<float>>& _outBuffers, std::vector<Common::CTransform>& _virtualSourcePositions) {


			if (source.getPosition() != sourcePosition.GetPosition() || 
				receiver.getPosition() != listenerPosition.GetPosition() ||
				hasChanged)
			{
				source.setPosition(sourcePosition.GetPosition());
				receiver.setPosition(listenerPosition.GetPosition());
				_virtualSourcePositions[SDNParameters::NUM_WALLS] = sourcePosition;
				hasChanged = true;
				updatePositions(_virtualSourcePositions);
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

			for (int i = 0; i < bufferDim; i++)
			{
				if (sourceListener.isInterpolating())
				{
					updateWaveguideLength();
				}
				processSample(_inBuffer, _outBuffers, i, maxIndex);
			}

			hasChanged = false;

		}

		bool isInBounds(Common::CVector3 position)
		{
			return position.x < dimensions.x && position.x > 0 &&
				position.y < dimensions.y && position.y > 0 &&
				position.z < dimensions.z && position.z > 0;
		}

	protected:
		
		void prepare(double samplerate, Common::CVector3 dimensions,
			Common::CTransform sourcePos, Common::CTransform playerPos,
			std::vector<Common::CTransform>& _virtualSourcePositions)
		{
			this->dimensions = dimensions;
			source.setPosition(sourcePos.GetPosition());
			receiver.setPosition(playerPos.GetPosition());

			_virtualSourcePositions[SDNParameters::NUM_WALLS] = sourcePos;

			initWalls(samplerate, _virtualSourcePositions);
			initWaveguides(samplerate);
		}

		Common::CVector3 dimensions = { 0.0f, 0.0f, 0.0f };
		bool hasChanged = false;


	private:

		//initialize wall nodes
		void initWalls(double samplerate, std::vector<Common::CTransform>& _virtualSourcePositions)
		{
			dimHelper[1] = dimensions.x;
			dimHelper[3] = dimensions.y;
			dimHelper[5] = dimensions.z;
			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				Common::CVector3 refl = reflectionPoint(source.getPosition(), receiver.getPosition(), SDNParameters::axishelper[i], dimHelper[i]);
				wallNodes[i].init(samplerate, refl, numConnectionsPerNode, &sourceNode[i], &nodeListener[i]);
				_virtualSourcePositions[i].SetPosition(refl);
			}
		}

		//initialize waveguides and link to correct wall nodes
		void initWaveguides(double samplerate)
		{

			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			float sourceListenerDist = distanceCalc(source.getPosition(), receiver.getPosition());
			sourceListener.prepare(samplerate, &source, &receiver, sourceListenerDist);
			sourceListener.setAttenuation(1 / sourceListenerDist);
			//source.outWaveguides[SDNParameters::NUM_WALLS] = &sourceListener;
			//receiver.inWaveguides[SDNParameters::NUM_WALLS] = &sourceListener;

			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				float sourceNodeDistance = distanceCalc(source.getPosition(), wallNodes[i].getPosition());
				float nodeListenerDistance = distanceCalc(wallNodes[i].getPosition(), receiver.getPosition());

				sourceNode[i].prepare(samplerate, &source, &wallNodes[i], sourceNodeDistance);
				sourceNode[i].setAttenuation(1 / sourceNodeDistance);
				//source.outWaveguides[i] = &sourceNode[i];

				nodeListener[i].prepare(samplerate, &wallNodes[i], &receiver, nodeListenerDistance);
				nodeListener[i].setAttenuation(1 / (1 + (nodeListenerDistance / sourceNodeDistance)));
				//receiver.inWaveguides[i] = &nodeListener[i];

				for (int j = i + 1; j < SDNParameters::NUM_WALLS; j++)
				{
					float nodeDist = distanceCalc(wallNodes[j].getPosition(), wallNodes[i].getPosition());

					wallNodes[i].inWaveguides[(j - 1)] = &NodeToNode[(j * numConnectionsPerNode) + i]; //j node to i node
					wallNodes[j].outWaveguides[i] = wallNodes[i].inWaveguides[(j - 1)];

					wallNodes[i].outWaveguides[(j - 1)] = &NodeToNode[(i * numConnectionsPerNode) + (j - 1)]; //i node to j node
					wallNodes[j].inWaveguides[i] = wallNodes[i].outWaveguides[(j - 1)];

					wallNodes[i].inWaveguides[(j - 1)]->prepare(samplerate, &wallNodes[j], &wallNodes[i], nodeDist);
					wallNodes[i].inWaveguides[(j - 1)]->setAttenuation(1.0f);

					wallNodes[i].outWaveguides[(j - 1)]->prepare(samplerate, &wallNodes[i], &wallNodes[j], nodeDist);
					wallNodes[i].outWaveguides[(j - 1)]->setAttenuation(1.0f);

				}
			}

		}

		//recalculate position of wall nodes and update variables accordingly
		void updatePositions(std::vector<Common::CTransform>& _virtualSourcePositions)
		{
			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;

			dimHelper[1] = dimensions.x;
			dimHelper[3] = dimensions.y;
			dimHelper[5] = dimensions.z;

			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				Common::CVector3 refl = reflectionPoint(source.getPosition(), receiver.getPosition(), SDNParameters::axishelper[i], dimHelper[i]);
				wallNodes[i].setPosition(refl);
				_virtualSourcePositions[i].SetPosition(refl);
			}

			float sourceListenerDist = distanceCalc(source.getPosition(), receiver.getPosition());
			sourceListener.setDistance(sourceListenerDist);

			if (!mutedLOS)
			{
				sourceListener.setAttenuation(1 / sourceListenerDist);
			}


			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				float sourceNodeDistance = distanceCalc(source.getPosition(), wallNodes[i].getPosition());
				float nodeListenerDistance = distanceCalc(wallNodes[i].getPosition(), receiver.getPosition());

				sourceNode[i].setDistance(sourceNodeDistance);
				sourceNode[i].setAttenuation(1 / sourceNodeDistance);
				nodeListener[i].setDistance(nodeListenerDistance);
				nodeListener[i].setAttenuation(1 / (1 + (nodeListenerDistance / sourceNodeDistance)));

				for (int j = i + 1; j < SDNParameters::NUM_WALLS; j++)
				{
					float nodeDist = distanceCalc(wallNodes[j].getPosition(), wallNodes[i].getPosition());

					NodeToNode[(j * numConnectionsPerNode) + i].setDistance(nodeDist); //j node to i node

					NodeToNode[(i * numConnectionsPerNode) + (j - 1)].setDistance(nodeDist); //i node to j node

				}
			}
		}

		//process a sample insertion into the system
		void processSample(CMonoBuffer<float>& sourceBuffer, std::vector<CMonoBuffer<float>>& _outBuffers, int sampleIndex, int maxIndex)
		{
			sourceListener.pushNextSample(sourceBuffer[sampleIndex]);
			for (WaveGuide& guide : sourceNode)
			{
				guide.pushNextSample(sourceBuffer[sampleIndex]);
			}

			processNodes();

			//TODO check if distance attenuation is applied by listenermodel too
			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{
				_outBuffers[i][sampleIndex] = nodeListener[i].getCurrentSample();
			}
			_outBuffers[SDNParameters::NUM_WALLS][sampleIndex] = sourceListener.getCurrentSample();

			timeStep();
		}

		//process wall nodes
		void processNodes()
		{
			for (ScatteringNode& node : wallNodes)
			{
				//if (node.hasNewAbsorption())
				//	node.updateFilterCoeffs(source.getSamplerate());

				node.process();
			}
		}

		//advance simulation by one sample
		void timeStep()
		{

			for (WaveGuide& guide : sourceNode)
			{
				guide.stepForward();
			}

			for (WaveGuide& guide : NodeToNode)
			{
				guide.stepForward();
			}

			for (WaveGuide& guide : nodeListener)
			{
				guide.stepForward();
			}

			sourceListener.stepForward();

		}

		void updateWaveguideLength()
		{
			sourceListener.interpolateDistance();

			int numConnectionsPerNode = SDNParameters::NUM_WALLS - 1;
			for (int i = 0; i < SDNParameters::NUM_WALLS; i++)
			{

				sourceNode[i].interpolateDistance();
				nodeListener[i].interpolateDistance();

				for (int j = i + 1; j < SDNParameters::NUM_WALLS; j++)
				{

					NodeToNode[(j * numConnectionsPerNode) + i].interpolateDistance(); //j node to i node

					NodeToNode[(i * numConnectionsPerNode) + (j - 1)].interpolateDistance(); //i node to j node

				}
			}
		}


		//returns vector from point b to point a
		Common::CVector3 dirVector(Common::CVector3& a, Common::CVector3& b)
		{
			return Common::CVector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		//return point of specular reflection between a and b on the wall aligned with reflAxis at distance WallPosition
		Common::CVector3 reflectionPoint(Common::CVector3 a, Common::CVector3 b, char reflAxis, float wallPosition)
		{
			Common::CVector3 direction;
			float positionParam;

			switch (reflAxis)
			{
			case 'x':
				a.x = (2 * wallPosition) - a.x;
				direction = dirVector(a, b);
				positionParam = (wallPosition - a.x) / direction.x;
				return Common::CVector3( wallPosition, a.y + direction.y * positionParam, a.z + direction.z * positionParam );
				break;

			case 'y':
				a.y = (2 * wallPosition) - a.y;
				direction = dirVector(a, b);
				positionParam = (wallPosition - a.y) / direction.y;
				return Common::CVector3( a.x + direction.x * positionParam, wallPosition, a.z + direction.z * positionParam );
				break;

			case 'z':
				a.z = (2 * wallPosition) - a.z;
				direction = dirVector(a, b);
				positionParam = (wallPosition - a.z) / direction.z;
				return Common::CVector3( a.x + direction.x * positionParam, a.y + direction.y * positionParam, wallPosition );
				break;
			}

			return Common::CVector3( 0, 0, 0 );

		}

		float distanceCalc(Common::CVector3 startPos, Common::CVector3 endPos)
		{
			float distance = sqrtf(powf((startPos.x - endPos.x), 2) + powf((startPos.y - endPos.y), 2)
				+ powf((startPos.z - endPos.z), 2));

			if (distance < 1) distance = 1.0f;

			return distance;
		}



		Common::CGlobalParameters globalParameters;

		WaveGuide sourceListener;
		std::vector<WaveGuide> sourceNode;
		std::vector<WaveGuide> NodeToNode;
		std::vector<WaveGuide> nodeListener;

		SDNNode source, receiver;

		std::vector<ScatteringNode> wallNodes;

		bool mutedLOS = false;
		float dimHelper[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	};

};
#endif
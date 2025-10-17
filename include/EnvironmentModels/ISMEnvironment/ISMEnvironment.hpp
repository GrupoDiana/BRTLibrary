/**
* \class CISMEnvironment
*
* \brief This class implements the ISM simulation. 
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
#ifndef _ISM_ENVIRONMENT_HPP_
#define _ISM_ENVIRONMENT_HPP_

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Common/Vector3.hpp>
#include <Common/Room.hpp>
#include "ISMParameters.hpp"
#include "ISMSourceImage.hpp"

namespace BRTEnvironmentModel {
	
	class CISMEnvironment {
	
		constexpr static float visibilityThreshold = 0.00001; // Threshold to consider a source as audible (visible)
	
	public:
		CISMEnvironment()
			: setupDone{ false }
			, reflectionOrder{ 1 }
			, ISMParameters{ std::make_shared<CISMParameters>() }
			, sourceLocation{ Common::CVector3(0, 0, 0) }
			, imageSources{ nullptr }
		{
		};

		/**
		 * @brief Initializes or reinitializes the image source model with the specified parameters for room acoustics simulation.
		 * @param order The reflection order to use for the image source model.
		 * @param _maxDistanceSourcesToListener The maximum allowed distance from image sources to the listener.
		 * @param _windowSlopeDistance The window slope distance parameter used in distance calculations.
		 * @param _room The room configuration to use for the simulation.
		 * @return Returns true if the setup was successful; returns false if the parameters are invalid or setup fails.
		 */
		bool Setup(const int& order, const float& _maxDistanceSourcesToListener, const float& _windowSlopeDistance, const Common::CRoom& _room, const Common::CTransform& _sourceTransform, const Common::CTransform& _listenerTransform) {
			if (setupDone) {
				setupDone = false;
				imageSources->Reset();
				imageSources.reset();
			}

			ISMParameters->room = _room;
			reflectionOrder = order;
			bool result = setMaxDistanceImageSources(_maxDistanceSourcesToListener, _windowSlopeDistance);
			if (!result) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "MaxDistanceSourcesToListener must be smaller than windowSlopeDistance/2");
				return false;
			}

			sourceLocation = _sourceTransform.GetPosition();
			ISMParameters->listenerPosition = _listenerTransform.GetPosition();

			imageSources = std::make_shared<CISMSourceImage>(ISMParameters);
			imageSources->createImagesTree(ISMParameters->room, reflectionOrder, sourceLocation);
			UpdateImageSourceDataFromImageTree();

			// TODO check if everything went fine before setting setupDone to true
			SetupWaveGuideProcessors();

			setupDone = true;
			return true;
		}

		/**
		 * @brief Sets the position of the listener based on the provided transform.
		 * @details Changing the listener's position only affects the visibility of image sources.
		 * @param _listenerTransform A reference to a CTransform object representing the listener's transform.
		 */
		void SetListenerPosition(const Common::CVector3& _listenerPosition) {
			std::lock_guard<std::mutex> l(mutex);
			if (_listenerPosition == ISMParameters->listenerPosition) return;
			ISMParameters->listenerPosition = _listenerPosition;
			UpdateImageSourcesTreeWithNewListenerPosition();
		}

		/** \brief Sets the original source location.
		*	\details Changing the position of the source affects the position of the image sources and their visibility
		*	\param [in] location: location of the original path source
		*/
		void SetSourcePosition(const Common::CVector3& _sourcePosition) {
			std::lock_guard<std::mutex> l(mutex);
			if (_sourcePosition == sourceLocation) return;
			sourceLocation = _sourcePosition;
			UpdateImageSourcesTreeWithNewSourcePosition();
		}

		void SetSourceAndListenerPositions(const Common::CVector3& _sourcePosition, const Common::CVector3& _listenerPosition) {
			std::lock_guard<std::mutex> l(mutex);
			SetSourceAndListenerPositions_withoutLock(_sourcePosition, _listenerPosition);								
		}

		/** \brief Returns the location of image sources
		*	\details This method returns a vector with the location of the image sources (reflectionsImage). The original source is not included
		*	\param [out] location: location of the direct path source
		*/
		std::vector<Common::CVector3> GetImageSourceLocations() const {			
			return imageSourcesPositionList;
		}

		/**
		 * @brief Returns the number of image sources available.
		 * @return The total count of image sources as a size_t value.
		 */
		size_t GetNumberOfImageSources() {			
			return imageSourcesPositionList.size();
		}
		
		/** \brief Returns data of all image sources
		*	\details This method returns the location of all image sources and wether they are visible or not, not including the
		*	original source (direct path).
		*	\param [out] ImageSourceData: Vector containing the data of the image sources
		*/
		std::vector<TImageSourceData> getImageSourceData() {
			std::lock_guard<std::mutex> l(mutex);
			return imageSourcesDataList;
		}

		/**
		* @brief Verify if a given position is inside the current room bounds
		* @return True if position is inside the current room bounds
		*/
		bool IsInBounds(const Common::CVector3& _position) {
			float distanceToNearestWall = 0.0f;
			return ISMParameters->room.CheckPointInsideRoom(_position, distanceToNearestWall);			
		}

		void Process(CMonoBuffer<float> & _inBuffer
			, const Common::CTransform & _sourceTransform
			, const Common::CTransform & _listenerTransform
			, std::vector<CMonoBuffer<float>> & _outBuffers
			, std::vector<Common::CTransform> & _virtualSourcePositions) 
		{
		
			std::lock_guard<std::mutex> l(mutex);
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			ASSERT(_outBuffers.size() == GetNumberOfImageSources() || _virtualSourcePositions.size() == GetNumberOfImageSources(), RESULT_ERROR_BADSIZE, "_outBuffers and _virtualSourcePositions size needs to be " + std::to_string(GetNumberOfImageSources()), "");


			SetSourceAndListenerPositions_withoutLock(_sourceTransform.GetPosition(), _listenerTransform.GetPosition());
			// Process the waveguide
			//Common::CVector3 waveGuideOutSourcePosition;			
			PushToWaveGuideProcessors(_inBuffer);
			//CMonoBuffer<float> _waveGuideOutBuffer;			
			
			// Pop front the waveguide to get the output buffer and the effective source position (with propagation delay)
			PopFromWaveGuideProcessors(_outBuffers, _virtualSourcePositions);		
						
			//Apply visiblity gain to each output buffer
			ApplyVisibilityGainToOutputBuffers(_outBuffers);
		}
		
	private:
		
		void ApplyVisibilityGainToOutputBuffers(std::vector<CMonoBuffer<float>> & _outBuffers) {
			if (_outBuffers.size() != imageSourcesDataList.size()) return;
			for (int i = 0; i < _outBuffers.size(); i++) {
				if (!imageSourcesDataList[i].visible) {
					_outBuffers[i] = CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f); //Set to zero
				} else if (imageSourcesDataList[i].visibility < 1.0f) {
					_outBuffers[i].ApplyGain(imageSourcesDataList[i].visibility);
				}
			}
		}

		/**
		 * @brief Sets the positions of the source and listener without acquiring a lock, and updates the image sources tree if either position has changed.
		 * @param _sourcePosition The new position of the source as a Common::CVector3 object.
		 * @param _listenerPosition The new position of the listener as a Common::CVector3 object.
		 */
		void SetSourceAndListenerPositions_withoutLock(const Common::CVector3 & _sourcePosition, const Common::CVector3 & _listenerPosition) {			
			if ((_listenerPosition != ISMParameters->listenerPosition) && (_sourcePosition != sourceLocation)) {
				ISMParameters->listenerPosition = _listenerPosition;
				sourceLocation = _sourcePosition;
				UpdateImageSourcesTreeWithNewSourcePosition();				
			} else if (_listenerPosition != ISMParameters->listenerPosition) {
				ISMParameters->listenerPosition = _listenerPosition;
				UpdateImageSourcesTreeWithNewListenerPosition();
			} else if (_sourcePosition != sourceLocation) {
				sourceLocation = _sourcePosition;
				UpdateImageSourcesTreeWithNewSourcePosition();
			} else
				return;
		}

		/**
		 * @brief Updates the listener position in ISM parameters.
		 * @details When the listener position changes, the visibility of image sources may also change.
		 * @param _listenerTransform 
		 */
		void UpdateImageSourcesTreeWithNewListenerPosition() {
			if (!setupDone) return;
			if (imageSources != nullptr) {
				imageSources->UpdateImagesTreeVisibilities();
				ActionsAfterUpdateImageSourcePositionsOrVisibilities();		
			}			
		}
		/**
		 * @brief Updates the image sources tree with a new source position if setup is complete.
		 * @details When the source position changes, both the positions and visibilities of image sources may change.
		 */
		void UpdateImageSourcesTreeWithNewSourcePosition() { 
			if (!setupDone) return;

			if (imageSources != nullptr) {
				imageSources->UpdateSourceLocation(sourceLocation);
				ActionsAfterUpdateImageSourcePositionsOrVisibilities();
			}			
		}

		/**
		 * @brief Performs a series of updates after image source positions or visibilities have changed.
		 */
		void ActionsAfterUpdateImageSourcePositionsOrVisibilities() {			
			UpdateImageSourceDataFromImageTree();
			UpdateImageSourcesPositionList();
			UpdateWaveGuideFiltersVisibility();
			ShowImageSourceData(imageSourcesDataList); // TO DELETE
		}

		/**
		 * @brief Updates image source data from the image source tree after any change
		 */
		void UpdateImageSourceDataFromImageTree() {
			if (imageSources == nullptr) return;
			
			imageSourcesDataList.clear();			
			imageSources->getImageSourcesData(imageSourcesDataList);
			UpdateImageSourcesPositionList();			
		}
		
		/**
		 * @brief Updates images source positions after any change
		 */
		void UpdateImageSourcesPositionList() {			
			imageSourcesPositionList.clear();
			for (auto & image : imageSourcesDataList) {
				imageSourcesPositionList.push_back(image.location);
			}
		}
		
		/** \brief Sets the maximum distance between the listener and each source image to be considered visible
		*	\details Sources that exceed the maximum distance will be considered non-visible sources.
		*	\param [in] maxDistanceSourcesToListener
		*	\param [in] windowSlopeDistance in meters (related to windowSlope time(s) in class CBRIR)
		*/
		bool setMaxDistanceImageSources(float _MaxDistanceSourcesToListener, float _windowSlopeDistance) {
			if (_windowSlopeDistance / 2 < _MaxDistanceSourcesToListener) {
				ISMParameters->maxDistanceSourcesToListener = _MaxDistanceSourcesToListener;
				ISMParameters->transitionMeters = _windowSlopeDistance;
				return true;
			}
			return false;
		}
		
		/**
		 * @brief Initializes the wave guide processors by clearing and resizing 
			the list of channel source-listeners based on the number of image source locations.
		 */
		void SetupWaveGuideProcessors() { 			
			listOfChannelSourceListener.clear();			
			listOfChannelSourceListener.resize(imageSourcesPositionList.size());
			SetupWaveGuidesFilters();
			EnablePropagationDelay();
		}

		/**
		 * @brief Sets up filters for all waveguide processors in the list using the reflection bands from the image sources data.
		 */ 
		void SetupWaveGuidesFilters() {
			if (listOfChannelSourceListener.size() != imageSourcesDataList.size()) return;
			for (int i = 0; i < listOfChannelSourceListener.size(); i++) {				
				listOfChannelSourceListener[i].SetupFilter(imageSourcesDataList[i].reflectionBands);
				if (imageSourcesDataList[i].visibility > visibilityThreshold) {
					listOfChannelSourceListener[i].EnablePropagationFilter();
				} else {
					listOfChannelSourceListener[i].DisablePropagationFilter();
				}
			}
		}
		/**
		 * @brief Updates the visibility of waveguide filters based on the visibility of image sources.
		 */
		void UpdateWaveGuideFiltersVisibility() {
			if (listOfChannelSourceListener.size() != imageSourcesDataList.size()) return;
			for (int i = 0; i < listOfChannelSourceListener.size(); i++) {
				if (imageSourcesDataList[i].visibility > visibilityThreshold) {
					listOfChannelSourceListener[i].EnablePropagationFilter();
				} else {
					listOfChannelSourceListener[i].DisablePropagationFilter();
				}
			}
		}		

		/**
		 * @brief Enables propagation delay for all waveguide processors in the list.
		 */
		void EnablePropagationDelay() {
			for (auto& _channel : listOfChannelSourceListener) {
				_channel.EnablePropagationDelay();
			}
		}
		
		/**
		 * @brief Disables propagation delay for all channel source listeners in the list.
		 */
		void DisablePropagationDelay() {
			for (auto& _channel : listOfChannelSourceListener) {
				_channel.DisablePropagationDelay();
			}
		}

		void PushToWaveGuideProcessors(CMonoBuffer<float> & _inBuffer) {						
			ASSERT(listOfChannelSourceListener.size() == imageSourcesPositionList.size(), RESULT_ERROR_BADSIZE, "Number of waveguide processors must be equal to the number of image sources", "");
			
			for (int i = 0; i < listOfChannelSourceListener.size(); i++) {
				listOfChannelSourceListener[i].PushBack(_inBuffer, imageSourcesPositionList[i], ISMParameters->listenerPosition);
			}
		}

		void PopFromWaveGuideProcessors(std::vector<CMonoBuffer<float>> & _outBuffers, std::vector<Common::CTransform> & _virtualSourcePositions) {
			int i =0;
			for (auto & _channel : listOfChannelSourceListener) {
				CMonoBuffer<float> _channelOutBuffer;
				Common::CVector3 _channelOutSourcePosition;
				_channel.PopFront(_channelOutBuffer, ISMParameters->listenerPosition, _channelOutSourcePosition);
				
				_outBuffers[i]=_channelOutBuffer;
				
				if (_channel.IsPropagationDelayEnabled()) {
					_virtualSourcePositions[i] = Common::CTransform(_channelOutSourcePosition);
				} else {
					_virtualSourcePositions[i] = Common::CTransform(sourceLocation);
				}
				i++;
			}
		};

		// TO remove
		void ShowImageSourceData(const std::vector<TImageSourceData> & data) {
			Common::CVector3 listenerLocation = ISMParameters->listenerPosition;
			int numVisibleSources = 0;

			auto w2 = std::setw(2);
			auto w5 = std::setw(5);
			auto w6 = std::setw(6);
			auto w7 = std::setw(7);
			std::cout << "------------------------------------------------List of Source Images ---------------------------------------------\n";
			std::cout << "  Visibility | Refl. |                Reflection coeficients                 |        Location       | Dist. (Room)\n";
			std::cout << "             | order | ";
			float freq = 62.5;
			for (int i = 0; i < NUM_BAND_ABSORTION; i++) {
				if (freq < 100) {
					std::cout << ' ';
				}
				if (freq < 1000) {
					std::cout << ((int)freq) << "Hz ";
				} else {
					std::cout << w2 << ((int)(freq / 1000)) << "kHz ";
				}
				freq *= 2;
			}
			std::cout << "|    X       Y       Z  |  \n";
			std::cout << "-------------+-------+-------------------------------------------------------+-----------------------+--------\n";
			for (int i = 0; i < data.size(); i++) {
				if (data.at(i).visible) {
					std::cout << "VISIBLE ";
					numVisibleSources++;
				}
				else
					std::cout << "        ";
				std::cout << w5 << std::fixed << std::setprecision(2) << data.at(i).visibility; //print source visibility
				std::cout << "|   " << data.at(i).reflectionWalls.size(); //print number of reflection needed for this source
				std::cout << "   | ";
				for (int j = 0; j < NUM_BAND_ABSORTION; j++) {
					std::cout << w5 << std::fixed << std::setprecision(2) << data.at(i).reflectionBands.at(j) << " "; //print abortion coefficientes for a source
				}
				std::cout << "| " << w6 << std::fixed << std::setprecision(2) << data.at(i).location.x << ", "; //print source location
				std::cout << w6 << std::fixed << std::setprecision(2) << data.at(i).location.y << ", ";
				std::cout << w6 << std::fixed << std::setprecision(2) << data.at(i).location.z << "|";

				std::cout << w6 << (data.at(i).location - listenerLocation).GetDistance(); //print distance to listener and distance between first and last reflection walls
				std::cout << " (" << data.at(i).reflectionWalls.front().GetMinimumDistanceFromWall(data.at(i).reflectionWalls.back()) << ")" << "\n";
			}
				
			std::cout << "Reflection order = " << reflectionOrder << "\n";
			std::cout << "Maximum distance to consider a source as visible = " << ISMParameters->maxDistanceSourcesToListener << " m\n";
			std::cout << "Visible sources: " << numVisibleSources << " out of " << data.size() << " total sources.\n";
			std::cout << "Source location = " << std::to_string(sourceLocation.x) << ", " << std::to_string(sourceLocation.y) << ", " << std::to_string(sourceLocation.z) << "\n";
			std::cout << "Listener location = " << std::to_string(listenerLocation.x) << ", " << std::to_string(listenerLocation.y) << ", " << std::to_string(listenerLocation.z) << "\n";
												
			std::cout << "Absortions = ";
			std::vector<std::vector<float>> wallsAbsortions;
			ISMParameters->room.GetAllWallsAbsortion(wallsAbsortions);
			for (int j = 0; j < NUM_BAND_ABSORTION; j++) {
			std::cout << wallsAbsortions.at(0).at(j) << ", ";
			}
			std::cout << std::endl << std::endl;
		}

		////////////////
		// Attributes
		////////////////
		mutable std::mutex mutex; // Thread management
		
		Common::CGlobalParameters globalParameters; // Get access to global render parameters

		bool setupDone;
		int reflectionOrder;								// Number of reflections t be simulated
		std::shared_ptr<CISMParameters> ISMParameters;		// Common parameters for the ISM simulation			
		Common::CVector3 sourceLocation;					// Location of the original source
		std::shared_ptr<CISMSourceImage> imageSources;		// Root of the image sources tree
		std::vector<Common::CVector3> imageSourcesPositionList; // List of image source locations
		std::vector<TImageSourceData> imageSourcesDataList; // List of image source data (location, visibility, reflection walls, etc)

		std::vector<Common::CWaveguide> listOfChannelSourceListener; // Waveguide processors
	};
}

#endif
/**
* \class CISMSourceImage
*
* \brief This class implements the ISM image source class.  
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


#ifndef _ISM_SOURCE_IMAGES_HPP_
#define _ISM_SOURCE_IMAGES_HPP_

#include <Common/Vector3.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/Room.hpp>
#include <Common/Wall.hpp>
#include "ISMParameters.hpp"


//#include <Common/CascadeGraphicEq9OctaveBands.h>

namespace BRTEnvironmentModel {			
	class CISMSourceImage {
	public:
		CISMSourceImage(std::shared_ptr<CISMParameters> _ISMParameters)
			: visible {false}
			, visibility{ 0 }
			, sourceLocation{ Common::CVector3(0,0,0) }
			/*, eq{ (float)_ISMParameters->sampleRate}*/
		{								
			ISMParameters = _ISMParameters;			
		}
		

		/** \brief Returns the location of the original source
		*   \param [out] Location: Current location for the original source.
		*/
		Common::CVector3 getImageLocation()		
		{
			return sourceLocation;
		}

		/** \brief Returns the first order reflections of the original source
		*   \param [out] Images: vector with the first order reflection images.
		*/
		/*std::vector<weak_ptr <SourceImages2>> getImages()
		{
			vector<weak_ptr<SourceImages2>> result;
			for (auto i = 0; i < imagesTree.size(); ++i) {
				result.push_back(weak_ptr<SourceImages2>(imagesTree[i]));
			}
			return result;
		}*/

		/** \brief Returns the locations of all images but the original source
		*   \details this method recurively goes through the image tree to collect all the image locations
		*   \param [out] imageSourceList: vector containing all image locations.
		*/
		void getImageLocations(std::vector<Common::CVector3>& imageSourceList)		
		{					
			for (auto & image : imagesTree) {
				if (image->getMyReflectionWall().IsActive()) {
					imageSourceList.push_back(image->getImageLocation());
					image->getImageLocations(imageSourceList);
				}
			}
			
		}
	
		/** \brief Returns data of all image sources
		*	\details This method returns the location of all image sources and wether they are visible or not, not including the
			original source (direct path).
		*	\param [out] imageSourceDataList: Vector containing the data of the image sources
		*/
		void getImageSourcesData(std::vector<TImageSourceData>& imageSourceDataList)		
		{			
			for (auto& image : imagesTree) {
				TImageSourceData temp;
				temp.location = image->getImageLocation();
				temp.reflectionWalls = image->reflectionWallsPath;
				temp.reflectionBands = image->reflectionBands;
				temp.visibility = image->visibility;
				temp.visible = image->visible;
				imageSourceDataList.push_back(temp);  //Once created, the image source data is added to the list
				image->getImageSourcesData(imageSourceDataList); //recurse to the next level
			}
		}
						
		/**
		 * @brief Creates all image sources up to a given order
		 * @details Creates a recursive tree of imagesources using all active walls up to the reflection order depth. This methos should be
					called every time the room geometry changes (walls are set as active or inactive), the reflection order changes
					or time parameters change (max distance, transition meters)
		 * @param _room Real (original) room geometry
		 * @param _order recursion depth
		 * @param _sourceLocation Current location of the real (original) source
		 */
		void createImagesTree(const Common::CRoom& _room, const int& _order, const Common::CVector3& _sourceLocation) {
			imagesTree.clear();
						
			std::vector<Common::CWall> path;			
			try {
				path.reserve(_order);
			}
			catch (const std::bad_alloc& e) {
				// Capture the exception if memory allocation fails
				// The operating system could not reserve the requested space				
			}			
			std::vector<float> absorptionCoefficients = std::vector<float>(NUM_BAND_ABSORTION, 1.0f);

			sourceLocation = _sourceLocation;
			createImagesTree(_room, _order, path, absorptionCoefficients);
			
			// TODO LIBERAR MEMORIA RESERVADA previamente EN PATH 
			// mediante std::vector<Wall>(path).swap(path);
			
			UpdateImagesTreeVisibilities();			
		}
						
		/** \brief updates imege source location, reflection and visibility
		*	\details Updates the recursive image source tree with the source locations and computes refelction coefficients and visibility
					 to be applied when process
		*/
		void UpdateSourceLocation(const Common::CVector3& _sourcePosition)
		{												
			for (auto& image : imagesTree) {
				image->SetSourceLocation(image->getMyReflectionWall().GetImagePoint(_sourcePosition));
			}
			UpdateImageVisibility();			
		}

		/**
		 * @brief Update visibility for all images in the tree
		 */
		void UpdateImagesTreeVisibilities() {
			for (auto& image : imagesTree) {
				image->UpdateImagesTreeVisibilities();
			}		
			UpdateImageVisibility();
		}		

		/*void UpdateImagesTreeWallsAbsorptionCoefficients() {						
			for (auto& image : imagesTree) {
				image->UpdateImagesTreeWallsAbsorptionCoefficients();
			}
			UpdateImageWallsAbsorptionCoefficients();
		}*/


		/** \brief Adds wall absortion to the sound
		*	\details Recursively process all source images providing an independent buffer for each of them with the original sound filtered
					 by the wall absortions. For non visible sources the output buffer contains zeros
		*	\param [in] inBuffer: original buffer used for all images
		*   \param [out] imageBuffers: vector of buffers with the sound filtered (One buffer per image)
		*	\param [in] listenerLocation: needed to know visibility of sources
		*/
		//void processAbsortion(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>>& imageBuffers, Common::CVector3 listenerLocation)
		//{
		//	for (int i = 0; i < imagesTree.size(); i++)  //process buffers for each of the image sources, adding the result to the output vector of buffers
		//	{

		//		CMonoBuffer<float> tempBuffer(inBuffer.size(), 0.0);

		//		if (imagesTree.at(i)->visibility > 0.00001)
		//			imagesTree.at(i)->eq.Process(inBuffer, tempBuffer);
		//		imageBuffers.push_back(tempBuffer);
		//		imagesTree.at(i)->processAbsortion(inBuffer, imageBuffers, listenerLocation);
		//	}
		//}

		void Reset() {
			imagesTree.clear();
			reflectionWallsPath.clear();
			reflectionBands.clear();
			sourceLocation = Common::CVector3(0, 0, 0);
			visibility = 1.0;
			visible = true;
			//eq = Common::CascadeGraphicEq9OctaveBands(ISMParameters->sampleRate);
		}

	private:

		/**
		 * @brief Check visibility through all reflection walls and compute a visibility coeficient
		 */
		void UpdateImageVisibility() {
			visibility = 1.0;
			visible = true;

			float distanceImageToListener = (ISMParameters->listenerPosition - sourceLocation).GetDistance();
			float upperBorder = ISMParameters->maxDistanceSourcesToListener + 0.5f * ISMParameters->transitionMeters;
			float lowerBorder = ISMParameters->maxDistanceSourcesToListener - 0.5f * ISMParameters->transitionMeters;

			if (distanceImageToListener > upperBorder)
			{
				visible = false;
				visibility = 0.0;
				return;
			}

			if (!reflectionWallsPath.empty()) {
				for (auto& wall : reflectionWallsPath) {
					float distanceToBorder;
					float wallVisibility;
					Common::CVector3 reflectionPoint = wall.GetIntersectionPointWithLine(sourceLocation, ISMParameters->listenerPosition);
					wall.CheckPointInsideWall(reflectionPoint, distanceToBorder, wallVisibility);
					visibility *= wallVisibility;
					visible &= (wallVisibility > 0);

					if (visibility == 0.0f) break; // It is possible to do this more robustly by using an epsilon comparison
				}
				visibility = std::pow(visibility, (1 / (float)reflectionWallsPath.size()));
			}

			if (distanceImageToListener > lowerBorder) {
				visibility *= 0.5f + 0.5f * std::cos(PI_F * (distanceImageToListener - lowerBorder) / ISMParameters->transitionMeters);
			}

		}

		void UpdateImageWallsAbsorptionCoefficients() {
			std::vector<float> absorptionCoefficients = std::vector<float>(NUM_BAND_ABSORTION, 1.0f);
			for (auto& wall : reflectionWallsPath) {
				AddCoefficientsFromWall(absorptionCoefficients, wall); // Calculate EQ filters 
			}
			reflectionBands.assign(absorptionCoefficients.begin(), absorptionCoefficients.end());
			//eq.SetCommandGains(ISMParameters->sampleRate, absorptionCoefficients);
		}


		/** \brief changes the location of the original source
		*	\details Sets a new location for the original source and updates all images accordingly.
		*   \param [in] _location: new location for the original source.
		*/
		void SetSourceLocation(const Common::CVector3& _sourceLocation)
		{			
			sourceLocation = _sourceLocation;
			UpdateSourceLocation(_sourceLocation);
		}



		void createImagesTree(const Common::CRoom & _currentRoom, int order, std::vector<Common::CWall> & path, std::vector<float> & absorptionCoefficients) {
			if (order == 0) return;
			
			const auto& wallsList = _currentRoom.GetWalls();
			for (auto& wall : wallsList) {
				
				if (!wall.IsActive()) continue;

				const auto newImageLocation = wall.GetImagePoint(sourceLocation);
				Common::CVector3 realRoomCenter = ISMParameters->room.GetCenter();

				// Filter out reflections that are not physically possible				
				/* If the image is closer to the room center than the previous original, 
				/  that reflection is not real and should not be included.
				/  this is equivalent to determine wether source and room center are on 
				/  the same side of the wall or not.
				*/
				float distanceRoomRealSource = (realRoomCenter - sourceLocation).GetSqrDistance();
				float distanceRoomImageSource = (realRoomCenter - newImageLocation).GetSqrDistance();			
				if (Common::is_greater_or_equal(distanceRoomRealSource, distanceRoomImageSource)) continue;
			
				// Filter out reflections that are too far away
				/* If the image is in a room that is too far away, do not create it.
				/  the distance criterion can be static or dynamic
				*/
				float roomsDistance = CalculateRoomsDistance(wall, path, newImageLocation, ISMParameters->listenerPosition);				
				float maxDistanceImageSources = CalculateMaxDistanceImageSources();				
				if (Common::is_greater(roomsDistance, maxDistanceImageSources)) continue;
				
				path.push_back(wall);
											
				auto child = std::make_shared<CISMSourceImage>(ISMParameters);
				child->sourceLocation = newImageLocation;				
				child->reflectionWallsPath.assign(path.begin(), path.end());
				
				AddCoefficientsFromWall(absorptionCoefficients, wall); // Calculate EQ
				child->reflectionBands.assign(absorptionCoefficients.begin(), absorptionCoefficients.end());				
				//child->eq.SetCommandGains(ISMParameters->sampleRate, absorptionCoefficients);

				if (order > 1) {
					Common::CRoom nextRoom;
					for (auto& wj : wallsList)
						nextRoom.InsertWall(wall.GetImageWall(wj));
					child->createImagesTree(nextRoom, order - 1, path, absorptionCoefficients);
				}				

				imagesTree.push_back(std::move(child));
				path.pop_back();
				RemoveCoefficientsFromWall(absorptionCoefficients, wall);				
			}
		}
				
		void AddCoefficientsFromWall(std::vector<float>& absorptionCoefficients, const Common::CWall& wall) {
			for (int n = 0; n < absorptionCoefficients.size(); n++) {
				absorptionCoefficients[n] *= std::sqrt(1 - wall.GetAbsortionBand().at(n));
			}			
		}
		void RemoveCoefficientsFromWall(std::vector<float> & absorptionCoefficients, const Common::CWall & wall) {
			for (int n = 0; n < absorptionCoefficients.size(); n++) {
				absorptionCoefficients[n] /= std::sqrt(1 - wall.GetAbsortionBand().at(n));
			}
		}

		float CalculateRoomsDistance(const Common::CWall & wall, const std::vector<Common::CWall> & path, const Common::CVector3 & imgPos, const Common::CVector3 & listenerPosition) {
			float roomsDistance = 0.0;
			if (path.size() > 0)
			{
				// the distance criterion can be static or dynamic
				if (ISMParameters->staticDistanceCriterion == false)
					roomsDistance = wall.GetMinimumDistanceFromWall(path.front());
				else					
					roomsDistance = (listenerPosition - imgPos).GetDistance();
			}
			return roomsDistance;
		}

		float CalculateMaxDistanceImageSources() {
			// the distance criterion can be static or dynamic
			float maxDistanceImageSources;
			if (ISMParameters->staticDistanceCriterion == false)
				maxDistanceImageSources = ISMParameters->maxDistanceSourcesToListener;
			else
				maxDistanceImageSources = ISMParameters->maxDistanceSourcesToListener + ISMParameters->transitionMeters * 0.5;
			return maxDistanceImageSources;
		}



		/** \brief Returns the  wall where the reflecion produced this image
		*   \param [out] Reflection wall.
		*/
		Common::CWall getMyReflectionWall()
		{
			return reflectionWallsPath.back();
		}


		/*bool is_greater(float a, float b) const {					
			return (a - b) > epsilon;
		}

		bool is_greater_or_equal(float a, float b) const {						
			return (a - b) > -epsilon;
		}*/
	
		////////////////
		/// Attributes
		////////////////											
		bool visible;									// false when visibility = 0, true otherwise
		float visibility;								// 1.0 if visible, 0.0 if not, something in the middle if the ray is close to the border of walls
		Common::CVector3 sourceLocation;				// Source location		
		std::vector<Common::CWall> reflectionWallsPath;			// vector containing the walls where the sound has been reflected in inverse order (last reflection first)
		std::vector<float> reflectionBands;				// coeficients, for each octave Band, to be applied to simulate walls' absortion
		std::vector<std::shared_ptr<CISMSourceImage>> imagesTree;	// recursive list of images			
		//Common::CascadeGraphicEq9OctaveBands eq;		// Filter to simulate walls' absortion		

		std::shared_ptr<CISMParameters> ISMParameters;	// To access ISM parameters
		

};
}
#endif 
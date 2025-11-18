/**
* \class COBJReader
*
* \brief Declaration of CSOFAReader class
* \date	October 2025
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

#ifndef _OBJ_READER_
#define _OBJ_READER_

#include <string>
#include <memory>
#include <ServiceModules/Room.hpp>
#include <third_party_libraries/rapidobj/include/rapidobj/rapidobj.hpp>

namespace BRTReaders {
	class COBJReader {

		using TFace = std::vector<int>;
		using TFaceList = std::vector<TFace>;
		using TVertexList = std::vector<Common::CVector3>;

	public:
		COBJReader() 
		: errorDescription { "No error." }
		{}
		~COBJReader() = default;

		/**
		 * @brief Get the last action error description, if any
		 * @return error description
		 */
		std::string GetLastError() {
			std::string _errorDescription = errorDescription;
			ResetError();
			return _errorDescription;
		}


		bool ReadOBJFile(const std::string & filePath, std::shared_ptr<BRTServices::CRoom> room) { 
			
			// Open file
			rapidobj::Result objLoaded = rapidobj::ParseFile(filePath);
			// Check to see if it loaded
			if (objLoaded.error) {
				errorDescription = "Error reading OBJ file - " + filePath;
				errorDescription += objLoaded.error.code.message();
				return false;
			}
			// Load Data
			return ReadFromOBJLDataType(objLoaded, room);												
		}	
		
		
	private:

		bool ReadFromOBJLDataType(const rapidobj::Result & objData, std::shared_ptr<BRTServices::CRoom> & room) {
			
			// Check if it have at least one mesh
			if (objData.shapes.size() == 0) {
				errorDescription = "The OBJ file does not contain any mesh.";
				return false;
			}
			if (objData.shapes.size() > 1) {
				//Warn that only the first mesh will be used
			}
			
			// Get vertex positions from the first mesh
			TVertexList vertexList;
			GetVertexPositions(objData.attributes, vertexList);
			if (vertexList.size() == 0) {
				errorDescription = "The OBJ file does not contain any vertex position.";
				return false;
			}
			// Store corners into the ROOM
			BRTServices::TRoomGeometry roomGeometry;
			for (auto & oneVertex : vertexList) {
				roomGeometry.corners.push_back(oneVertex);
			}

			// Get Walls
			TFaceList faceList;
			bool result = GetFaces(objData.shapes[0].mesh, faceList);
			if (!result) {
				errorDescription = "Error getting faces from the mesh.";
				return false;
			}
			
			// Store walls into the ROOM from the vertex positions
			for (auto & face : faceList) {				
				roomGeometry.walls.push_back(face);
			}
			
			room->SetupRoomGeometry(roomGeometry);
			
			// Get materials and assign to walls if needed (not implemented yet)
			if (objData.materials.size() != 0) {				
				SetAbsortionCoefficients(objData, room);			
			} else {
				// Warn that no materials were found
				SET_RESULT(RESULT_WARNING, "No materials found in the OBJ file.");
			}
			return true;
		}
		
		/**
		 * @brief Get the vertex positions from the mesh
		 * @param curMesh mesh to extract the vertex positions from
		 * @param vertexPositions list to store the vertex positions
		 */
		void GetVertexPositions(const rapidobj::Attributes & objAttributes, std::vector<Common::CVector3> & vertexPositions) {
			vertexPositions.clear();

			for (int i = 0; i < objAttributes.positions.size(); i = i + 3) {
				Common::CVector3 tempV = Common::CVector3(
					objAttributes.positions[i], objAttributes.positions[i + 1], objAttributes.positions[i + 2]);
				vertexPositions.push_back(tempV);
			}
		}

		/**
		 * @brief Get the faces from the mesh
		 * @param objMesh mesh to extract the faces from
		 * @param faceList list to store the faces
		 * @return true if successful, false otherwise
		 */ 
		bool GetFaces(const rapidobj::Mesh &objMesh, TFaceList & faceList) {
			bool result = true;
			int indicesPosition = 0;
			for (auto & faceVertices : objMesh.num_face_vertices) {
				std::vector<int> wallIndices;
				result &= GetWallVertexIndices(objMesh, faceVertices, indicesPosition, wallIndices);
				indicesPosition += faceVertices;
				faceList.push_back(wallIndices);
			}
			return result;
		}

		/**
		 * @brief Get the vertex indices for a wall
		 * @param objMesh 
		 * @param numberOfVertices 
		 * @param position 
		 * @param wallIndices 
		 * @return 
		 */
		bool GetWallVertexIndices(const rapidobj::Mesh & objMesh, int numberOfVertices, int position, std::vector<int> & wallIndices) {

			if ((numberOfVertices + position) > objMesh.indices.size()) return false;

			for (int i = position; i < position + numberOfVertices; i++) {
				rapidobj::Index index = objMesh.indices[i];
				wallIndices.push_back(index.position_index);
			}
			return true;
		}

		void SetAbsortionCoefficients(const rapidobj::Result & objData, std::shared_ptr<BRTServices::CRoom> & room) {
			
			int numberOfFaces = objData.shapes[0].mesh.num_face_vertices.size();
			for (int faceIndex = 0; faceIndex < numberOfFaces; faceIndex++) {
				int materialID = objData.shapes[0].mesh.material_ids[faceIndex];
				
				if (materialID == -1) continue; // No material assigned to this face
				auto& coefs = objData.materials[materialID].acoustic_coeffs;		
				room->SetWallAbsortion(faceIndex, std::vector<float>(coefs.begin(), coefs.end()));	
			}
		}


		/**
		 * @brief Clear the error description
		 */
		void ResetError() {
			errorDescription = "No error.";
		}
		
		
		////////////////
		// Attributes
		////////////////
		std::string errorDescription;
	};
}
#endif
/**
* \class CHRTFTester
*
* \brief Declaration of CSRTF class interface to store directivity data
* \version
* \date	May 2023
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

#include <ServiceModules/HRTF.hpp>
#include <sstream> // for std::ostringstream

#ifndef _CHRTF_TESTER_HPP_
#define _CHRTF_TESTER_HPP_

namespace BRTServices
{
	class CHRTFTester {
	public:
		CHRTFTester() {};

		/**
		 * @brief Print to .csv file all orientations of the Grid resampled.
		 * @param _hrtf 
		*/
		void TestGrid(std::shared_ptr<BRTServices::CHRTF> _hrtf) {

			// QuasiUniformSphereDistibution object thanks to friend class
			BRTServices::CQuasiUniformSphereDistribution  _gridQuasiUniform;

			// Cause we call the Reader of the SOFA without the process, now we can call only the method that create the grid in which later will be emplace the data
			_gridQuasiUniform.CreateGrid(_hrtf->t_HRTF_Resampled_partitioned, _hrtf->stepVector, _hrtf->gridSamplingStep);

			//std::cout << _hrtf->GetFilename() << std::endl;
			//std::cout << _hrtf->t_HRTF_DataBase.size() << std::endl;

			// Once created the table with the orientations of HRTF Grid, we print to a .csv file

			std::fstream gridFile;
			std::string filename = "GridTest.csv";
			gridFile.open(filename, std::ios::out);
			gridFile << "Azimuth;Elevation\n";
			for (auto& orientationToSave : _hrtf->t_HRTF_Resampled_partitioned) 
			{
				//std::cout << "Azimuth: " << orientationToSave.azimuth << "Elevation: " << orientationToSave.elevation << std::endl;
				//gridFile << orientationToSave.first.azimuth << ";" << orientationToSave.first.elevation << "\n";

				// Emplacing the decimal point to decimal comma to improve the reading from other apps like Excel.
				std::ostringstream azimuthStream;
				azimuthStream << std::fixed << orientationToSave.first.azimuth;
				std::string azimuthString = azimuthStream.str();
				std::replace(azimuthString.begin(), azimuthString.end(), '.', ',');

				std::ostringstream elevationStream;
				elevationStream << std::fixed << orientationToSave.first.elevation;
				std::string elevationString = elevationStream.str();
				std::replace(elevationString.begin(), elevationString.end(), '.', ',');

				gridFile << azimuthString << ";" << elevationString << "\n";
			}
			gridFile.close();
			std::cout << "File created: " + filename << std::endl;
		}

		/**
		 * @brief Prints to console the number of HRIRs interpolated to check if a SOFA already interpolated needs to be interpolate
		 * @param _hrtf 
		*/
		void TestGridInterpolation(std::shared_ptr<BRTServices::CHRTF> _hrtf)
		{
			// QuasiUniformSphereDistibution object thanks to friend class
			BRTServices::CQuasiUniformSphereDistribution  _gridQuasiUniform;

			// Call to HRTF CalculateListOfOrientations_T_HRTF_DataBase to make the list that needs later FillResampledTable
			_hrtf->CalculateListOfOrientations_T_HRTF_DataBase();

			// Cause we call the Reader of the SOFA without the process, now we can call only the method that create the grid in which later will be emplace the data
			_gridQuasiUniform.CreateGrid(_hrtf->t_HRTF_Resampled_partitioned, _hrtf->stepVector, _hrtf->gridSamplingStep);

			// Call to HRTF FillResampledTable to check if it occurs any interpolation with a Grid already interpolated out the app.
			_hrtf->FillResampledTable();

		}

	};
}
#endif
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
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence:
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
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

		std::vector<orientation> TestGrid(std::shared_ptr<BRTServices::CHRTF> _hrtf) {
			/// Testing the access to HRTF class

			BRTServices::CQuasiUniformSphereDistribution  _gridQuasiUniform;

			_hrtf->CalculateHRIR_InPoles(_hrtf->resamplingStep);
			_hrtf->FillOutTableOfAzimuth360(_hrtf->resamplingStep);
			_hrtf->FillSphericalCap_HRTF(_hrtf->gapThreshold, _hrtf->resamplingStep);

			_gridQuasiUniform.CreateGrid(_hrtf->t_HRTF_Resampled_partitioned, _hrtf->stepVector, _hrtf->resamplingStep);

			std::cout << _hrtf->GetFilename() << std::endl;
			std::cout << _hrtf->t_HRTF_DataBase.size() << std::endl;

			std::fstream gridFile;
			gridFile.open("GridTest.csv", std::ios::out);
			gridFile << "Azimuth;Elevation\n";
			for (auto& orientationToSave : _hrtf->t_HRTF_Resampled_partitioned) 
			{
				//std::cout << "Azimuth: " << orientationToSave.azimuth << "Elevation: " << orientationToSave.elevation << std::endl;
				//gridFile << orientationToSave.first.azimuth << ";" << orientationToSave.first.elevation << "\n";

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
			
			std::vector<orientation> temp;
			for (auto position : _hrtf->t_HRTF_DataBase) {
				temp.push_back(position.first);
			}
			return temp;
		}

		void TestGridInterpolation(std::shared_ptr<BRTServices::CHRTF> _hrtf)
		{
			BRTServices::CQuasiUniformSphereDistribution  _gridQuasiUniform;



			_hrtf->CalculateListOfOrientations_T_HRTF_DataBase();
			_gridQuasiUniform.CreateGrid(_hrtf->t_HRTF_Resampled_partitioned, _hrtf->stepVector, _hrtf->resamplingStep);
			_hrtf->FillResampledTable();

		}

	};
}
#endif
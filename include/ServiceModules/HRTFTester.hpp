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

#ifndef _CHRTF_TESTER_HPP_
#define _CHRTF_TESTER_HPP_

namespace BRTServices
{
	class CHRTFTester {
	public:
		CHRTFTester() {};

		void TestGrid(std::shared_ptr<BRTServices::CHRTF> _hrtf) {
			/// Testing the access to HRTF class

			// Justa and example
			std::cout << _hrtf->GetFilename() << std::endl;
			std::cout << _hrtf->t_HRTF_DataBase.size() << std::endl;
		}

	};
}
#endif
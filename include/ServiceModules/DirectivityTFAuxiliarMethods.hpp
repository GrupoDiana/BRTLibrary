/**
* \class CDirectivityTFAuxiliarMethods
*
* \brief Declaration of CDirectivityTFAuxiliarMethods class 
* \date	July 2023
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
* \b Copyright: University of Malaga 2023. Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CDIRECTIVITYTF_AUXILIARMETHODS_HPP
#define _CDIRECTIVITYTF_AUXILIARMETHODS_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <list>
#include <cstdint>
#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>
#include <Common/CommonDefinitions.hpp>
#include <Common/GlobalParameters.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/DirectivityTFDefinitions.hpp>

namespace BRTServices
{

	/** \brief Auxiliary methods used in different classes working with HRTFs
*/
	class CDirectivityTFAuxiliarMethods {
	public:

		struct CalculateDirectivityTFFromHemisphereParts {
			//static THRIRStruct CalculateHRIRFromHemisphereParts(T_HRTFTable& _t_HRTF_DataBase, int _HRIRLength, std::vector < std::vector <orientation>> _hemisphereParts) {
			BRTServices::TDirectivityTFStruct operator () (T_DirectivityTFTable& _t_DirectivityTF_DataBase, int _DirectivityTFLength, std::vector < std::vector <orientation>> _hemisphereParts) {

				BRTServices::TDirectivityTFStruct calculatedDirectivityTF;

				//Calculate the DirectivityTF of each hemisphere part
				std::vector< BRTServices::TDirectivityTFStruct> newDirectivityTF;
				newDirectivityTF.resize(_hemisphereParts.size());

				for (int q = 0; q < _hemisphereParts.size(); q++)
				{
					newDirectivityTF[q].realPart.resize(_DirectivityTFLength, 0.0f);
					newDirectivityTF[q].imagPart.resize(_DirectivityTFLength, 0.0f);

					float scaleFactor;
					if (_hemisphereParts[q].size() != 0) {
						scaleFactor = 1.0f / _hemisphereParts[q].size();
					}
					else { scaleFactor = 0.0f; }

					for (auto it = _hemisphereParts[q].begin(); it != _hemisphereParts[q].end(); it++)
					{
						auto itDirectivityTF = _t_DirectivityTF_DataBase.find(orientation(it->azimuth, it->elevation));

						//Get the DirectivityTF
						for (int i = 0; i < _DirectivityTFLength; i++) {
							newDirectivityTF[q].realPart[i] = (newDirectivityTF[q].realPart[i] + itDirectivityTF->second.realPart[i] * scaleFactor);
							newDirectivityTF[q].imagPart[i] = (newDirectivityTF[q].imagPart[i] + itDirectivityTF->second.imagPart[i] * scaleFactor);
						}
					}
				}

				//Get the FINAL values
				float scaleFactor_final = 1.0f / _hemisphereParts.size();

				//calculate Final HRIR
				calculatedDirectivityTF.realPart.resize(_DirectivityTFLength, 0.0f);
				calculatedDirectivityTF.imagPart.resize(_DirectivityTFLength, 0.0f);

				for (int i = 0; i < _DirectivityTFLength; i++)
				{
					for (int q = 0; q < _hemisphereParts.size(); q++)
					{
						calculatedDirectivityTF.realPart[i] = calculatedDirectivityTF.realPart[i] + newDirectivityTF[q].realPart[i] * scaleFactor_final;
						calculatedDirectivityTF.imagPart[i] = calculatedDirectivityTF.imagPart[i] + newDirectivityTF[q].imagPart[i] * scaleFactor_final;
					}
				}
				return calculatedDirectivityTF;
			}
		};
	};

}


#endif
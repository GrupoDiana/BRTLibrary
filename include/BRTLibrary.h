/**
*
* \brief Includes of BRT LIBRARY
* \date	June 2023
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
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _BRT_LIBRARY_
#define _BRT_LIBRARY_

#include "Common/CommonDefinitions.hpp"
#include "Common/GlobalParameters.hpp"
#include "Common/ErrorHandler.hpp"
#include "Base/BRTManager.hpp"
#include "Base/ListenerModelBase.hpp"
#include "ListenerModels/ListenerHRTFbasedModel.hpp"
#include "SourceModels/SourceSimpleModel.hpp"
#include "SourceModels/SourceDirectivityModel.hpp"
#include "ProcessingModules/SingleProcessor.hpp"
#include "ProcessingModules/SingleBinauralProcessor.hpp"
#include "ProcessingModules/DistanceAttenuationProcessor.hpp"
#include "ProcessingModules//HRTFConvolverProcessor.hpp"
#include "ProcessingModules/DirectivityTFConvolver.hpp"
#include "ServiceModules/HRTF.hpp"
#include "ServiceModules/ILD.hpp"
#include "ServiceModules/DirectivityTF.hpp"
#include "Readers/SofaReader.hpp"
#include "third_party_libraries/nlohmann/json.hpp"


#endif
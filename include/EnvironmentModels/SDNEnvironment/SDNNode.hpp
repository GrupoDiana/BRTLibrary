/**
* \class SDNNode
*
* \brief   		   
* \date Sep 2023
* 
* \authors  Developer's team (University of Milan), in alphabetical order: F. Avanzini, D. Fantini , M. Fontana, G. Presti,
* Coordinated by F. Avanzini (University of Milan) ||
*
* \b Contact: federico.avanzini@unimi.it
*
* \b Copyright: University of Milan - 2023
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM (https://www.sonicom.eu/) ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
*/
#ifndef _SDN_NODE_HPP_
#define _SDN_NODE_HPP_


#include <Common/Buffer.hpp>

class SDNNode
{
public:

	SDNNode() {};

	void SetPosition(Common::CVector3 newPos) { position = newPos; };
	void SetX(float newPos) { position.x = newPos; };
	void SetY(float newPos) { position.y = newPos; };
	void SetZ(float newPos) { position.z = newPos; };
	Common::CVector3& GetPosition() { return position; };

	~SDNNode() {};

private:

	Common::CVector3 position = { 0, 0, 0 };

};

#endif
#ifndef _SDN_NODE_HPP_
#define _SDN_NODE_HPP_


#include <Common/Buffer.hpp>

class SDNNode
{
public:

	SDNNode() {};

	void setPosition(Common::CVector3 newPos) { position = newPos; };
	void setX(float newPos) { position.x = newPos; };
	void setY(float newPos) { position.y = newPos; };
	void setZ(float newPos) { position.z = newPos; };
	Common::CVector3& getPosition() { return position; };

	~SDNNode() {};

private:

	Common::CVector3 position = { 0, 0, 0 };

};

#endif
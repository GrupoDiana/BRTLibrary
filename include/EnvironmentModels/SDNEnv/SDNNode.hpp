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
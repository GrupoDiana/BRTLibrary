#ifndef _CEARS_TRANSFORM_H_
#define _CEARS_TRANSFORM_H_

#include <Common/Transform.h>
#include <Common/Vector3.h>

namespace Common {
	//
	/* \brief Class declared to share ear location information
	*
	*/
	struct CEarsTransforms {
	public:
		CTransform	leftEarTransform;
		CTransform	rightEarTransform;
		CVector3	leftEarLocalPosition;
		CVector3	rightEarLocalPosition;
	};
}

#endif

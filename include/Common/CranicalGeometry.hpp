/**
* \class CCranialGeometry
*
* \brief Declaration of CCranialGeometry
* \date	May 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/


#ifndef _CCRANICAL_GEOMETRY_HPP_
#define _CCRANICAL_GEOMETRY_HPP_

#include <Common/Vector3.hpp>

namespace Common {

	class CCranialGeometry {
	public:
		CCranialGeometry() : headRadius{ -1 }, leftEarLocalPosition{ Common::CVector3() }, rightEarLocalPosition{ Common::CVector3() } { }
		CCranialGeometry(float _headRadius) : headRadius{ _headRadius }, leftEarLocalPosition{ Common::CVector3() }, rightEarLocalPosition{ Common::CVector3() } { }
		CCranialGeometry(float _headRadius, Common::CVector3 _leftEarLocalPosition, Common::CVector3 _rightEarLocalPosition) : headRadius{ _headRadius }, leftEarLocalPosition{ _leftEarLocalPosition }, rightEarLocalPosition{ _rightEarLocalPosition } { }

		float GetHeadRadius() { return headRadius; }
		Common::CVector3 GetLeftEarLocalPosition() { return leftEarLocalPosition; }
		Common::CVector3 GetRightEarLocalPosition() { return rightEarLocalPosition; }

		/**
		 * @brief Set the radius of the listener head. A new ear position is calculated
		 * @param _headRadius head radius in meters
		 */
		void SetHeadRadius(float _headRadius) {
			headRadius = _headRadius;
			CalculateEarLocalPositionFromHeadRadius();
		}
		/**
		 * @brief Set the relative position of one ear (to the listener head center). A new head radius is calculated
		 * @param _earPosition ear local position
		 */
		void SetLeftEarPosition(Common::CVector3 _earPosition) {
			leftEarLocalPosition = _earPosition;
			CalculateHeadRadiusFromEarPosition();	// Update the head radius			
		}

		/**
		 * @brief Set the relative position of one ear (to the listener head center). A new head radius is calculated
		 * @param _earPosition ear local position
		 */
		void SetRightEarPosition(Common::CVector3 _earPosition) {
			rightEarLocalPosition = _earPosition;
			CalculateHeadRadiusFromEarPosition();	// Update the head radius			
		}
		

	private:
		float headRadius;								// Head radius of listener 
		Common::CVector3 leftEarLocalPosition;			// Listener left ear relative position
		Common::CVector3 rightEarLocalPosition;			// Listener right ear relative position
		Common::CGlobalParameters globalParameters;		// Access to defined global parameters

		/**
		* @brief Calculate head radius from the listener ear positions
		* @return new head radius
		*/
		void CalculateHeadRadiusFromEarPosition() {
			headRadius = (0.5f * (leftEarLocalPosition.GetDistance() + rightEarLocalPosition.GetDistance()));
		}

		/** \brief	Calculate the relative position of one ear taking into account the listener head radius
		*	\param [in]	_ear			ear type
		*   \return  Ear local position in meters
		*   \eh <<Error not allowed>> is reported to error handler
		*/
		void CalculateEarLocalPositionFromHeadRadius() {
			leftEarLocalPosition.SetAxis(RIGHT_AXIS, -headRadius);
			rightEarLocalPosition.SetAxis(RIGHT_AXIS, headRadius);
		}

					
		
	};
}

#endif
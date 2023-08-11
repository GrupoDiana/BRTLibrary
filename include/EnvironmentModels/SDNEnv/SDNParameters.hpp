#ifndef _SDN_PARAMETERS_HPP_
#define _SDN_PARAMETERS_HPP_

namespace SDNParameters
{
	constexpr auto SOUND_SPEED = 343;
	constexpr char axishelper[6] = { 'x', 'x', 'y', 'y', 'z', 'z' };
	constexpr int NUM_WALLS = 6;
	constexpr int NUM_FREQ = 8;
	constexpr int NUM_WAVEGUIDES_TO_OUTPUT = 7;
	constexpr float ROOM_MAX_DIMENSION = 100.0f;
	constexpr float SMOOTHING_TIME_SECONDS = 0.015f;
}

#endif
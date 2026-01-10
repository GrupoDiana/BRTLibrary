/**
* \class KDTree
*
* \brief Declaration of KDTree class
* \date	Dec 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#pragma once
#ifndef _KD_TREE_HPP
#define _KD_TREE_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

/*
    KDTree.hpp (header-only)

    Balanced 3D KD-tree for nearest-neighbor search on a sphere.

    - Converts (azimuth, elevation) to a 3D unit direction vector.
    - Nearest neighbor search minimizes angular distance on the sphere.
      For unit vectors, minimizing angular distance is equivalent to
      minimizing Euclidean distance (monotonic relationship).

    Typical usage:

        KDTreeHRTF::KDTree<orientation> kd;
        kd.build(orientationsVector);          // once (e.g., EndSetup)

        orientation nearestOri = kd.nearest(az, el); // runtime

    Requirements on OrientationT:
        double azimuth;
        double elevation;
        (distance field is ignored here)

    IMPORTANT:
        - azimuth must be normalized to [0, 360)
        - elevation must follow your convention:
            [0, 90] U [270, 360)
          (so 350° is close to 10° through wrap-around)
*/

namespace BRTServices {
template <class OrientationT> class KDTree {
public:
	KDTree() = default;

	// Build the KD-tree from a vector of orientations (internal copy).
	void build(const std::vector<OrientationT> & orientations) {
		m_orientations = orientations;
		rebuildDirectionsAndTree();
	}

	// Build the KD-tree from rvalue orientations (move in, no extra copy).
	void build(std::vector<OrientationT> && orientations) {
		m_orientations = std::move(orientations);
		rebuildDirectionsAndTree();
	}

	// Return the nearest orientation for (azimuth, elevation).
	// Inputs must be normalized using your convention.
	OrientationT nearest(double azimuth0_360, double elevation0_360) const {
		if (!m_root || m_directions.empty())
			return OrientationT(azimuth0_360, elevation0_360);

		const auto queryDir = sphericalToUnitVector(azimuth0_360, elevation0_360);

		int bestIndex = -1;
		float bestDistance2 = std::numeric_limits<float>::infinity();
		nearestSearch(m_root.get(), queryDir, bestIndex, bestDistance2);

		if (bestIndex >= 0)
			return m_orientations[bestIndex];

		// Extremely unlikely fallback
		return OrientationT(azimuth0_360, elevation0_360);
	}

	// Convenience accessors
	size_t size() const noexcept { return m_orientations.size(); }
	bool empty() const noexcept { return m_orientations.empty(); }

	void clear() {
		m_orientations.clear();
		m_directions.clear();
		m_root.reset();
	}

private:
	struct Node {
		int index = -1; // Index into m_orientations/m_directions
		int axis = 0; // Split axis: 0=x, 1=y, 2=z
		float splitValue = 0.f; // Coordinate value on split axis
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;
	};

	std::vector<OrientationT> m_orientations;
	std::vector<std::array<float, 3>> m_directions; // Unit vectors per orientation
	std::unique_ptr<Node> m_root;

private:
	static inline double degToRad(double degrees) {
		return degrees * 3.14159265358979323846 / 180.0;
	}

	// Convert elevation from your wrapped convention to signed range [-90, 90].
	// elevation in [0, 90] U [270, 360)
	static inline double toSignedElevation(double elevation0_360) {
		if (elevation0_360 >= 270.0)
			return elevation0_360 - 360.0; // -> [-90, 0)
		return elevation0_360; // -> [0, 90]
	}

	// (azimuth, elevation) -> 3D unit vector
	static inline std::array<float, 3> sphericalToUnitVector(double azimuth0_360,
		double elevation0_360) {
		const double az = degToRad(azimuth0_360);
		const double el = degToRad(toSignedElevation(elevation0_360));

		const double cosEl = std::cos(el);
		return {
			static_cast<float>(cosEl * std::cos(az)),
			static_cast<float>(cosEl * std::sin(az)),
			static_cast<float>(std::sin(el))
		};
	}

	static inline float squaredDistance(const std::array<float, 3> & a,
		const std::array<float, 3> & b) {
		const float dx = a[0] - b[0];
		const float dy = a[1] - b[1];
		const float dz = a[2] - b[2];
		return dx * dx + dy * dy + dz * dz;
	}

	void rebuildDirectionsAndTree() {
		m_directions.clear();
		m_directions.reserve(m_orientations.size());

		for (const auto & ori : m_orientations)
			m_directions.push_back(sphericalToUnitVector(ori.azimuth, ori.elevation));

		std::vector<int> indices(m_directions.size());
		std::iota(indices.begin(), indices.end(), 0);

		m_root = buildRecursive(indices, 0);
	}

	std::unique_ptr<Node> buildRecursive(std::vector<int> & indices, int depth) {
		if (indices.empty())
			return nullptr;

		const int axis = depth % 3;
		const size_t mid = indices.size() / 2;

		auto axisComparator = [&](int a, int b) {
			return m_directions[a][axis] < m_directions[b][axis];
		};
		std::nth_element(indices.begin(), indices.begin() + mid, indices.end(), axisComparator);

		const int currentIndex = indices[mid];

		std::vector<int> leftIndices(indices.begin(), indices.begin() + mid);
		std::vector<int> rightIndices(indices.begin() + mid + 1, indices.end());

		auto node = std::make_unique<Node>();
		node->index = currentIndex;
		node->axis = axis;
		node->splitValue = m_directions[currentIndex][axis];
		node->left = buildRecursive(leftIndices, depth + 1);
		node->right = buildRecursive(rightIndices, depth + 1);

		return node;
	}

	void nearestSearch(const Node * node,
		const std::array<float, 3> & queryDir,
		int & bestIndex,
		float & bestDistance2) const {
		if (!node)
			return;

		const int nodeIndex = node->index;

		const float d2 = squaredDistance(m_directions[nodeIndex], queryDir);
		if (d2 < bestDistance2) {
			bestDistance2 = d2;
			bestIndex = nodeIndex;
		}

		const int axis = node->axis;
		const float diff = queryDir[axis] - node->splitValue;

		const Node * nearChild = (diff < 0.f) ? node->left.get() : node->right.get();
		const Node * farChild = (diff < 0.f) ? node->right.get() : node->left.get();

		nearestSearch(nearChild, queryDir, bestIndex, bestDistance2);

		// Check if the hypersphere crosses the splitting plane.
		if (diff * diff < bestDistance2)
			nearestSearch(farChild, queryDir, bestIndex, bestDistance2);
	}
};
} // namespace

#endif 

//
// Created by adamyuan on 10/29/17.
//

#ifndef PROBLEMA_DRONEGROUP_HPP
#define PROBLEMA_DRONEGROUP_HPP

#include <glm/glm.hpp>
#include <vector>
#include <MyGL/FrameRate.hpp>

#define DRONE_SPEED 0.3f
class DroneGroup
{
private:
	std::vector<glm::vec3> Drones;
	std::vector<glm::vec3> Destinations;
	std::vector<glm::vec3> Directions;
	std::vector<bool> Arrived;
	float SecondsDelay, LastTime;
	int LaunchedSize;
public:
	DroneGroup() = default;
	explicit DroneGroup(const std::vector<glm::vec3> &initialPosition);
	void Init(const std::vector<glm::vec3> &initialPosition);
	bool AllArrived() const;
	const std::vector<glm::vec3> &GetDronePositions() const;
	void SetDestinations(const std::vector<glm::vec3> &destinations, float secondsBetween = 0.0f);
	void NextTick(const MyGL::FrameRateManager &FPSManager);
};


#endif //PROBLEMA_DRONEGROUP_HPP

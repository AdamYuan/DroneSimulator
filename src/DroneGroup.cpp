//
// Created by adamyuan on 10/29/17.
//

#include "DroneGroup.hpp"
#include <GLFW/glfw3.h>

DroneGroup::DroneGroup(const std::vector<glm::vec3> &initialPosition) {
	Init(initialPosition);
}

void DroneGroup::Init(const std::vector<glm::vec3> &initialPosition) {
	Drones = initialPosition;
	Arrived = std::move(std::vector<bool>(Drones.size(), true));
	Destinations.reserve(Drones.size());
	Directions.resize(Drones.size());
}

void DroneGroup::SetDestinations(const std::vector<glm::vec3> &destinations, float secondsBetween)
{
	if(!AllArrived())
		return;
	Destinations = destinations;
	Arrived = std::move(std::vector<bool>(Drones.size(), false));
	for(size_t i=0; i<Drones.size(); ++i)
		Directions[i] = glm::normalize(Destinations[i] - Drones[i]);

	SecondsDelay = secondsBetween;
	LastTime = static_cast<float>(glfwGetTime());
	LaunchedSize = 0;
}

bool DroneGroup::AllArrived() const {
	for (bool i : Arrived)
		if(!i)
			return false;
	return true;
}

void DroneGroup::NextTick(const MyGL::FrameRateManager &FPSManager){
	float dist = FPSManager.GetMovementDistance(DRONE_SPEED);
	for(auto i = static_cast<size_t>(LaunchedSize); i < Drones.size(); ++i)
		if(LastTime + SecondsDelay <= glfwGetTime()) {
			LaunchedSize ++;
			LastTime += SecondsDelay;
		}
		else
			break;
	for(size_t i = 0; i < LaunchedSize; ++i)
	{
		if(Arrived[i])
			continue;

		if(glm::distance(Drones[i], Destinations[i]) <= dist)
			Drones[i] = Destinations[i], Arrived[i] = true;
		else
			Drones[i] += Directions[i] * dist;
	}
}

const std::vector<glm::vec3> &DroneGroup::GetDronePositions() const {
	return Drones;
}

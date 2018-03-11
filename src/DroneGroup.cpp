//
// Created by adamyuan on 10/29/17.
//

#include "DroneGroup.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

#define CSV_SEPARATOR ','

DroneGroup::DroneGroup(const std::vector<glm::vec3> &initialPosition) {
	Init(initialPosition);
}

void DroneGroup::Init(const std::vector<glm::vec3> &initialPosition) {
	Drones = initialPosition;
	Arrived = std::vector<bool>(Drones.size(), true);
	Destinations.reserve(Drones.size());
	Directions.resize(Drones.size());

	for(size_t i=0; i<RECORD_NUM; ++i)
	{
		std::string fileName = std::string("drone") + std::to_string(i) + ".csv";
		RecordFiles[i].open(std::string(fileName));
	}
}

void DroneGroup::SetDestinations(const std::vector<glm::vec3> &destinations, float secondsBetween)
{
	if(!AllArrived())
		return;
	Destinations = destinations;
	Arrived = std::vector<bool>(Drones.size(), false);
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

void DroneGroup::NextTick(MyGL::FrameRateManager &FPSManager){

	float dist = FPSManager.GetMovementDistance(DRONE_SPEED);

	static int s_seconds = 0;
	if(glfwGetTime() > s_seconds + 1)
	{
		s_seconds = static_cast<int>(glfwGetTime());
		for(int i=0; i<RECORD_NUM; ++i) {
			if(RecordDroneIndices[i] >= Drones.size())
				continue;
			glm::vec3 &pos = Drones[RecordDroneIndices[i]];
			RecordFiles[i] << pos.x << CSV_SEPARATOR << pos.y << CSV_SEPARATOR << pos.z << std::endl;
			if(!RecordPositions[i].empty() && RecordPositions[i].back() == pos)
				continue;
			RecordPositions[i].push_back(pos);
		}
	}


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

		if(glm::length(Drones[i] - Destinations[i]) <= dist)
		{
			Drones[i] = Destinations[i];
			Arrived[i] = true;
		}
		else
			Drones[i] += Directions[i] * dist;
	}
}

const std::vector<glm::vec3> &DroneGroup::GetDronePositions() const {
	return Drones;
}

const std::vector<glm::vec3> &DroneGroup::GetRecordPositions(size_t index) const {
	return RecordPositions[index];
}

void DroneGroup::ClearRecords() {
	for (auto &RecordPosition : RecordPositions)
		if(RecordPosition.size() > 1)
			RecordPosition.erase(RecordPosition.begin() + 1, RecordPosition.end());
}

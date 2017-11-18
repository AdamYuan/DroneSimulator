//
// Created by adamyuan on 10/29/17.
//

#ifndef PROBLEMA_DRONEGROUP_HPP
#define PROBLEMA_DRONEGROUP_HPP

#include <glm/glm.hpp>
#include <vector>
#include <fstream>
#include <MyGL/FrameRate.hpp>

#define DRONE_SPEED 0.3f
#define RECORD_NUM 5
class DroneGroup
{
private:
	std::vector<glm::vec3> Drones;
	std::vector<glm::vec3> Destinations;
	std::vector<glm::vec3> Directions;
	std::vector<bool> Arrived;
	float SecondsDelay, LastTime;
	int LaunchedSize;

	std::ofstream RecordFiles[RECORD_NUM];
	std::vector<glm::vec3> RecordPositions[RECORD_NUM];
	int RecordDroneIndices[RECORD_NUM] = {0, 250, 1000, 750, 500};
public:
	DroneGroup() = default;
	explicit DroneGroup(const std::vector<glm::vec3> &initialPosition);
	void Init(const std::vector<glm::vec3> &initialPosition);
	bool AllArrived() const;
	const std::vector<glm::vec3> &GetDronePositions() const;
	const std::vector<glm::vec3> &GetRecordPositions(size_t index) const;
	void ClearRecords();
	void SetDestinations(const std::vector<glm::vec3> &destinations, float secondsBetween = 0.0f);
	void NextTick(MyGL::FrameRateManager &FPSManager);
};


#endif //PROBLEMA_DRONEGROUP_HPP

//
// Created by adamyuan on 10/28/17.
//

#include "ObjFileLoader.hpp"

#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <glm/gtx/hash.hpp>
#include <numeric>
#include <cfloat>

void ObjFileLoader::Load(const std::string &filename, float density, float scale, float deltaY)
{
	glm::vec3 maxPosition(FLT_MIN), minPosition(FLT_MAX);

	Points.clear();
	std::ifstream file(filename);
	if(!file.is_open())
		throw std::runtime_error("Unable to load obj file: " + filename);

	std::unordered_map<glm::ivec3, std::pair<glm::vec3, size_t>> PointMap;

	std::string line;
	while(std::getline(file, line))
	{
		if(line.empty())
			continue;
		if(line.size() > 1 && isalpha(line[1])) //prevent "nv %f %f %f"
			continue;

		if(line[0] == 'v')
		{
			float x, y, z;
			sscanf(line.c_str(), "%*c%f%f%f", &x, &y, &z);

			glm::vec3 v(x, y, z);
			v *= scale;
			v.y += deltaY;

			glm::ivec3 key = glm::ivec3(glm::round((v) / density));
			PointMap[key].first += v;
			PointMap[key].second ++;
		}
	}

	Points.clear();
	for(const auto &p : PointMap) {
		Points.push_back(p.second.first / (float) p.second.second);
		for(int i=0; i<3; ++i)
		{
			maxPosition[i] = std::max(maxPosition[i], Points.back()[i]);
			minPosition[i] = std::min(minPosition[i], Points.back()[i]);
		}
	}

	printf("max position: (%f, %f, %f)\n", maxPosition[0], maxPosition[1], maxPosition[2]);
	printf("min position: (%f, %f, %f)\n", minPosition[0], minPosition[1], minPosition[2]);
	std::cout << Points.size() << std::endl;
}

ObjFileLoader::ObjFileLoader(const std::string &filename, float density, float scale, float deltaY)
{
	Load(filename, density, scale, deltaY);
}

void ObjFileLoader::PickPoints(int num, std::vector<glm::vec3> &result)
{
	result.clear();
	if(Points.empty() || num == 0)
		return;

	num = std::min(num, (int)Points.size());

	int unit = (int)Points.size() / num;

	for(int i=0; i<num; ++i)
		result.push_back(Points[(float)Points.size() / (float)num * i + unit/2]);
}

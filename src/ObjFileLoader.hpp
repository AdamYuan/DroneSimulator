//
// Created by adamyuan on 10/28/17.
//

#ifndef PROBLEMA_OBJFILELOADER_HPP
#define PROBLEMA_OBJFILELOADER_HPP

#include <glm/glm.hpp>
#include <vector>
#include <iostream>

class ObjFileLoader
{
private:
	std::vector<glm::vec3> Points;

public:
	ObjFileLoader() = default;
	explicit ObjFileLoader(const std::string &filename, float density, float scale, float deltaY);

	void Load(const std::string &filename, float density, float scale, float deltaY);

	void PickPoints(int num, std::vector<glm::vec3> &result);
};


#endif //PROBLEMA_OBJFILELOADER_HPP

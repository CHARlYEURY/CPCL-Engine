#pragma once 
#include "ECS/Entity.h"
#include "ECS/ComponentPool.h"
#include <memory>
#include <vector>

template <typename T>
class Registry
{
private: 

	Entity _nextEntity = 0;
	std::vector<std::unique_ptr<IPool>> Ipools;

public: 

	Entity createEntity()
	{
		return _nextEntity++;
	}
	

};
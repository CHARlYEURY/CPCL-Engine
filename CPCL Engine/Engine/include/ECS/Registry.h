#pragma once 
#include "ECS/Entity.h"
#include "ECS/ComponentPool.h"
#include <memory>
#include <vector>

class Registry
{
private: 

	Entity _nextEntity = 0;

public: 

	Entity createEntity()
	{
		return _nextEntity++;
	}


};
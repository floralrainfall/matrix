#pragma once
#include <mscene.hpp>
#include <map.hpp>

namespace awesops
{
    class WorkerComponent : public mtx::SceneComponent
    {
	glm::ivec2 m_pathfindGoal;
	glm::ivec2 m_mapPosition;
	MapComponent* m_map;
    public:
	WorkerComponent(MapComponent* mapComponent);

	void setPathfindGoal(glm::ivec2 goal);
	void path();
    };
};


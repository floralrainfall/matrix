#pragma once
#include <mscene.hpp>
#include <mhwabs.hpp>
#include <mmaterial.hpp>
#include <map>
#include <set>

#define MAP_WIDTH 64
#define MAP_HEIGHT 64

namespace awesops
{
    enum MapTileType
    {
	MTT_GRASS,
	MTT_CONCRETE,
	MTT_PATHWAY,
	MTT_WATER,
    };
    
    struct MapTile
    {
	bool pathable;
	bool walkable;
	float height;
	glm::vec3 color;
    };
    
    class MapComponent : public mtx::SceneComponent
    {
	mtx::HWBufferReference* m_vertices;
	mtx::HWLayoutReference* m_layout;
	mtx::HWBufferReference* m_indices;
	mtx::Material* m_material;
	bool m_ready;
	int m_mapIndiceCount;

	MapTile m_mapData[MAP_WIDTH][MAP_HEIGHT];
    public:
	MapComponent();
	
	void updateMesh();
	virtual void renderComponent();

	MapTile& getMapTile(int x, int y) { return m_mapData[x][y]; }
	MapTile& getMapTile(glm::ivec2 v)
	{
	    return m_mapData[v.x][v.y];
	}
    };
}

#include <worker.hpp>
#include <cstring>
#include <mdev.hpp>

namespace awesops
{
    typedef std::pair<double, std::pair<int, int>> MapPair;
    
    struct MapTilePathInfo
    {
	MapTile d;
	float f;
	float g;
	float h;
	int parent_x;
	int parent_y;
    };

    static MapTilePathInfo __getMapTilePathInfo(MapTile m)
    {
	MapTilePathInfo i;
	i.d = m;
	i.f = FLT_MAX;
	i.g = FLT_MAX;
	i.h = FLT_MAX;
	i.parent_x = -1;
	i.parent_y = -1;
	
	return i;
    }

    WorkerComponent::WorkerComponent(MapComponent* mapComponent)
    {
	m_map = mapComponent;
    }

    void WorkerComponent::setPathfindGoal(glm::ivec2 goal)
    {
	m_pathfindGoal = goal;
    }

    enum MapDirection
    {
	MD_NW,
	MD_N,
	MD_NE,
	MD_E,
	MD_SE,
	MD_S,
	MD_SW,
	MD_W,
	__MD_MAX
    };

    std::pair<int, int> __ivec2ToPair(glm::ivec2 v)
    {
	return std::make_pair(v.x, v.y);
    }
    
    void WorkerComponent::path()
    {
	if(!m_map)
	{
	    DEV_SOFTWARN("Attempted to path without map");
	    return;
	}
	
	MapTilePathInfo mapDetails[MAP_WIDTH][MAP_HEIGHT];
	std::set<MapPair> openList;
	openList.insert(std::make_pair(0.0,
				       __ivec2ToPair(m_mapPosition)));

	bool closedList[MAP_WIDTH][MAP_HEIGHT];
	std::memset(closedList, false, sizeof(closedList));

	for(int i = 0; i < MAP_WIDTH; i++)
	{
	    for(int j = 0; j < MAP_HEIGHT; j++)
	    {
		mapDetails[i][j] =
		    __getMapTilePathInfo(m_map->getMapTile(i,j));
	    }
	}

	bool foundDest;
	while(!openList.empty())
	{
	    MapPair p = *openList.begin();
	    openList.erase(p);
	    glm::ivec2 ps = glm::ivec2(p.second.first,
				       p.second.second);
	    closedList[m_mapPosition.x][m_mapPosition.y] = true;
	    double gNew, hNew, fNew;

	    for(int d = 0; d < __MD_MAX; d++)
	    {
		glm::ivec2 ts = ps;
		switch((MapDirection)d)
		{
		case MD_NW:
		    ts.y++;
		    ts.x--;
		    break;
		case MD_N:
		    ts.y++;
		    break;
		case MD_NE:
		    ts.y++;
		    ts.x++;
		case MD_E:
		    ts.x++;
		    break;
		case MD_SE:
		    ts.y--;
		    ts.x++;
		case MD_S:
		    ts.y--;
		    break;
		case MD_SW:
		    ts.y--;
		    ts.x--;
		    break;
		case MD_W:
		    ts.x--;
		    break;
		default:
		    continue;
		}
		if(ts.x > MAP_WIDTH)
		    continue;
		if(ts.y > MAP_HEIGHT)
		    continue;
		if(ts.x < 0)
		    continue;
		if(ts.y < 0)
		    continue;
		
		if(ts == m_pathfindGoal)
		{
		    mapDetails[ts.x][ts.y].parent_x = ps.x;
		    mapDetails[ts.x][ts.y].parent_y = ps.y;
		    DEV_MSG("destination cell found @ %i,%i",
			    ts.x, ts.y);
		    foundDest = true;
		    return;
		}
		else if(closedList[ts.x][ts.y] == false &&
			mapDetails[ts.x][ts.y].d.pathable)
		{
		    gNew = mapDetails[ts.x][ts.y].g + 1.0;
		    glm::vec2 tsf = glm::vec2(ts);
		    glm::vec2 psf = glm::vec2(ps);
		    hNew = glm::distance(tsf, psf);
		    fNew = gNew + hNew;

		    if(mapDetails[ts.x][ts.y].f == FLT_MAX ||
		       mapDetails[ts.x][ts.y].f > fNew)
		    {
			openList.insert(std::make_pair(
					    fNew, __ivec2ToPair(ts)
					    ));
			mapDetails[ts.x][ts.y].f = fNew;
			mapDetails[ts.x][ts.y].g = gNew;
			mapDetails[ts.x][ts.y].h = hNew;
			mapDetails[ts.x][ts.y].parent_x = ps.x;
			mapDetails[ts.x][ts.y].parent_y = ps.y;
		    }
		}
	    }
	}

	if(foundDest)
	    DEV_SOFTWARN("failed to find destination cell");
	return;
    }
}

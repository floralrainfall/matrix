#include <map.hpp>
#include <vector>
#include <mapp.hpp>

namespace awesops
{
    struct MapVertex
    {
	glm::vec3 position;
	glm::vec3 color;
    };

    MapComponent::MapComponent()
    {
	m_ready = false;
	m_material = 
	    mtx::Material::getMaterial("materials/map.mmf");
	for(int i = 0; i < MAP_WIDTH; i++)
	    for(int j = 0; j < MAP_HEIGHT; j++)
	    {
		MapTile& t = getMapTile(i, j);
		t.height = sinf(i)*2.0;
		t.color =
		    ((j%2==0) ? glm::vec3(0.5) :
		     glm::vec3(0.0)) +
		    ((i%2==0) ? glm::vec3(0.5) :
		     glm::vec3(0.0));
	    }
    }

    void MapComponent::updateMesh()
    {
	std::vector<MapVertex> vertices;
	for(unsigned int i = 0; i < MAP_HEIGHT; i++)
	    for(unsigned int j = 0; j < MAP_WIDTH; j++)
	    {
		MapVertex v;
		MapTile t = getMapTile(j, i);
		v.color = t.color;

		float z = t.height;
		
		v.position = glm::vec3(
		    -MAP_HEIGHT/2.0f + i,
		    -MAP_WIDTH/2.0f + j,
		    z
		    );
		vertices.push_back(v);
	    }

	std::vector<unsigned int> indices;
	for(unsigned int i = 0; i < MAP_HEIGHT - 1; i++)
	    for(unsigned int j = 0; j < MAP_WIDTH; j++)
		for(int k = 0; k < 2; k++)
		{
		    indices.push_back(j + MAP_WIDTH
					* (i + k));

		    
		}
	m_mapIndiceCount = indices.size();
		
	m_vertices = mtx::App::getHWAPI()->newBuffer();
	m_vertices->upload(vertices.size() * sizeof(MapVertex),
			   vertices.data());
	m_indices = mtx::App::getHWAPI()->newBuffer();
	m_indices->upload(indices.size() * sizeof(unsigned int),
			  indices.data());
	m_layout = mtx::App::getHWAPI()->newLayout();
	m_layout->addEntry(
	    {m_vertices, 3, mtx::HWT_FLOAT, false,
	     sizeof(MapVertex),
	     (void*)offsetof(MapVertex, position)});
	m_layout->addEntry(
	    {m_vertices, 3, mtx::HWT_FLOAT, false,
	     sizeof(MapVertex),
	     (void*)offsetof(MapVertex, color)});
	m_layout->upload();
	m_ready = true;
    }

    void MapComponent::renderComponent()
    {
	if(!m_ready)
	    return;
	mtx::App::getHWAPI()->gfxDrawElements(
	    mtx::HWAPI::HWPT_TRIANGLE_STRIP,
	    m_layout,
	    m_mapIndiceCount, m_indices,
	    m_material->getProgram());
    }
}

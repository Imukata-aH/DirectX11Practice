#pragma once

#include <vector>
#include <minwindef.h>

#include "d3dUtil.h"

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

class Batch
{
public:
	Batch(std::vector<Vertex>* vertices, std::vector<UINT>* indices);
	~Batch();

	void Release();

private:
	std::vector<Vertex>* m_vertices;
	std::vector<UINT>* m_indices;

};
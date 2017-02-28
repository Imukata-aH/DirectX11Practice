#pragma once

#include <d3dUtil.h>
#include <vector>
#include <minwindef.h>

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

private:
	std::vector<Vertex>* vertices;
	std::vector<UINT>* indices;
};
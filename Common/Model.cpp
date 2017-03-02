#include "Model.h"
#include "Batch.h"

Model::Model( Batch* batch ) :
	m_batch( batch )
{

}

Model::~Model()
{
}

void Model::Release()
{
	m_batch->Release();
}
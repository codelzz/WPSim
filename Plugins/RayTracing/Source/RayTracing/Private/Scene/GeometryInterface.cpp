#include "GeometryInterface.h"
#include "MeshBatch.h"

namespace RayTracing 
{

	TArray<FMeshBatch> FGeometryInstanceRenderStateRef::GetMeshBatchesForGBufferRendering(FTileVirtualCoordinates CoordsForCulling)
	{
		return Collection.GetMeshBatchesForGBufferRendering(*this, CoordsForCulling);
	}

}
#include "Common.h"
#include "Storage.h"

namespace RayTracing
{
	bool FRenderState::DoesTileHaveValidCPUData(FTileVirtualCoordinates Coords, int32 CurrentRevision)
	{
		return RetrieveTileState(Coords).CPURevision == CurrentRevision;
	}
}
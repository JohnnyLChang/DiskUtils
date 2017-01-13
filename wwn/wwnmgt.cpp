#include "stdafx.h"
#include "wwnmgt.h"

std::map<VendorWWN, VendorID> g_mapWWN;

/*not thread safe!!*/
bool IsSupportWWN(VendorWWN wwn)
{
	if (g_mapWWN.empty())
	{
		for(int i=0;i<g_nCountTMVendorWWN;i++)
			g_mapWWN.insert(std::pair<VendorWWN, VendorID>(g_TMVendorWWN[i].vendorWWN, g_TMVendorWWN[i].vendorID));
	}

	return (g_mapWWN.find(wwn) != g_mapWWN.end());
}

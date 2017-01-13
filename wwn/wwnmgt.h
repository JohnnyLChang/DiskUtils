#pragma once
#include <map>

enum enumVendorID {
	Seagate,
	SanDisk,
	Toshiba,
	Samsung
};

#pragma pack(push, byte_stuff, 1)
typedef union _FOUR_BYTE {
	struct {
		unsigned char Byte0;
		unsigned char Byte1;
		unsigned char Byte2;
		unsigned char Byte3;
	};
	unsigned long AsULong;
} FOUR_BYTE, *PFOUR_BYTE;
#pragma pack(pop, byte_stuff)

typedef unsigned int VendorID;
typedef unsigned long long VendorWWN;

typedef struct _VendorWWNPair {
	VendorWWN vendorWWN;
	VendorID vendorID;
}VendorWWNPair;


/*
How to add Vendor OUI
1. Query https://regauth.standards.ieee.org/standards-ra-web/pub/view.html#registries
2. use 002538 as Samsung example
3. 
	00-25-38 (hex)    MA-L 	Samsung Electronics Co., Ltd., Memory Division 	San #16 Banwol-Dong, Taean-Gu
	002538			  Hwasung City Gyeonggi Do 445-701 KR 

4. Add 0x00253800 into g_TMVendorWWN

*/
const unsigned int g_nCountTMVendorWWN = 12;
const VendorWWNPair g_TMVendorWWN[g_nCountTMVendorWWN] = { 
	{ 0x00253800, Samsung }, //Seagate WWN, just for testing
	{ 0x00203700, Seagate },
	{ 0xB4525300, Seagate },
	{ 0x0024B600, Seagate },
	{ 0x00186200, Seagate },
	{ 0x0014C300, Seagate },
	{ 0x0004CF00, Seagate },
	{ 0x001D3800, Seagate },
	{ 0x0011C600, Seagate },
	{ 0x000C5000, Seagate },

	{ 0x001B4400, SanDisk },
	{ 0x001E8200, SanDisk }
};


bool IsSupportWWN(VendorWWN wwn);

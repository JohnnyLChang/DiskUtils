// test3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <Setupapi.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <ntddscsi.h>
#include "Ata.h"
#include <map>
#include "wwnmgt.h"

#define TM_MAJOR_VERSION	(5)
#define TM_MINOR_VERSION	(4)


//
// This structure is used to convert little endian
// ULONGs to SCSI CDB big endians values.
//

#pragma pack(push, byte_stuff, 1)
typedef union _EIGHT_BYTE {

	struct {
		UCHAR Byte0;
		UCHAR Byte1;
		UCHAR Byte2;
		UCHAR Byte3;
		UCHAR Byte4;
		UCHAR Byte5;
		UCHAR Byte6;
		UCHAR Byte7;
	};

	ULONGLONG AsULongLong;
} EIGHT_BYTE, *PEIGHT_BYTE;

typedef union _TWO_BYTE {

	struct {
		UCHAR Byte0;
		UCHAR Byte1;
	};

	USHORT AsUShort;
} TWO_BYTE, *PTWO_BYTE;
#pragma pack(pop, byte_stuff)

#define REVERSE_BYTES(Destination, Source) {                \
    PFOUR_BYTE d = (PFOUR_BYTE)(Destination);               \
    PFOUR_BYTE s = (PFOUR_BYTE)(Source);                    \
    d->Byte3 = s->Byte0;                                    \
    d->Byte2 = s->Byte1;                                    \
    d->Byte1 = s->Byte2;                                    \
    d->Byte0 = s->Byte3;                                    \
}

#define REVERSE_BYTES_SHORT(Destination, Source) {          \
    PTWO_BYTE d = (PTWO_BYTE)(Destination);                 \
    PTWO_BYTE s = (PTWO_BYTE)(Source);                      \
    d->Byte1 = s->Byte0;                                    \
    d->Byte0 = s->Byte1;                                    \
}

static const GUID CLSID_DISK_UPPER_FILTER =
{ 0x4d36e967, 0xe325, 0x11ce,{ 0xbf, 0xc1, 0x8, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

 bool GetVendorWWN(int id, VendorWWN &vendorwwn)
{
	WCHAR _wszDiskPath[MAX_PATH] = { 0 };
	_snwprintf(_wszDiskPath, sizeof(_wszDiskPath), L"\\\\.\\PhysicalDrive%d", id);
	// Open handle to disk.
	HANDLE hDevice(::CreateFileW(_wszDiskPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL));
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		std::wcout << L"CreateFileW " << _wszDiskPath << L" failed.  LastError: " << GetLastError() << std::endl;
		return false;
	}

	//
	// Use IOCTL_ATA_PASS_THROUGH
	//
	std::vector< UCHAR > vBuffer(sizeof(ATA_PASS_THROUGH_EX) + sizeof(IDENTIFY_DEVICE_DATA), 0);
	PATA_PASS_THROUGH_EX pATARequest(reinterpret_cast< PATA_PASS_THROUGH_EX >(&vBuffer[0]));
	pATARequest->AtaFlags = ATA_FLAGS_DATA_IN | ATA_FLAGS_DRDY_REQUIRED;
	pATARequest->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATARequest->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);
	pATARequest->DataTransferLength = sizeof(IDENTIFY_DEVICE_DATA);
	pATARequest->TimeOutValue = 2;
	pATARequest->CurrentTaskFile[6] = ID_CMD;

	ULONG ulBytesRead;
	if (DeviceIoControl(hDevice, IOCTL_ATA_PASS_THROUGH,
		&vBuffer[0], ULONG(vBuffer.size()),
		&vBuffer[0], ULONG(vBuffer.size()),
		&ulBytesRead, NULL) == FALSE)
	{
		std::cout << "DeviceIoControl(IOCTL_ATA_PASS_THROUGH) failed.  LastError: " << GetLastError() << std::endl;
		CloseHandle(hDevice);
		return false;
	}
	else
	{
		// Fetch identity blob from output buffer.
		PIDENTIFY_DEVICE_DATA pIdentityBlob(reinterpret_cast< PIDENTIFY_DEVICE_DATA >(&vBuffer[sizeof(ATA_PASS_THROUGH_EX)]));
		//printf("Security Frozen: %d\n", pIdentityBlob->SecurityStatus.SecurityFrozen);
		//printf("TrustedComputing: %d\n", pIdentityBlob->TrustedComputing.FeatureSupported);
		//unsigned char model[40] = { 0 };
		//for (int i = 0; i<40; i += 2)
		//	REVERSE_BYTES_SHORT(&model[i], &pIdentityBlob->ModelNumber[i]);
		//printf("Model: %s\n", model);
		//unsigned char serial[20] = { 0 };
		//for (int i = 0; i<20; i += 2)
		//	REVERSE_BYTES_SHORT(&serial[i], &pIdentityBlob->SerialNumber[i]);
		//printf("Serial: %s\n", serial);
		unsigned short upper = 0;
		unsigned short lower = 0;
		memcpy(&upper, &pIdentityBlob->WorldWideName[0], sizeof(upper));
		memcpy(&lower, &pIdentityBlob->WorldWideName[1], sizeof(lower));
		lower = lower >> 4 << 4;
		vendorwwn = upper << 20;
		vendorwwn |= lower << 4;
		printf("OUI: %08lX\n", vendorwwn);
		printf("WWN: %04X %04X %04X %04X\n", pIdentityBlob->WorldWideName[0], pIdentityBlob->WorldWideName[1], pIdentityBlob->WorldWideName[2], pIdentityBlob->WorldWideName[3]);
	}

	CloseHandle(hDevice);

	return true;
}

int main(int argc, _TCHAR* argv[])
{
	VendorWWN wwn = { 0 };
	if (GetVendorWWN(0, wwn))
	{
		std::cout<<"IsSupportWWN ? "<<IsSupportWWN(wwn)<<std::endl;
	}
	else
	{
		std::cout<<"GetVendorWWN failed"<<std::endl;
	}
	return 0;
}


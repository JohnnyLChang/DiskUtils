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

#define TM_MAJOR_VERSION	(5)
#define TM_MINOR_VERSION	(4)

static const GUID CLSID_DISK_UPPER_FILTER = 
{ 0x4d36e967, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x8, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

int foo()
{
    int iRet( 0 );

    // Open handle to disk.
    HANDLE hDevice( ::CreateFileW( L"\\\\.\\PhysicalDrive2", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) );
    if( hDevice == INVALID_HANDLE_VALUE )
    {
        std::wcout << L"CreateFileW failed.  LastError: " << GetLastError() << std::endl;
        return -1;
    }

    //
    // Use IOCTL_ATA_PASS_THROUGH
    //
    std::vector< UCHAR > vBuffer( sizeof( ATA_PASS_THROUGH_EX ) + sizeof( IDENTIFY_DEVICE_DATA ), 0 );
    PATA_PASS_THROUGH_EX pATARequest( reinterpret_cast< PATA_PASS_THROUGH_EX >( &vBuffer[0] ) );
    pATARequest->AtaFlags = ATA_FLAGS_DATA_IN | ATA_FLAGS_DRDY_REQUIRED;
    pATARequest->Length = sizeof( ATA_PASS_THROUGH_EX );
    pATARequest->DataBufferOffset = sizeof( ATA_PASS_THROUGH_EX );
    pATARequest->DataTransferLength = sizeof( IDENTIFY_DEVICE_DATA );
    pATARequest->TimeOutValue = 2;
    pATARequest->CurrentTaskFile[6] = ID_CMD;

    ULONG ulBytesRead;
    if( DeviceIoControl( hDevice, IOCTL_ATA_PASS_THROUGH, 
        &vBuffer[0], ULONG( vBuffer.size() ),
        &vBuffer[0], ULONG( vBuffer.size() ),
        &ulBytesRead, NULL ) == FALSE )
    {
        std::cout << "DeviceIoControl(IOCTL_ATA_PASS_THROUGH) failed.  LastError: " << GetLastError() << std::endl;
        iRet = -1;
    }
    else
    {
        // Fetch identity blob from output buffer.
        PIDENTIFY_DEVICE_DATA pIdentityBlob( reinterpret_cast< PIDENTIFY_DEVICE_DATA >( &vBuffer[ sizeof( ATA_PASS_THROUGH_EX ) ] ) );
        printf("Security Frozen: %d\n", pIdentityBlob->SecurityStatus.SecurityFrozen);
        printf("TrustedComputing: %d\n", pIdentityBlob->TrustedComputing.FeatureSupported);
    }

    CloseHandle( hDevice );

    return iRet;
}

int main(void)
{
    //ULONG ver = TM_MAJOR_VERSION << 16 | TM_MINOR_VERSION;
    //printf("version %08x\n", ver);
    foo();
    return 0;
}

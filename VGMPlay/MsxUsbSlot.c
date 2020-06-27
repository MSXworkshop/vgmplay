// MsxUsbSlot.c: C Source File for MSX USB SLOT

#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "MsxUsbSlot.h"

static HANDLE s_hComm = INVALID_HANDLE_VALUE;
static int s_nPort = 0;

bool serial_Open(int nPort)
{
	char commName[64];
	sprintf(commName, "\\\\.\\COM%d", nPort);
	s_hComm = CreateFile(commName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (s_hComm == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	GetCommState(s_hComm, &dcb);
	dcb.BaudRate = 2000000;
	dcb.Parity = NOPARITY;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.fDsrSensitivity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	SetCommState(s_hComm, &dcb);

	COMMTIMEOUTS Timeouts;
	GetCommTimeouts(s_hComm, &Timeouts);
	Timeouts.WriteTotalTimeoutMultiplier = 0;
	Timeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(s_hComm, &Timeouts);

	//COMMTIMEOUTS Timeouts;
	GetCommTimeouts(s_hComm, &Timeouts);
	Timeouts.ReadIntervalTimeout = MAXDWORD;
	Timeouts.ReadTotalTimeoutMultiplier = 0;
	Timeouts.ReadTotalTimeoutConstant = 0;
	SetCommTimeouts(s_hComm, &Timeouts);

	return true;
}

bool serial_IsOpen()
{
	return s_hComm != INVALID_HANDLE_VALUE;
}

void serial_Close()
{
	CloseHandle(s_hComm);
	s_hComm = INVALID_HANDLE_VALUE;
}

DWORD serial_Write(const void* lpBuffer, DWORD dwNumberOfBytesToWrite)
{
	DWORD dwBytesWritten = 0;
	WriteFile(s_hComm, lpBuffer, dwNumberOfBytesToWrite, &dwBytesWritten, NULL);
	return dwBytesWritten;
}

int serial_Read(void* lpBuffer, DWORD dwNumberOfBytesToRead)
{
	int nBytesRead = 0;
	if (!ReadFile(s_hComm, lpBuffer, dwNumberOfBytesToRead, &nBytesRead, NULL))
	{
		return -1;
	}
	return nBytesRead;
}

int serial_FindAndOpenPort()
{
	for (int i = 1; i < 256; i++)
	{
		char sPort[64];
		sprintf(sPort, "\\\\.\\COM%d", i);

		bool bSuccess = false;
		HANDLE hPort = CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPort == INVALID_HANDLE_VALUE)
		{
			const DWORD dwError = GetLastError();

			if ((dwError == ERROR_ACCESS_DENIED) || (dwError == ERROR_GEN_FAILURE) || (dwError == ERROR_SHARING_VIOLATION) || (dwError == ERROR_SEM_TIMEOUT))
				bSuccess = true;
		}
		else
		{
			CloseHandle(hPort);
			bSuccess = true;
		}

		if (bSuccess)
		{
			BOOL bOpen = serial_Open(i);
			if (bOpen) {
				BYTE buf[256];
				buf[0] = 'V';
				buf[1] = '\n';
				DWORD dwWritten = serial_Write(buf, 2);
				BYTE* version = "MSX USB SLOT";
				int nVerLen = strlen(version);
				DWORD dwStart = GetTickCount();
				int nRead = 0;
				while (true) {
					int rbyte = serial_Read(buf + nRead, 256 - nRead);
					if (rbyte < 0) {
						serial_Close();
						break;
					} else {
						nRead += rbyte;
						if (0 < nRead) {
							if (nRead < nVerLen) {
								if (strncmp(version, buf, nRead) != 0) {
									serial_Close();
									break;
								}
							} else {
								if (strncmp(version, buf, nVerLen) != 0) {
									serial_Close();
								} else {
									// Found
								}
								break;
							}
						}
					}

					if (dwStart + 1000 < GetTickCount()) {
						serial_Close();
						break;
					}

				} // while
				if (serial_IsOpen()) {
					return i;
				}
			}
		}
	} // for

	return 0;
}

int msxusbslot_Open()
{
	if (serial_IsOpen()) {
		return s_nPort;
	}
	s_nPort = serial_FindAndOpenPort();
	return s_nPort;
}

void msxusbslot_Close()
{
	serial_Close();
	s_nPort = 0;
}

bool msxusbslot_IsOpen()
{
	return serial_IsOpen();
}

void msxusbslot_writeByte(int addr, int data)
{
	UINT8 buf[4];
	buf[0] = 'b';
	buf[1] = addr & 0xFF;
	buf[2] = (addr >> 8) & 0xFF;
	buf[3] = data;
	serial_Write(buf, 4);
}

void msxusbslot_writeOPLL(int reg, int data)
{
	if (!serial_IsOpen()) {
		return;
	}
	BYTE buf[3];
	buf[0] = 'f';
	buf[1] = reg;
	buf[2] = data;
	serial_Write(buf, 3);
}

void msxusbslot_writeSCC(int addr, int data)
{
	if (!serial_IsOpen()) {
		return;
	}
	BYTE buf[3];
	buf[0] = 'c';
	buf[1] = addr;
	buf[2] = data;
	serial_Write(buf, 3);
}

#endif

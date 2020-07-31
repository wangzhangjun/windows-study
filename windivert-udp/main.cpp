#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "windivert.h"

#define MAXBUF          0xFFFF

#pragma comment(lib, "Ws2_32.lib")

OVERLAPPED *poverlapped;
OVERLAPPED overlapped;
HANDLE  event;
HANDLE handle;
HANDLE ioport;
unsigned char packet[MAXBUF];
UINT packet_len;
WINDIVERT_ADDRESS addr;
PWINDIVERT_IPHDR ip_header;
PWINDIVERT_UDPHDR udp_header;

static void cleanup(HANDLE ioport, OVERLAPPED *ignore)
{
	OVERLAPPED *overlapped;
	DWORD iolen;
	ULONG_PTR iokey = 0;

	while (GetQueuedCompletionStatus(ioport, &iolen, &iokey, &overlapped, 0))
	{
		if (overlapped != ignore)
		{
			free(overlapped);
		}
	}
}

void monitor()
{
	std::cout << "start to monitor udp package" << std::endl;
	while (TRUE)
	{
		memset(&overlapped, 0, sizeof(overlapped));
		ResetEvent(event);
		overlapped.hEvent = event;

		if (!WinDivertRecvEx(handle, packet, sizeof(packet), 0, &addr,
			&packet_len, &overlapped))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
			read_failed:
				std::cout << "failed to read packet (%d)\n" << GetLastError() << std::endl;
				continue;
			}
			while (WaitForSingleObject(event, 1000) == WAIT_TIMEOUT)
			{
				cleanup(ioport, &overlapped);
			}
			DWORD len;
			if (!GetOverlappedResult(handle, &overlapped, &len, FALSE))
			{
				goto read_failed;
			}
			packet_len = len;
		}
		cleanup(ioport, &overlapped);
		if (!WinDivertHelperParsePacket(packet, packet_len, &ip_header, NULL,
			NULL, NULL, NULL, &udp_header, NULL, NULL))
		{
			std::cout << "failed to parse packet (%d)" << GetLastError() << std::endl;
			continue;
		}

		switch (addr.Direction)
		{
		case WINDIVERT_DIRECTION_OUTBOUND:
			std::cout << htons(udp_header->DstPort) << std::endl;
			std::cout << "outbound" << std::endl;
			break;
		case WINDIVERT_DIRECTION_INBOUND:
			std::cout << "inbound" << std::endl;
			break;
		}
		WinDivertHelperCalcChecksums(packet, packet_len, &addr, 0);
		poverlapped = (OVERLAPPED *)malloc(sizeof(OVERLAPPED));
		if (poverlapped == NULL)
		{
			std::cout << " to allocate memory"<<std::endl;
		}
		memset(poverlapped, 0, sizeof(OVERLAPPED));
		if (WinDivertSendEx(handle, packet, packet_len, 0, &addr, NULL,
			poverlapped))
		{
			continue;
		}
		if (GetLastError() != ERROR_IO_PENDING)
		{
			std::cout << "failed to send packet (%d)" << GetLastError()<< std::endl;
			continue;
		}
	}
}

int main()
{
	char filter[256];
	int filter_result;
	ioport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (ioport == NULL)
	{
		std::cout <<"failed to create I/O completion port (%d)\n" <<  GetLastError() << std::endl;
		return -1;
	}
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (event == NULL)
	{
		std::cout << "failed to create event (%d)" << GetLastError() << std::endl;;
		return -1;
	}

	//cretae filter
	filter_result = sprintf_s(filter, sizeof(filter),
		"udp and "
		"udp.PayloadLength > 0 and "
		"udp.DstPort == 12345 or udp.SrcPort = 12345"
		);

	//open windivert
	handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		std::cout << "failed to open the WinDivert device (%d)" << GetLastError() << std::endl;
		return -1;
	}

	if (CreateIoCompletionPort(handle, ioport, 0, 0) == NULL)
	{
		std::cout << "failed to associate I/O completion port (%d)" << GetLastError() << std::endl;
		return -1;
	}

	monitor();
	
}
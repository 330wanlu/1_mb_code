#include "udp.h"
#include <Winsock2.h>
#include <iphlpapi.h>
#include "typedef_data.h"
#include "terminal_manage.h"
#include "log.h"
//#include "system.h"

#define out(format,...)		log_out(format , ##__VA_ARGS__ )
#pragma comment(lib, "IPHLPAPI.lib")

#define kBufferSize   1000
/*UDP广播 发现设备*/
int udp_send(struct Cabinets *data)
{
	SOCKET sockClient = socket(AF_INET , SOCK_DGRAM , 0);
	SOCKADDR_IN addrServer, peer;
	IP_ADAPTER_INFO *pAdapterInfo;
	ULONG ulOutBufLen;
	DWORD dwRetVal;
	PIP_ADAPTER_INFO pAdapter = NULL;
	unsigned char cRecvBuffer[kBufferSize];
	int sendret = 0,postion = 0;
	char send_buf[100] = {0};
	uint32 i = 0,len = 0,n_ip = 0;
	struct timeval timeout = {0};
	timeout.tv_sec = 3000;
	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

	ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));
	send_buf[0] = 0x01;
	send_buf[1] = 0x10;
	send_buf[2] = 0x00;
	send_buf[3] = 0x06;
	send_buf[4] = 0x04;
	send_buf[5] = 0x01;

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS){
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
	}
	
	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS){
		return 0;
	}
	pAdapter = pAdapterInfo;
	
	while (pAdapter){
		
		if (strstr(pAdapter->Description, "Virtual") != NULL || strstr(pAdapter->Description, "TAP-Windows") != NULL || strstr(pAdapter->Description, "Bluetooth") != NULL){
			out("pAdapter->Type:%d", pAdapter->Type);
			pAdapter = pAdapter->Next;
			continue;
		}
		out("pAdapter->Type:%d", pAdapter->Type);
		struct in_addr target_address;
		unsigned long ipAddress = inet_addr(pAdapter->IpAddressList.IpAddress.String);
		unsigned long maskaddress = inet_addr(pAdapter->IpAddressList.IpMask.String);
		addrServer.sin_addr.S_un.S_addr = ((~maskaddress) | (ipAddress & maskaddress));
		addrServer.sin_port = htons(10002);
		addrServer.sin_family = AF_INET;
		target_address.S_un.S_addr = addrServer.sin_addr.S_un.S_addr;
		len = sizeof(addrServer);

		out("UDP广播:%s", inet_ntoa(target_address));

		sendret = sendto(sockClient, send_buf, 6, 0, (struct sockaddr*)&addrServer, sizeof(addrServer));
		while (1){
			if (sendret <= SOCKET_ERROR){
				pAdapter = pAdapter->Next;
				break;
			}
				
			memset(cRecvBuffer, 0, sizeof(cRecvBuffer));
			postion = 6;
			int bRecv = recvfrom(sockClient, cRecvBuffer, kBufferSize, 0, (struct sockaddr*)&peer, &len);
			if (bRecv <= 0) {
				pAdapter = pAdapter->Next;
				break;
			}
			n_ip = (((((uint32)cRecvBuffer[postion]) << 24)&(0xFF000000)) + ((((uint32)cRecvBuffer[postion + 1]) << 16)&(0x00FF0000)) + ((((uint32)cRecvBuffer[postion + 2]) << 8)&(0x0000FF00)) + (((uint32)cRecvBuffer[postion + 3])&(0x000000FF)));//  
			ip2ip_asc(n_ip, data->attribute[i].ca_SettingIP);
			postion += 4;
			data->attribute[i].port = (((int)cRecvBuffer[postion] << 8) + ((int)cRecvBuffer[postion + 1]));
			postion += 2;
			data->attribute[i].dataport = (((int)cRecvBuffer[postion] << 8) + ((int)cRecvBuffer[postion + 1]));
			postion += 2;
			memcpy(data->attribute[i].ca_TerminalID, cRecvBuffer + postion, 12);
			postion += 12;
			data->attribute[i].i_USBSumNumber = (((((uint32)cRecvBuffer[postion]) << 24)&(0xFF000000)) + ((((uint32)cRecvBuffer[postion + 1]) << 16)&(0x00FF0000)) + ((((uint32)cRecvBuffer[postion + 2]) << 8)&(0x0000FF00)) + (((uint32)cRecvBuffer[postion + 3])&(0x000000FF)));
			postion += 4;
			memcpy(data->attribute[i].ca_SoftVesion, cRecvBuffer + postion, 8);
			postion += 8;
			memcpy(data->attribute[i].ca_TerminalModel, cRecvBuffer + postion, 10);
			i++;

		}
	}
	socket_close(sockClient);
	return i;
}


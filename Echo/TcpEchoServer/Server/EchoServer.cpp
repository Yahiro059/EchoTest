#include <WinSock2.h>
#include <stdio.h>
#include <thread>
#include <WS2tcpip.h>
#include "wsock32error.h"

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#define PORT_NO 30250
#define GROUP_ADDR "239.0.1.23"

SOCKET sock;

//#define TCP_MODE (0)
#define TCP_MODE (1)

void recvThread();

void main(){
	//������
	WSADATA wsa;
	WORD verno;
	int sts;
	std::thread th_handle;

	//WinSock2������
	verno = MAKEWORD(2, 2);
	sts = WSAStartup(verno, &wsa);

	if (sts == SOCKET_ERROR){
		errcom(WSAGetLastError());
		return;
	}

	if (verno != wsa.wVersion){
		printf("Version Error\n");
		return;
	}

	//
#if TCP_MODE
	//TCP�\�P�b�g
	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
#else
	//UDP�\�P�b�g
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
#endif

#if !TCP_MODE
	//�}���`�L���X�g�O���[�v�ւ̎Q��
	ip_mreq mreq; //�}���`�L���X�g�A�h���X�p�\����
	mreq.imr_multiaddr.S_un.S_addr = inet_addr(GROUP_ADDR);
	mreq.imr_interface.S_un.S_addr = INADDR_ANY;

	//�o�^
	sts = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
	if (sts == SOCKET_ERROR){
		errcom(WSAGetLastError());
	}
#endif

	th_handle = std::thread(recvThread);
	//�X���b�h���N�����Ă�����getchar()�őҊ�
	if (th_handle.joinable()){
		getchar();
		//SOCKET�N���[�Y
		closesocket(sock);
		th_handle.join();
	}

	//WinSock�I��
	WSACleanup();
}//main

void recvThread(){
	int sts;

	//�|�[�g�ԍ��̐ݒ�(bind)
	sockaddr_in myaddr;
	int addr_len;

	//�A�h���X�̐ݒ�
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORT_NO);
	myaddr.sin_addr.S_un.S_addr = INADDR_ANY;
	//sin_addr�͎g���l�b�g���[�N�|�[�g�̃A�h���X
	//INADDR_ANY�͂O�ŁA�u�S�āv�̈Ӗ��ɂȂ�

	//sock�Ɏ��A�h���X��o�^(Server�͕K���s��)
	addr_len = sizeof(sockaddr_in); //�A�h���X�T�C�Y
	sts = bind(sock, (sockaddr*)&myaddr, addr_len);

	if (sts == SOCKET_ERROR){
		errcom(WSAGetLastError());
		return;
	}

#if TCP_MODE
	//listen
	listen(sock, SOMAXCONN);
	//��2�����͍ő�҂���.�ڑ��ł͂Ȃ�.
	//listen��conect�҂��X�^�[�g
#endif

	//��M
	char recvBuff[256];
	sockaddr_in fromaddr;
	char recvData[] = "�T�[�o�[����̑��M�f�[�^";
	//��M���[�v
	while (true){
#if TCP_MODE
		//SOCKET�錾
		SOCKET cl_sock;	//�ʐM�p�\�P�b�g
		//accept
		cl_sock = accept(sock, (sockaddr*)&fromaddr, &addr_len);
		//conect�̕ԓ��A����ŒʐM�p�\�P�b�g�����������.
		//conect������܂ŃX���b�h���~�܂�
		//�V�������[�U��������ƃ\�P�b�g��������
		//�G���[�`�F�b�N
		if (cl_sock == INVALID_SOCKET)
		{
			errcom(WSAGetLastError());
			return;
		}
		//recv
		sts = recv(cl_sock,recvBuff,sizeof(recvBuff),0);
#else
		sts = recvfrom(sock, recvBuff, sizeof(recvBuff), 0, (sockaddr*)&fromaddr, &addr_len);
#endif

		if (sts == SOCKET_ERROR) 
		{
			errcom(WSAGetLastError());
			break;
		}

		//�f�[�^�\��
		printf("��M�f�[�^�F%s\n", recvBuff);

		//�f�[�^�����M���ꂽ���Ƃ̊m�F�Ƃ��Ċm�F�p�f�[�^��Ԃ�.
#if TCP_MODE
		//send
		
		sts = send(cl_sock, recvData,strlen(recvData) + 1,0);
		//cl_sock���g�����Ƃɒ���
		if (sts == SOCKET_ERROR)
		{
			errcom(WSAGetLastError());
			return;
		}
		//�N���C�A���g�\�P�b�g��close
		//Echo���Ƃ����ő���̑S�Ă̑S�ʐM���I���Ȃ̂�
		closesocket(cl_sock);
		//�����Aclose�Ȃ��ŏI������Ǝ��̃��[�v��cl_socket���㏑�������
		//�O���[�v��cl_socket�����������[�N�ɂȂ�.
		
#else
		sts = sendto(sock, recvBuff, strlen(recvBuff) + 1, 0, (sockaddr*)&fromaddr, addr_len);
#endif

		if (sts == SOCKET_ERROR){
			errcom(WSAGetLastError());
			break;
		}
	}//while
}

/*-------------------------------------------
UDP �G�R�[�N���C�A���g(�R���\�[��)
---------------------------------------------*/

#include	<winsock2.h>
#include	<stdio.h>
#include	"wsock32error.h"

#pragma comment (lib, "ws2_32.lib")
#pragma warning (disable:4996)

//#define TCP_MODE (0)
#define TCP_MODE (1)
//------------------------------------
// �}�N��
//------------------------------------
#define		PORTNO		30250		

void main(){
	int		sts;
	int		errcode;
	WSADATA		m_wd;					// �v�r�`�r�s�`�q�s�t�o�p

	WORD	requiredversion;
	SOCKET	mysock;				// �\�P�b�g�ԍ�	�O����I�������邽�߁B
	sockaddr_in toaddr;				// ���M��A�h���X
	int			len;				// ���M��A�h���X��

	char*	senddata = "�N���C�A���g����̑��M�f�[�^";

	char	ip_str[256];		// IP�A�h���X�擾�p�o�b�t�@

	// ���̃v���O�������v������o�[�W����
	requiredversion = MAKEWORD(2, 2);

	// �v�h�m�r�n�b�j������
	sts = WSAStartup(MAKEWORD(2, 2), &m_wd);
	if (sts != 0)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return;
	}
	// �o�[�W�����`�F�b�N
	if (m_wd.wVersion != requiredversion)
	{
		printf("Version Error\n");
		return;
	}

#if TCP_MODE
	// �\�P�b�g�쐬(TCP�p�̃\�P�b�g�쐬)
	mysock = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
#else
	// �\�P�b�g�쐬(UDP�p�̃\�P�b�g�쐬)
	mysock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
#endif
	if (mysock == INVALID_SOCKET)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return;
	}

	//���M��IP�A�h���X�擾
	printf("IP�A�h���X����͂��ĉ������B\n");
	scanf_s("%s", ip_str, sizeof(ip_str));

#if !TCP_MODE
	//Limited BroadCast
	bool optval;
	optval = true;
	sts = setsockopt(mysock, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(bool));

	if (sts == SOCKET_ERROR)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return;
	}
#endif

	// ����̐ݒ�
	toaddr.sin_port = htons(PORTNO);
	toaddr.sin_family = AF_INET;
	toaddr.sin_addr.s_addr = inet_addr(ip_str);

	// ��M
	len = sizeof(sockaddr);	// ���M���A�h���X��

#if TCP_MODE
	//connect
	sts = connect(mysock, (sockaddr*)&toaddr, len);
	if(sts==SOCKET_ERROR)
	{
		errcom(WSAGetLastError());
		return;
	}
	//send
	sts = send(mysock, senddata, strlen(senddata) + 1, 0);
	//TCP�ł͑����ύX�ł���sendto�͎g���Ȃ�.
#else
	sts = sendto(mysock, senddata, strlen(senddata) + 1, 0, (sockaddr*)&toaddr, len);
#endif
	if (sts == SOCKET_ERROR)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return;
	}
	else
	{
		char rcvbuf[256];
		sockaddr_in fromaddr;
		int fromLen = sizeof(sockaddr);

#if TCP_MODE
		//recv
		sts = recv(mysock, rcvbuf, sizeof(rcvbuf), 0);
		//recv���g���Ȃ�
#else
		sts = recvfrom(mysock, rcvbuf, sizeof(rcvbuf), 0, (sockaddr*)&fromaddr, &fromLen);
#endif

		if (sts == SOCKET_ERROR)
		{
			errcode = WSAGetLastError();
			errcom(errcode);
		}
		else
		{
			rcvbuf[sts] = '\0';
			printf("��M�f�[�^ : %s \n", rcvbuf);
		}

	}

	printf("�����L�[�������ƏI�����܂��B\n");
	rewind(stdin);
	getchar();

	// �\�P�b�g���N���[�Y
	sts = closesocket(mysock);	//�\�P�b�g���N���[�Y����B
	//��M�X���b�h�ŃG���[����������while�𔲂���B
	if (sts == SOCKET_ERROR)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
	}

	// �v�h�m�r�n�b�j�̌㏈��
	WSACleanup();

}
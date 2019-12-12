/*-------------------------------------------
UDP エコークライアント(コンソール)
---------------------------------------------*/

#include	<winsock2.h>
#include	<stdio.h>
#include	"wsock32error.h"

#pragma comment (lib, "ws2_32.lib")
#pragma warning (disable:4996)

//#define TCP_MODE (0)
#define TCP_MODE (1)
//------------------------------------
// マクロ
//------------------------------------
#define		PORTNO		30250		

void main(){
	int		sts;
	int		errcode;
	WSADATA		m_wd;					// ＷＳＡＳＴＡＲＴＵＰ用

	WORD	requiredversion;
	SOCKET	mysock;				// ソケット番号	外から終了させるため。
	sockaddr_in toaddr;				// 送信先アドレス
	int			len;				// 送信先アドレス長

	char*	senddata = "クライアントからの送信データ";

	char	ip_str[256];		// IPアドレス取得用バッファ

	// このプログラムが要求するバージョン
	requiredversion = MAKEWORD(2, 2);

	// ＷＩＮＳＯＣＫ初期化
	sts = WSAStartup(MAKEWORD(2, 2), &m_wd);
	if (sts != 0)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return;
	}
	// バージョンチェック
	if (m_wd.wVersion != requiredversion)
	{
		printf("Version Error\n");
		return;
	}

#if TCP_MODE
	// ソケット作成(TCP用のソケット作成)
	mysock = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
#else
	// ソケット作成(UDP用のソケット作成)
	mysock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
#endif
	if (mysock == INVALID_SOCKET)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return;
	}

	//送信先IPアドレス取得
	printf("IPアドレスを入力して下さい。\n");
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

	// 宛先の設定
	toaddr.sin_port = htons(PORTNO);
	toaddr.sin_family = AF_INET;
	toaddr.sin_addr.s_addr = inet_addr(ip_str);

	// 受信
	len = sizeof(sockaddr);	// 送信元アドレス長

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
	//TCPでは相手を変更できるsendtoは使えない.
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
		//recvも使えない
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
			printf("受信データ : %s \n", rcvbuf);
		}

	}

	printf("何かキーを押すと終了します。\n");
	rewind(stdin);
	getchar();

	// ソケットをクローズ
	sts = closesocket(mysock);	//ソケットをクローズする。
	//受信スレッドでエラーが発生してwhileを抜ける。
	if (sts == SOCKET_ERROR)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
	}

	// ＷＩＮＳＯＣＫの後処理
	WSACleanup();

}
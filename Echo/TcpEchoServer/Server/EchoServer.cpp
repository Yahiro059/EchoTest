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
	//初期化
	WSADATA wsa;
	WORD verno;
	int sts;
	std::thread th_handle;

	//WinSock2初期化
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
	//TCPソケット
	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
#else
	//UDPソケット
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
#endif

#if !TCP_MODE
	//マルチキャストグループへの参加
	ip_mreq mreq; //マルチキャストアドレス用構造体
	mreq.imr_multiaddr.S_un.S_addr = inet_addr(GROUP_ADDR);
	mreq.imr_interface.S_un.S_addr = INADDR_ANY;

	//登録
	sts = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
	if (sts == SOCKET_ERROR){
		errcom(WSAGetLastError());
	}
#endif

	th_handle = std::thread(recvThread);
	//スレッドが起動していたらgetchar()で待期
	if (th_handle.joinable()){
		getchar();
		//SOCKETクローズ
		closesocket(sock);
		th_handle.join();
	}

	//WinSock終了
	WSACleanup();
}//main

void recvThread(){
	int sts;

	//ポート番号の設定(bind)
	sockaddr_in myaddr;
	int addr_len;

	//アドレスの設定
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORT_NO);
	myaddr.sin_addr.S_un.S_addr = INADDR_ANY;
	//sin_addrは使うネットワークポートのアドレス
	//INADDR_ANYは０で、「全て」の意味になる

	//sockに自アドレスを登録(Serverは必ず行う)
	addr_len = sizeof(sockaddr_in); //アドレスサイズ
	sts = bind(sock, (sockaddr*)&myaddr, addr_len);

	if (sts == SOCKET_ERROR){
		errcom(WSAGetLastError());
		return;
	}

#if TCP_MODE
	//listen
	listen(sock, SOMAXCONN);
	//第2引数は最大待ち受数.接続ではない.
	//listenでconect待ちスタート
#endif

	//受信
	char recvBuff[256];
	sockaddr_in fromaddr;
	char recvData[] = "サーバーからの送信データ";
	//受信ループ
	while (true){
#if TCP_MODE
		//SOCKET宣言
		SOCKET cl_sock;	//通信用ソケット
		//accept
		cl_sock = accept(sock, (sockaddr*)&fromaddr, &addr_len);
		//conectの返答、これで通信用ソケットが生成される.
		//conectが来るまでスレッドが止まる
		//新しいユーザが増えるとソケットも増える
		//エラーチェック
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

		//データ表示
		printf("受信データ：%s\n", recvBuff);

		//データが送信されたことの確認として確認用データを返す.
#if TCP_MODE
		//send
		
		sts = send(cl_sock, recvData,strlen(recvData) + 1,0);
		//cl_sockを使うことに注意
		if (sts == SOCKET_ERROR)
		{
			errcom(WSAGetLastError());
			return;
		}
		//クライアントソケットのclose
		//Echoだとここで相手の全ての全通信が終了なので
		closesocket(cl_sock);
		//もし、closeなしで終了すると次のループでcl_socketが上書きされて
		//前ループのcl_socketがメモリリークになる.
		
#else
		sts = sendto(sock, recvBuff, strlen(recvBuff) + 1, 0, (sockaddr*)&fromaddr, addr_len);
#endif

		if (sts == SOCKET_ERROR){
			errcom(WSAGetLastError());
			break;
		}
	}//while
}

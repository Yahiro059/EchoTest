#ifndef __GN31_WINSOCK32_ERROR_POPUP__
#define __GN31_WINSOCK32_ERROR_POPUP__

#include <winsock2.h>
#pragma comment (lib, "wsock32.lib")

struct errdata_t{
	int	eno;					// �G���[�R�[�h
	char* errmsg;				// �G���[���b�Z�[�W
};

void errcom(int errcode);

#endif


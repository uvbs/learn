/*
* sendBuffer.h
*
*  Created on: 2010-2-15
*      Author: lichong
*/

#ifndef SENDBUFFER_H_
#define SENDBUFFER_H_
#include "common.h"
#include "CLogger.h"

//======================================================================================
// EveryBlock size 128K, Max buffer size 8M

const size_t BLOCK_SIZE = (8*1024 - 64) ;
const size_t MAX_BLOCKS = ((64*1024*1024) / BLOCK_SIZE);
class SendBuffer{

	struct Block{
		struct Block *next;
		size_t dataSize;
		size_t bufferLeft;
		char *dataFront;
		char *dataTail;
		char buffer[BLOCK_SIZE];
		Block()
		{
			next = NULL;
			dataSize = 0;
			bufferLeft = BLOCK_SIZE;
			dataFront = buffer;
			dataTail = buffer;
		}
		~Block()
		{
		}
	};
	typedef struct Block Block_t;

	Block_t *pFirstBlock;
	Block_t *pLastBlock;
	uint32_t blockNum;
public:
	size_t dataLeft;
	bool bEncrypto;
	RC4_KEY rc4;
	uint32_t max_blocks;

	SendBuffer();
	~SendBuffer();
	int empty() const;
	uint32_t block();
	void setRC4Key(const unsigned char *data, size_t len);
	int flush(int socket);
	int pushData(const char * msg, size_t size);
};
#endif /* SENDBUFFER_H_ */

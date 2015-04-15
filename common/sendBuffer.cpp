/*
* sendBuffer.cpp
*
*  Created on: 2010-2-15
*      Author: lichong
*/
#include <openssl/rc4.h>
#include <openssl/sha.h>
#include <winsock2.h>

#include "sendBuffer.h"
#include "string.h"

SendBuffer::SendBuffer()
{
	pFirstBlock = new Block;
	pLastBlock = pFirstBlock;
	//log(Debug, "SendBuffer sizeof(Block)=%u", sizeof(struct Block)); //test
	blockNum = 1;
	dataLeft = 0;
	max_blocks = MAX_BLOCKS;
	bEncrypto = false;
}

SendBuffer::~SendBuffer()
{
	while(pFirstBlock){
		Block_t *p = pFirstBlock->next;
		dataLeft -= pFirstBlock->dataSize; //test
		blockNum--; //test
		delete pFirstBlock;
		pFirstBlock = p;
	}
	pLastBlock = NULL;
	if(dataLeft != 0 || blockNum != 0){ //test
		log(Error, "SendBuffer free not clean: dataLeft=%ld,blockNum=%ld!", dataLeft, blockNum);
	}
}

int SendBuffer::empty() const{
	return !dataLeft;
}

uint32_t SendBuffer::block(){
	return blockNum;
}

int SendBuffer::flush(int socket)
{
	int sended = 0;

	while(1){
		int sendSize = pFirstBlock->dataSize;
		if(sendSize == 0){
			if(pFirstBlock->next != NULL){
				//log(Warn, "head not clean num=%u", blockNum);
				Block_t *p = pFirstBlock->next;
				delete pFirstBlock;
				blockNum--;
				pFirstBlock = p;
				sendSize = pFirstBlock->dataSize;
			}else{
				break;
			}
		}
		int nsent = ::send(socket, (const char*)pFirstBlock->dataFront, sendSize, 0);
		if(nsent < 0){
			if(errno == 11){//Resource temporarily unavailable
				log(Warn, "SendBuffer::flush nsent=%u errno=%u, %s,", nsent, errno, strerror(errno));
			}else{
				log(Error, "SendBuffer::flush nsent=%u errno=%u, %s,", nsent, errno, strerror(errno));
			}
			nsent = 0;
		}

		dataLeft -= nsent;
		pFirstBlock->dataFront += nsent;
		pFirstBlock->dataSize -= nsent;
		sended += nsent;
		if(nsent<sendSize){
			break;
		}

		if(blockNum == 1){
			break;
		}
		//log(Debug, "SendBuffer::flush free block %u", blockNum);
		Block_t *p = pFirstBlock->next;
		if(p == NULL){
			break;
		}
		delete pFirstBlock;
		blockNum--;
		pFirstBlock = p;
	}
	return sended;
}

int SendBuffer::pushData(const char * msg, size_t size)
{
	//log(Info, "SendBuffer::pushData size=%u", size);
	if (size == 0){
		log(Error, "SendBuffer::pushData, size=0");
		return -1;
	}
	if(dataLeft + size > BLOCK_SIZE * (MAX_BLOCKS-1)){
		//log(Warn, "SendBuffer::pushData out of buffer, Left=%ld, write=%ld", dataLeft, size);
		return -2;
	}

	while(size > 0){
		if(pLastBlock->bufferLeft == 0){
			Block_t *p = new Block;
			pLastBlock->next = p;
			pLastBlock = p;
			blockNum++;
			//log(Info, "SendBuffer::pushData add block num %u", blockNum);
			if(blockNum > MAX_BLOCKS){ //test
				log(Error, "SendBuffer::pushData out of MAX_BLOCKS, blockNum=%u", blockNum);
			}
		}

		size_t sizeToPush = pLastBlock->bufferLeft>size?size:pLastBlock->bufferLeft;
		if(bEncrypto){
			RC4(&rc4, sizeToPush, (unsigned char *)msg, (unsigned char *)pLastBlock->dataTail);
		}else{
			memcpy(pLastBlock->dataTail, msg, sizeToPush);
		}
		pLastBlock->bufferLeft -= sizeToPush;
		pLastBlock->dataSize += sizeToPush;
		pLastBlock->dataTail += sizeToPush;
		dataLeft += sizeToPush;
		msg += sizeToPush;
		size -= sizeToPush;
	}
	return 0;
}

void SendBuffer::setRC4Key(const unsigned char *data, size_t len){
	RC4_set_key(&rc4, len, data);
	bEncrypto = true;
}

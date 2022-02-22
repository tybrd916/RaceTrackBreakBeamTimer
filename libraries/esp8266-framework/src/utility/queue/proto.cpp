#include "proto.h"
#include "ringbuf.h"
char PROTO_Init(PROTO_PARSER *parser, PROTO_PARSE_CALLBACK *completeCallback, unsigned char *buf, uint16_t bufSize)
{
    parser->buf = buf;
    parser->bufSize = bufSize;
    parser->dataLen = 0;
    parser->callback = completeCallback;
    parser->isEsc = 0;
    return 0;
}

char PROTO_ParseByte(PROTO_PARSER *parser, unsigned char value)
{
	switch(value){
		case 0x7D:
			parser->isEsc = 1;
			break;

		case 0x7E:
			parser->dataLen = 0;
			parser->isEsc = 0;
			parser->isBegin = 1;
			break;

		case 0x7F:
			if (parser->callback != NULL)
				parser->callback();
			parser->isBegin = 0;
			return 0;
			break;

		default:
			if(parser->isBegin == 0) break;

			if(parser->isEsc){
				value ^= 0x20;
				parser->isEsc = 0;
			}

			if(parser->dataLen < parser->bufSize)
				parser->buf[parser->dataLen++] = value;

			break;
	}
    return -1;
}

char PROTO_Parse(PROTO_PARSER *parser, unsigned char *buf, uint16_t len)
{
    while(len--)
        PROTO_ParseByte(parser, *buf++);

    return 0;
}
int PROTO_ParseRb(RINGBUF* rb, unsigned char *bufOut, uint16_t* len, uint16_t maxBufLen)
{
	unsigned char c;

	PROTO_PARSER proto;
	PROTO_Init(&proto, NULL, bufOut, maxBufLen);
	while(RINGBUF_Get(rb, &c) == 0){
		if(PROTO_ParseByte(&proto, c) == 0){
			*len = proto.dataLen;
			return 0;
		}
	}
	return -1;
}
int PROTO_Add(unsigned char *buf, const unsigned char *packet, int bufSize)
{
    uint16_t i = 2;
    uint16_t len = *(uint16_t*) packet;

    if (bufSize < 1) return -1;

    *buf++ = 0x7E;
    bufSize--;

    while (len--) {
        switch (*packet) {
        case 0x7D:
        case 0x7E:
        case 0x7F:
            if (bufSize < 2) return -1;
            *buf++ = 0x7D;
            *buf++ = *packet++ ^ 0x20;
            i += 2;
            bufSize -= 2;
            break;
        default:
            if (bufSize < 1) return -1;
            *buf++ = *packet++;
            i++;
            bufSize--;
            break;
        }
    }

    if (bufSize < 1) return -1;
    *buf++ = 0x7F;

    return i;
}

int PROTO_AddRb(RINGBUF *rb, const unsigned char *packet, int len)
{
    uint16_t i = 2;
    if(RINGBUF_Put(rb, 0x7E) == -1) return -1;
    while (len--) {
        switch (*packet) {
        case 0x7D:
        case 0x7E:
        case 0x7F:
        	if(RINGBUF_Put(rb, 0x7D) == -1) return -1;
        	if(RINGBUF_Put(rb, *packet++ ^ 0x20) == -1) return -1;
            i += 2;
            break;
        default:
        	if(RINGBUF_Put(rb, *packet++) == -1) return -1;
            i++;
            break;
        }
    }
    if(RINGBUF_Put(rb, 0x7F) == -1) return -1;

    return i;
}

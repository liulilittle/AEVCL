#include "rtp.h"
#include <sstream>
#include "..\mempool\mempool.h"

#define EDIAN_ENABLE_SETTING

long long edian_btol(char* buf, int ofs, int size)
{
#ifdef EDIAN_ENABLE_SETTING
	long long value = 0;
	if (buf != NULL) {
		char* ptr = (char*)&value;
		for (int i = 0; i < size; i++)
			ptr[i] = buf[i];
	}
	return value;
#else
	long num = 0;
	if (buf != NULL) {
		char* ptr = (char*)&num;
		size--;
		for (int i = 0; i <= size; i++)
			ptr[i] = buf[size - i];
	}
	return num;
#endif
}

char* edian_ltob(long num, int size)
{
	if (size <= 0)
		return NULL;
	char* buf = (char*)mempool_alloc(size);
	char* ptr = (char*)&num;
#ifdef EDIAN_ENABLE_SETTING
	for (int i = 0; i < size; i++)
		buf[i] = ptr[i];
	return buf;
#else
	size--;
	for (int i = 0; i <= size; i++)
		buf[size - i] = ptr[i];
	return buf;
#endif
}

template<typename T>
T edian_ltob_g(long num, int size)
{
	T* p = (T*)edian_ltob(num, size);
	T v = *p;
	mempool_free(p);
	return v;
}

long edian_flipbit(long num, int start, int end)
{
#ifdef EDIAN_ENABLE_SETTING
	return num;
#else 
	long value = 0;
	for (int i = start; i < end; i++) {
		value <<= 1;
		value |= (num >> i) & 1;
	}
	return value;
#endif
}

RTP_HEADER* rtp_header_parse(char* buf, int size)
{
	if (buf == NULL || size < RTP_HEADER_SIZE)
		return NULL;
	char* low = buf;
	long stx = edian_btol(buf, 0, 4);
	if (stx != RTP_HEADER_STX_FLAGS)
		return NULL;
	RTP_HEADER* header = (RTP_HEADER*)mempool_alloc(sizeof(RTP_HEADER));
	memset(header, 0x00, sizeof(RTP_HEADER));
	//
	header->stx = stx;
	buf += 4;
	int eax = edian_flipbit(*buf, 0, 8);
	header->cc = eax & 0x0F; // 低四位
	//
	eax = (eax & 0xF0) >> 4; // 高四位
	header->x = eax & 1;
	header->p = (eax >> 2) & 1;
	header->v = (eax >> 2);
	buf += 1;
	//
	eax = edian_flipbit(*buf, 0, 8);
	header->pt = (eax & 0x7F);
	header->m = (eax >> 7) & 1;
	buf += 1;
	//
	header->seq = edian_btol(buf, 0, 2); // *++buf | *++buf << 8
	buf += 2;
	memcpy(header->sim, buf, 6);
	buf += 6;
	header->channel = *buf;
	buf += 1;
	//
	eax = edian_flipbit(*buf, 0, 8);
	header->pack = eax & 0x0F;
	header->type = (eax & 0xF0) >> 4;
	buf += 1;
	//
	header->type_i = (header->type == 0);
	header->type_p = (header->type == 1);
	header->type_b = (header->type == 2);
	header->type_a = (header->type == 3);
	header->type_t = (header->type == 4);
	//
	/*if (header->type_i | header->type_p | header->type_b | header->type_a) {*/
	header->ts = edian_btol(buf, 0, 8);
	buf += 8;
	/*if (!header->type_a) {*/
	header->invl_i = edian_btol(buf, 0, 2);
	buf += 2;
	header->invl_p = edian_btol(buf, 0, 2);
	buf += 2;
	//}
	//}
	header->pack_a = (header->pack == 0);
	header->pack_f = (header->pack == 1);
	header->pack_l = (header->pack == 2);
	header->pack_m = (header->pack == 3);
	//
	header->size = edian_btol(buf, 0, 2);
	buf += 2;
	header->body.size = (size - (buf - low));
	header->body.buf = (char*)mempool_alloc(header->size);
	if (header->body.size > 0)
		memcpy(header->body.buf, buf, header->body.size);
	return header;
}

char* rtp_header_to(RTP_HEADER* header, int* len)
{
	if (header == NULL)
		return NULL;
	std::stringstream s;
	char* ltob = edian_ltob(header->stx, 4);
	s.write(ltob, 4);
	mempool_free(ltob);
	//
	int eax = header->v << 2 | header->p << 1 | header->x;
	eax = eax << 4 | header->cc;
	s << edian_ltob_g<char>(edian_flipbit(eax, 0, 8), 1);
	//
	eax = header->m << 7 | header->pt;
	s << edian_ltob_g<char>(edian_flipbit(eax, 0, 8), 1);
	//
	ltob = edian_ltob(header->seq, 2);
	s.write(ltob, 2);
	mempool_free(ltob);
	//
	s.write(header->sim, 6);
	s.put(header->channel);
	//
	eax = header->type << 4 | header->pack;
	s << edian_ltob_g<char>(edian_flipbit(eax, 0, 8), 1);
	//
	/*if (header->type_i | header->type_p | header->type_b | header->type_a) {*/
	ltob = edian_ltob(header->ts, 8);
	s.write(ltob, 8);
	mempool_free(ltob);
	//
	//if (!header->type_a) {
	ltob = edian_ltob(header->invl_i, 2);
	s.write(ltob, 2);
	mempool_free(ltob);
	//
	ltob = edian_ltob(header->invl_p, 2);
	s.write(ltob, 2);
	mempool_free(ltob);
	//	}
	//}
	ltob = edian_ltob(header->size, 2);
	s.write(ltob, 2);
	mempool_free(ltob);
	//
	if (header->body.buf != NULL)
		s.write(header->body.buf, header->body.size);
	if (len != NULL)
		*len = s.tellp();
	char* nat = (char*)mempool_alloc(*len);
	s.read(nat, *len);
	return nat;
}

bool rtp_header_default(RTP_HEADER* rtp)
{
	if (rtp == NULL)
		return FALSE;
	memset(rtp, 0x00, sizeof(RTP_HEADER));
	rtp->stx = RTP_HEADER_STX_FLAGS;
	return TRUE;
}

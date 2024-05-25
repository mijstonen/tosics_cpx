#!/usr/bin/env cpx
#|


#!= gen_crc32table.cpp 20


uint32_t crc32_block_endian0(uint32_t val, const void *buf, unsigned len, uint32_t *crc_table)
{
    const void *end = (uint8_t*)buf + len;

    while (buf != end) {
        val = crc_table[ (uint8_t)val ^ *(uint8_t*)buf ] ^ (val >> 8);
        buf = (uint8_t*)buf + 1;
    }
    return val;
}

#(

char TestBuf[]= {"Dit is crc test data"};

INFO(VARVAL(crc32_block_endian0(0,TestBuf,( sizeof TestBuf ),little_endian_table)));
#)

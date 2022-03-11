#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"

Queue::Queue()
{
	head = 0;
	tail = 0;
}


Queue::~Queue()
{
}


uint16_t Queue::data_size(void)
{
    if(head >= tail)
        return head - tail;
    else
        return head + BUF_SIZE - tail;
}

uint16_t Queue::free_size(void)
{
    return BUF_SIZE - data_size();
}

int Queue::add_queue(const uint8_t *ptr, uint16_t len)
{
    if(len > free_size())       
        return -1;
    
    if(head+len > BUF_SIZE){
        memcpy(&data[head], ptr, BUF_SIZE-head);
        memcpy(data, ptr+BUF_SIZE-head, len- BUF_SIZE+head);
    }
    else
        memcpy(&data[head], ptr, len);    
    head += len;
    if(head >= BUF_SIZE)
        head -= BUF_SIZE;
    return len;
}


int Queue::get_queue(uint8_t *ptr, uint16_t len)
{
    uint16_t qlen = data_size();
    if(len > qlen)
        len = qlen;
    if(len == 0)
        return 0;
    
    if(tail+len > BUF_SIZE){
        memcpy(ptr, &data[tail], BUF_SIZE-tail);
        memcpy(ptr+BUF_SIZE-tail, data, len- BUF_SIZE+tail);
    }
    else
        memcpy( ptr, &data[tail],len);    
    tail += len;
    if(tail >= BUF_SIZE)
        tail -= BUF_SIZE;

    return len;
}

int Queue::peek_queue(uint8_t *ptr, uint16_t len)
{
    uint16_t qlen = data_size();
    if(len > qlen)
        len = qlen;
    if(len == 0)
        return 0;
    
    if(tail+len > BUF_SIZE){
        memcpy(ptr, &data[tail], BUF_SIZE-tail);
        memcpy(ptr+BUF_SIZE-tail, data, len- BUF_SIZE+tail);
    }
    else
        memcpy(ptr, &data[tail],len);    
    return len;
}


void Queue::clear_queue(void)
{
	head = tail = 0;
}

int Queue::sync_queue(const uint8_t *ptr, uint16_t len)
{
    uint16_t qlen = data_size();
	if(len > qlen)
		return -1;

	for(int n=0, ok=0; n<qlen; n++){
		uint8_t ch = data[(tail+n)%BUF_SIZE];
		ok = (ch==ptr[ok])? ok+1 : 0;
		if(ok == len){
			tail = (tail+n-(len-1))%BUF_SIZE;
			return 1;
		}
	}
	return -1;
}



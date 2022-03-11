#ifndef __QUEUE_H__
#define __QUEUE_H__

#define BUF_SIZE	1024

class Queue
{
public:
	Queue();
	~Queue();

	int add_queue(const uint8_t *, uint16_t len);
	int get_queue(uint8_t *, uint16_t len);
	int peek_queue(uint8_t *ptr, uint16_t len);
	int sync_queue(const uint8_t *ptr, uint16_t len);
	void clear_queue(void);
	uint16_t data_size(void);
	uint16_t free_size(void);

private:
	uint16_t head;
	uint16_t tail;
	uint8_t  data[BUF_SIZE];
};
        
#endif

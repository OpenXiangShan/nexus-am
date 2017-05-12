#include "common.h"
#include "burst.h"
#include "misc.h"
#include <time.h>
#include <stdlib.h>

#define CACHE_WAYS (1 << 3)
#define CACHE_GROUPS (1 << 7)
#define CACHE2_GROUPS (1 << 12)
#define CACHE2_WAYS (1 << 4)
#define DATA_LEN (1 << 6)

void dram_write(hwaddr_t addr, size_t len, uint32_t data);
uint32_t dram_read(hwaddr_t addr, size_t len);

typedef struct {
	uint32_t tag : 14;
	uint8_t data[64];          
	uint32_t valid : 1;	
} CB; //definition of L1 cache block

typedef struct {
	uint32_t tag : 9;
	uint8_t data[64];
	uint32_t valid : 1;
	uint32_t dirty : 1;
} CB2; //definition of L2 cache block

typedef union {
	struct {
		uint32_t offset : 6;
		uint32_t index : 7;
		uint32_t tag : 14;
	};
	struct {
		uint32_t offset2 : 6;
		uint32_t index2 : 12;
		uint32_t tag2 : 9;
	};
	uint32_t addr;
} dram_addr;

typedef struct {
	CB cache_blocks[CACHE_GROUPS][CACHE_WAYS];
} cache;

typedef struct {
	CB2 cache_blocks[CACHE2_GROUPS][CACHE2_WAYS];
} cache2;

cache L1_cache;
cache2 L2_cache;

void init_cache() {
	/*initialize the cache(set all flag bits to invalid)*/
	int i,j;
	for(i = 0; i < CACHE_GROUPS; i++)
	{
		for(j = 0; j < CACHE_WAYS; j++)
			L1_cache.cache_blocks[i][j].valid = 0x0;
	}
	for(i = 0; i < CACHE2_GROUPS; i++)
	{
		for(j = 0; j < CACHE2_WAYS; j++)
		{
			L2_cache.cache_blocks[i][j].valid = 0x0;
			L2_cache.cache_blocks[i][j].dirty = 0x0;
		}
	}
}

static void cache3_read(hwaddr_t addr, void *data) {
	dram_addr temp;
	temp.addr = addr;
	
	uint32_t index = temp.index;
	uint32_t index2 = temp.index2;
	uint32_t offset2 = temp.offset2;
	int i,j,k;/*i for cache1 j for cache2 k for data*/

	for(i = 0; i < CACHE_WAYS; i++)
	{
		if(L1_cache.cache_blocks[index][i].tag == temp.tag && L1_cache.cache_blocks[index][i].valid == 0x1)
		{
			/*return data*/
			memcpy(data,L1_cache.cache_blocks[index][i].data,DATA_LEN);
			return;
		}
	}

	for(i = 0; i < CACHE_WAYS; i++)
	{
		/*locate a cache block to use*/
		if(L1_cache.cache_blocks[index][i].valid == 0x0)
		{
			/*read data from lower memory into cache block*/
			for(j = 0; j < CACHE2_WAYS; j++)
			{
				if(L2_cache.cache_blocks[index2][j].tag == temp.tag2 && L2_cache.cache_blocks[index2][j].valid == 0x1)
				{
					/*return data*/
					memcpy(L1_cache.cache_blocks[index][i].data,L2_cache.cache_blocks[index2][j].data,DATA_LEN);
					L1_cache.cache_blocks[index][i].tag = temp.tag;
					L1_cache.cache_blocks[index][i].valid = 0x1;
					memcpy(data,L1_cache.cache_blocks[index][i].data,DATA_LEN);
					return;
				}
			}
			for(j = 0;j < CACHE2_WAYS; j++)
			{
				/*locate a cache block to use*/
				if(L2_cache.cache_blocks[index2][j].valid == 0x0)
				{
					/* is it dirty*/
					if(L2_cache.cache_blocks[index2][j].dirty == 0x1)
					{
						/*write its previous data to lower memory*/
						for(k = 0;k < DATA_LEN; k++)
							dram_write(addr - offset2 + k,1,L2_cache.cache_blocks[index2][j].data[k]);
					}
					/*read data from lower memory into cache block*/
					for(k = 0;k < DATA_LEN; k++)
						L2_cache.cache_blocks[index2][j].data[k] = dram_read(addr - offset2 + k,1);
					L2_cache.cache_blocks[index2][j].tag = temp.tag2;
					L2_cache.cache_blocks[index2][j].valid = 0x1;
					/*make it clean*/
					L2_cache.cache_blocks[index2][j].dirty = 0x0;	
					/*return data*/
					memcpy(L1_cache.cache_blocks[index][i].data,L2_cache.cache_blocks[index2][j].data,DATA_LEN);
					L1_cache.cache_blocks[index][i].tag = temp.tag;
					L1_cache.cache_blocks[index][i].valid = 0x1;
					memcpy(data,L1_cache.cache_blocks[index][i].data,DATA_LEN);
					return;
				}
			}
			/*locate a cache block to use*/
			srand(time(0));
			int row2 = rand()%16;
			/* is it dirty*/
			if(L2_cache.cache_blocks[index2][row2].dirty == 0x1)
			{
				/*write its previous data to lower memory*/
				for(k = 0;k < DATA_LEN; k++)
					dram_write(addr - offset2 + k,1,L2_cache.cache_blocks[index2][row2].data[k]);
			}
			/*read data from lower memory into cache block*/
			for(k = 0;k < DATA_LEN; k++)
				L2_cache.cache_blocks[index2][row2].data[k] = dram_read(addr - offset2 + k,1);
			L2_cache.cache_blocks[index2][row2].tag = temp.tag2;
			L2_cache.cache_blocks[index2][row2].valid = 0x1;
			/*make it clean*/
			L2_cache.cache_blocks[index2][row2].dirty = 0x0;
			/*return data*/
			memcpy(L1_cache.cache_blocks[index][i].data,L2_cache.cache_blocks[index2][row2].data,DATA_LEN);
			L1_cache.cache_blocks[index][i].tag = temp.tag;
			L1_cache.cache_blocks[index][i].valid = 0x1;
			memcpy(data,L1_cache.cache_blocks[index][i].data,DATA_LEN);
			return;
		}
	}
	/*locate a cache block to use*/
	srand(time(0));
	int row1 = rand()%8;
	/*read data from lower memory*/
	for(j = 0; j < CACHE2_WAYS; j++)
	{
		if(L2_cache.cache_blocks[index2][j].tag == temp.tag2 && L2_cache.cache_blocks[index2][j].valid == 0x1)
		{
			/*return data*/
			memcpy(L1_cache.cache_blocks[index][row1].data,L2_cache.cache_blocks[index2][j].data,DATA_LEN);
			L1_cache.cache_blocks[index][row1].tag = temp.tag;
			L1_cache.cache_blocks[index][row1].valid = 0x1;
			memcpy(data,L1_cache.cache_blocks[index][row1].data,DATA_LEN);
			return;
		}
	}
	for(j = 0;j < CACHE2_WAYS; j++)
	{
		/*locate a cache block to use*/
		if(L2_cache.cache_blocks[index2][j].valid == 0x0)
		{
			/* is it dirty*/
			if(L2_cache.cache_blocks[index2][j].dirty == 0x1)
			{
				/*write its previous data to lower memory*/
				for(k = 0;k < DATA_LEN; k++)
					dram_write(addr - offset2 + k,1,L2_cache.cache_blocks[index2][j].data[k]);
			}
			/*read data from lower memory into cache block*/
			for(k = 0;k < DATA_LEN; k++)
				L2_cache.cache_blocks[index2][j].data[k] = dram_read(addr - offset2 + k,1);
			L2_cache.cache_blocks[index2][j].tag = temp.tag2;
			L2_cache.cache_blocks[index2][j].valid = 0x1;
			/*make it clean*/
			L2_cache.cache_blocks[index2][j].dirty = 0x0;	
			/*return data*/
			memcpy(L1_cache.cache_blocks[index][row1].data,L2_cache.cache_blocks[index2][j].data,DATA_LEN);
			L1_cache.cache_blocks[index][row1].tag = temp.tag;
			L1_cache.cache_blocks[index][row1].valid = 0x1;
			memcpy(data,L1_cache.cache_blocks[index][row1].data,DATA_LEN);
			return;
		}
	}
	/*locate a cache block to use*/
	srand(time(0));
	int row2 = rand()%16;
	/* is it dirty*/
	if(L2_cache.cache_blocks[index2][row2].dirty == 0x1)
	{
		/*write its previous data to lower memory*/
		for(k = 0;k < DATA_LEN; k++)
			dram_write(addr - offset2 + k,1,L2_cache.cache_blocks[index2][row2].data[k]);
	}
	/*read data from lower memory into cache block*/
	for(k = 0;k < DATA_LEN; k++)
		L2_cache.cache_blocks[index2][row2].data[k] = dram_read(addr - offset2 + k,1);
	L2_cache.cache_blocks[index2][row2].tag = temp.tag2;
	L2_cache.cache_blocks[index2][row2].valid = 0x1;
	/*make it clean*/
	L2_cache.cache_blocks[index2][row2].dirty = 0x0;
	/*return data*/
	memcpy(L1_cache.cache_blocks[index][row1].data,L2_cache.cache_blocks[index2][row2].data,DATA_LEN);
	L1_cache.cache_blocks[index][row1].tag = temp.tag;
	L1_cache.cache_blocks[index][row1].valid = 0x1;
	memcpy(data,L1_cache.cache_blocks[index][row1].data,DATA_LEN);
	return;
}

uint32_t cache_read(hwaddr_t addr, size_t len) {
	dram_addr ad;
	ad.addr = addr ;
	uint32_t offset = ad.offset;
	uint8_t temp[2 * DATA_LEN];
	cache3_read(addr, temp);
	if(offset + len > DATA_LEN) 
	{
		// data cross the cache boundary 
		cache3_read(addr + DATA_LEN, temp + DATA_LEN);
	}
	return unalign_rw(temp + offset, 4);
}

//---------------------------------------------------read has been completed-----------------------------------------------------------------------//


static void cache3_write(hwaddr_t addr, void *data, uint8_t *mask) {
	dram_addr temp;
	temp.addr = addr;
	
	uint32_t index = temp.index;
	uint32_t index2 = temp.index2;
	uint32_t offset2 = temp.offset2;
	int i,j,k;/*i for cache1 j for cache2 k for data*/
	
	for(i = 0; i < CACHE_WAYS; i++)
	{
		if(L1_cache.cache_blocks[index][i].tag == temp.tag)
		{
			/*write data to cache block*/
			memcpy_with_mask(L1_cache.cache_blocks[index][i].data,data,DATA_LEN,mask);
			L1_cache.cache_blocks[index][i].valid = 0x1;
			break;
		}
	}
	/*write data to lower memory*/
	for(j = 0; j < CACHE2_WAYS; j++)
	{
		if(L2_cache.cache_blocks[index2][j].tag == temp.tag2)
		{
			/*write the new data to cache block*/
			memcpy_with_mask(L2_cache.cache_blocks[index2][j].data,data,DATA_LEN,mask);
			L2_cache.cache_blocks[index2][j].valid = 0x1;
			/*make it dirty*/
			L2_cache.cache_blocks[index2][j].dirty = 0x1;
			return;
		}
	}
	for(j = 0; j < CACHE2_WAYS; j++)
	{
		/*locate a cache block to use*/
		if(L2_cache.cache_blocks[index2][j].valid == 0x0)
		{
			/* is it dirty*/
			if(L2_cache.cache_blocks[index2][j].dirty == 0x1)
			{
				/*write its previous data to lower memory*/
				for(k = 0;k < DATA_LEN; k++)
					dram_write(addr - offset2 + k,1,L2_cache.cache_blocks[index2][j].data[k]);
			}
			/*read data from lower memory into cache block*/
			for(k = 0;k < DATA_LEN; k++)
				L2_cache.cache_blocks[index2][j].data[k] = dram_read(addr - offset2 + k,1);
			/*write the new data to cache block*/
			memcpy_with_mask(L2_cache.cache_blocks[index2][j].data,data,DATA_LEN,mask);
			L2_cache.cache_blocks[index2][j].tag = temp.tag2;
			L2_cache.cache_blocks[index2][j].valid = 0x1;
			/*make it dirty*/
			L2_cache.cache_blocks[index2][j].dirty = 0x1;
			return;	
		}
	}
	/*locate a cache block to use*/
	srand(time(0));
	int row2 = rand()%16;
	/* is it dirty*/
	if(L2_cache.cache_blocks[index2][row2].dirty == 0x1)
	{
		/*write its previous data to lower memory*/
		for(k = 0;k < DATA_LEN; k++)
			dram_write(addr - offset2 + k,1,L2_cache.cache_blocks[index2][row2].data[k]);
	}
	/*read data from lower memory into cache block*/
	for(k = 0;k < DATA_LEN; k++)
		L2_cache.cache_blocks[index2][row2].data[k] = dram_read(addr - offset2 + k,1);
	/*write the new data to cache block*/
	memcpy_with_mask(L2_cache.cache_blocks[index2][row2].data,data,DATA_LEN,mask);
	L2_cache.cache_blocks[index2][row2].tag = temp.tag2;
	L2_cache.cache_blocks[index2][row2].valid = 0x1;
	/*make it dirty*/
	L2_cache.cache_blocks[index2][row2].dirty = 0x1;
	return;	
}

void cache_write(hwaddr_t addr, size_t len, uint32_t data) {
	dram_addr ad;
	ad.addr = addr;
	uint32_t offset = ad.offset;
	uint8_t temp[2 * DATA_LEN];
	uint8_t mask[2 * DATA_LEN];
	memset(mask, 0, 2 * DATA_LEN);
	*(uint32_t *)(temp + offset) = data;
	memset(mask + offset, 1, len);
	cache3_write(addr, temp, mask);
	if(offset + len > DATA_LEN) 
	{
		/* data cross the cache boundary */
		cache3_write(addr + DATA_LEN, temp + DATA_LEN, mask + DATA_LEN);
	}
}


#ifndef _BCACHE_H
#define _BCACHE_H

#define BITMASK(name, type, field, offset, size)		\
static inline uint64_t name(const type *k)			\
{ return (k->field >> offset) & ~(((uint64_t) ~0) << size); }	\
								\
static inline void SET_##name(type *k, uint64_t v)		\
{								\
	k->field &= ~(~((uint64_t) ~0 << size) << offset);	\
	k->field |= v << offset;				\
}

static const char bcache_magic[] = {
	0xc6, 0x85, 0x73, 0xf6, 0x4e, 0x1a, 0x45, 0xca,
	0x82, 0x65, 0xf5, 0x7f, 0x48, 0xba, 0x6d, 0x81 };

/* Version 1: Backing dev
 * Version 2: Seed pointer into btree node checksum
 * Version 3: Backing dev superblock has offset of start of data
 */

#define BCACHE_SB_BDEV_VERSION	3
#define BCACHE_SB_MAX_VERSION	3

#define SB_SECTOR		8
#define SB_LABEL_SIZE		32
#define BDEV_DATA_START		16	/* sectors */


struct cache_sb {
	uint64_t		csum;
	uint64_t		offset;	/* sector where this sb was written */
	uint64_t		version;
#define CACHE_BACKING_DEV	1

	uint8_t			magic[16];

	uint8_t			uuid[16];
	union {
		uint8_t		set_uuid[16];
		uint64_t	set_magic;
	};
	uint8_t			label[SB_LABEL_SIZE];

	uint64_t		flags;
	uint64_t		seq;
	uint64_t		pad[8];

	uint64_t		nbuckets;	/* device size */
	uint16_t		block_size;	/* sectors */
	uint16_t		bucket_size;	/* sectors */

	uint16_t		nr_in_set;
	uint16_t		nr_this_dev;

	uint32_t		last_mount;	/* time_t */

	uint16_t		first_bucket;
	uint16_t		keys;		/* number of journal buckets */
	uint64_t		d[];		/* journal buckets */
};

BITMASK(SB_BDEV,	struct cache_sb, version, 0, 1);

BITMASK(BDEV_WRITEBACK,	struct cache_sb, flags, 0, 1);

BITMASK(CACHE_DISCARD,	struct cache_sb, flags, 1, 1);
BITMASK(CACHE_REPLACEMENT, struct cache_sb, flags, 2, 3);

inline uint64_t crc64(const void *_data, size_t len);

#define node(i, j)		((void *) ((i)->d + (j)))
#define end(i)			node(i, (i)->keys)

#define csum_set(i)							\
	crc64(((void *) (i)) + 8, ((void *) end(i)) - (((void *) (i)) + 8))

#endif

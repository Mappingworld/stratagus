#ifndef ETLIB_HASH_H
#define ETLIB_HASH_H

#ifndef ETLIB_GENERIC_H
#include <etlib/generic.h>
#endif

struct hash_st
{
    int nelem;
    int hashsize;
    int maxdepth;
    int middepth;
};

#ifdef _MSC_VER
typedef void hash_no_data;
#else
typedef struct { } hash_no_data;
#endif

extern void *_hash_get(u8 *id, void *table, int size, int usize);
extern void *_hash_find(u8 *id, void *table, int size, int usize);
extern void  _hash_del(u8 *id, void *table, int size, int usize);
extern void _hash_stat(void *table, int size, struct hash_st *stat_buffer);

#ifdef _MSC_VER

#define hash_get(tab, id)	_hash_get(id, (tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#define hash_find(tab, id)	_hash_find(id,(tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#else

#define hash_get(tab, id)	(typeof((tab).table[0]->user)*) \
    _hash_get(id, (tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#define hash_find(tab, id)	(typeof((tab).table[0]->user)*) \
    _hash_find(id,(tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#endif

#define hash_del(tab, id)	\
    _hash_del(id, (tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#define hash_name(tab, sym) (((u8 *)sym) + sizeof((tab).table[0]->user) + 1)

#define hash_stat(tab, st) _hash_stat((tab).table, NELEM((tab).table), st)

#define hash_add(tab, id) hash_get(tab, id)

#define hashtable(type, size) struct	\
{					\
    struct				\
    {					\
	void *left;			\
	void *right;			\
	type user;			\
	u8 name[2];			\
    } *table[size];			\
}

#endif /* ETLIB_HASH_H */

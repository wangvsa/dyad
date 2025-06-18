#ifndef DYAD_CACHE_CLIENT_H
#define DYAD_CACHE_CLIENT_H

int cache_add_file(const char* fname, size_t len);
int cache_evict_file(const char* fname);
int cache_access_file(const char* fname);

#endif

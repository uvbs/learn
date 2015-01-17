/*************************************************************************
    > File Name: client.h
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Sat 17 Jan 2015 04:33:31 PM CST
 ************************************************************************/

#ifndef _CLIENT_H__
#define _CLIENT_H__

#include <libmemcached/memcached.h>

#include <iostream>
#include <string>
#include <time.h>

class Client
{
public:
    Client() {
        memcached_return rc;
        memcached_server_st * server = NULL;

        memc = memcached_create(NULL);
        server = memcached_server_list_append(server, "127.0.0.1", 11211, &rc);
        rc = memcached_server_push(memc, server);

        if (MEMCACHED_SUCCESS != rc) {
            std::cout << "push failed" << std::endl;
        }

        memcached_server_list_free(server);
    }

    ~Client() {
        memcached_free(memc);
    }

    int insert(const char * key, const char * value, time_t expiration = 3) {
        if (key == NULL || value == NULL) {
            return -1;
        }

        uint32_t flag = 0;
        memcached_return rc;

        rc = memcached_set(memc, key, strlen(key), value, strlen(value), expiration, flag);

        if (MEMCACHED_SUCCESS == rc) {
            return 0;
        } else {
            return -1;
        }
    }

    std::string get(const char * key) {
        if (key == NULL) {
            return "";
        }

        uint32_t flag = 0;
        memcached_return rc;
        size_t vbytes = 0;

        char * value = memcached_get(memc, key, strlen(key), &vbytes, &flag, &rc);

        if (rc == MEMCACHED_SUCCESS) {
            return value;
        } else {
            return "";
        }
    }
private:
    memcached_st * memc;
};

#endif


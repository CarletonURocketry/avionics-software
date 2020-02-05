/**
 * @file service_watchdog.h
 * @desc Manage services and watch for errors
 * @author Quinn Parrott
 * @date 2020-02-01
 * Last Author:
 * Last Edited On:
 */

#ifndef service_watchdog_h
#define service_watchdog_h

#include "global.h"


typedef struct service_t {
    void* storage;
    void* (*call)(void*);
    //uint8_t (*status_callback)(void*);
} service_t;

/**
 * Entrypoint for the loop that calls all service.
 *
 * @param services An array containing the definitions of all the services.
 * @param services_size The size of services.
 */
extern void service_loop(service_t *services, uint8_t services_size);

#endif /* service_watchdog_h */

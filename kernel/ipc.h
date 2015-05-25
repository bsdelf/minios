#ifndef IPC_H
#define IPC_H

#include <types.h>

typedef struct {
    uint32 sender;
    uint32 port;
    uint32 type;
    uint32 length;
} message_t;

int ipc_send(const message_t* msg);
void ipc_receive(message_t* msg);

#endif

//#ifdef CONFIG_EVM_MODULE_NET
#include "evm_module.h"
#include <rtthread.h>
#include <sys/socket.h>
#include "netdb.h"

#define _NET_LISTENER_DEFAULT_SIZE  8
static evm_val_t *_net_listener_registry;

typedef struct _net_sock_t {
    struct sockaddr_in addr;
    int sockfd;
    int listener_id;
} _net_sock_t;

static void _net_server_thread(_net_sock_t *server_sock) {
    _net_sock_t *client_sock;
    while(1) {
        client_sock = evm_malloc(sizeof(_net_sock_t));
        if( client_sock ) {
            client_sock->sockfd = accept(server_sock->sockfd, (struct sockaddr *)&client_sock->addr, sizeof(struct sockaddr_in));
            if (client_sock->sockfd < 0)
            {
                evm_print("accept connection failed! errno = %d\r\n", errno);
                evm_free(client_sock);
                continue;
            }
        }
    }
}

static int _net_listener_add(evm_t *e) {
    for(uint32_t i = 0; i < evm_list_len(_net_listener_registry); i++){
        evm_val_t *v = evm_list_get(e, _net_listener_registry, i);
        if( evm_is_undefined(v) ) {
            evm_val_t *obj = evm_object_create(e, GC_OBJECT, 0, 0);
            if( obj ) {
                evm_list_pushvalue(e, _net_listener_registry, i, obj);
                return i;
            } else {
                return -1;
            }
        }
    }
    return -1;
}

static void _net_listener_remove(evm_t *e, int id) {
    evm_list_set(e, _net_listener_registry, id, EVM_VAL_UNDEFINED);
}

static evm_val_t *_net_listener_get(evm_t *e, int id) {
    return evm_list_get(e, _net_listener_registry, id);
}

//server.close([closeListener])
static evm_val_t evm_module_net_server_close(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    _net_sock_t *server_sock = evm_object_get_ext_data(p);
    if( !server_sock )
        return EVM_VAL_UNDEFINED;
    closesocket(server_sock->sockfd);
	return EVM_VAL_UNDEFINED;
}

//server.listen(port[, host][, backlog][, listenListener])
//server.listen(options[, listenListener])
static evm_val_t evm_module_net_server_listen(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    if( argc < 1 || !evm_is_integer(v) ) {
        return EVM_VAL_UNDEFINED;
    }

    _net_sock_t *server_sock = evm_object_get_ext_data(p);
    if( !server_sock )
        return EVM_VAL_UNDEFINED;

    server_sock->addr.sin_family = AF_INET;
    server_sock->addr.sin_port = htons(evm_2_integer(v));
    server_sock->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(server_sock->sockfd, (struct sockaddr*)&server_sock->addr, sizeof(struct sockaddr_in)) < 0 )
    {
        evm_print("Failed to bind server socket\r\n");
        closesocket(server_sock->sockfd);
        return EVM_VAL_UNDEFINED;
    }

    if( listen(server_sock->sockfd, 1) < 0 ) {
        printf("Failed to listen\r\n");
        closesocket(server_sock->sockfd);
    }

    server_sock->listener_id = _net_listener_add(e);

    rt_thread_t tid = rt_thread_create("_net_server_thread", _net_server_thread, server_sock, 1536, 26, 2);
    if (tid) {
        rt_thread_startup(tid);
    }
	return EVM_VAL_UNDEFINED;
}

//server.on(event, callback)
static evm_val_t evm_module_net_server_on(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    if( argc < 2 || !evm_is_string(v) || !evm_is_script(v + 1) ) {
        return EVM_VAL_UNDEFINED;
    }

    _net_sock_t *server_sock = evm_object_get_ext_data(p);
    if( !server_sock )
        return EVM_VAL_UNDEFINED;
    
    if( server_sock->listener_id == -1 )
        return EVM_VAL_UNDEFINED;

    evm_val_t *listener = _net_listener_get(e, server_sock->listener_id);
    if( !listener )
        return EVM_VAL_UNDEFINED;

    evm_prop_append(e, listener, evm_2_string(v), *(v + 1));
	return EVM_VAL_UNDEFINED;
}

//socket.connect(options[, connectListener])
//socket.connect(port[, host][, connectListener])
static evm_val_t evm_module_net_socket_connect(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    _net_sock_t *sock = evm_object_get_ext_data(p);
    if( !sock )
        return EVM_VAL_UNDEFINED;

    if( argc > 2 && evm_is_integer(v) && evm_is_string(v + 1) ) {
        sock->addr.sin_family = AF_INET;
        sock->addr.sin_port = htons(evm_2_integer(v));
        sock->addr.sin_addr.s_addr = inet_addr(evm_2_string(v + 1));
        rt_memset(&(sock->addr.sin_zero), 0, sizeof(sock->addr.sin_zero));
    } else {
        return EVM_VAL_UNDEFINED;
    }


    if ((sock->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        evm_print("Socket create failed.");
        return EVM_VAL_UNDEFINED;
    }


    if ( connect(sock->sockfd, (struct sockaddr *)&(sock->addr), sizeof(struct sockaddr)) == -1 ) {
        evm_print("socket connect failed!");
        closesocket(sock->sockfd);
        return EVM_VAL_UNDEFINED;
    }

    if( argc > 2 && evm_is_script(v + 2) ) {
        evm_run_callback(e, v + 2, &e->scope, NULL, 0);
    }

    sock->listener_id = _net_listener_add(e);

	return EVM_VAL_UNDEFINED;
}

//socket.destroy()
static evm_val_t evm_module_net_socket_destroy(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	_net_sock_t *sock = evm_object_get_ext_data(p);
    if( !sock )
        return EVM_VAL_UNDEFINED;
    closesocket(sock->sockfd);
    return EVM_VAL_UNDEFINED;
}

//socket.end([data][, callback])
static evm_val_t evm_module_net_socket_end(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    _net_sock_t *sock = evm_object_get_ext_data(p);
    if( !sock )
        return EVM_VAL_UNDEFINED;
    if( argc > 0 ) {
        if( evm_is_string(v) ) {
            send(sock->sockfd, evm_2_string(v), evm_string_len(v), 0);
        } else if( evm_is_buffer(v) ) {
            send(sock->sockfd, evm_buffer_addr(v), evm_buffer_len(v), 0);
        }
    }

    if( argc > 1 && evm_is_script(v + 1) ) {
        evm_run_callback(e, v + 2, NULL, NULL, 0);
    }

    closesocket(sock->sockfd);
	return EVM_VAL_UNDEFINED;
}

//socket.pause()
static evm_val_t evm_module_net_socket_pause(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	return EVM_VAL_UNDEFINED;
}

//socket.resume()
static evm_val_t evm_module_net_socket_resume(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	return EVM_VAL_UNDEFINED;
}

//socket.setKeepAlive([enable][, initialDelay])
static evm_val_t evm_module_net_socket_setKeepAlive(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    _net_sock_t *sock = evm_object_get_ext_data(p);
    if( !sock )
        return EVM_VAL_UNDEFINED;
    if( argc > 0 && evm_is_boolean(v) ) {
        if( evm_is_true(v) ) {
            int keepAlive = 1;
            setsockopt(sock->sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
        } else {
            int keepAlive = 0;
            setsockopt(sock->sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
        }
    }
	return EVM_VAL_UNDEFINED;
}

//socket.setTimeout(timeout[, callback])
static evm_val_t evm_module_net_socket_setTimeout(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	return EVM_VAL_UNDEFINED;
}

//socket.write(data[, callback])
static evm_val_t evm_module_net_socket_write(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    _net_sock_t *sock = evm_object_get_ext_data(p);
    if( !sock )
        return EVM_VAL_UNDEFINED;
    if( argc > 0 ) {
        if( evm_is_string(v) ) {
            send(sock->sockfd, evm_2_string(v), evm_string_len(v), 0);
        } else if( evm_is_buffer(v) ) {
            send(sock->sockfd, evm_buffer_addr(v), evm_buffer_len(v), 0);
        }
    }
	return EVM_VAL_UNDEFINED;
}

//socket.on(event, callback)
static evm_val_t evm_module_net_socket_on(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    _net_sock_t *sock = evm_object_get_ext_data(p);
    if( !sock )
        return EVM_VAL_UNDEFINED;

    evm_val_t *listener = _net_listener_get(e, sock->listener_id);
    if( !listener )
        return EVM_VAL_UNDEFINED;
    
    evm_prop_append(e, listener, evm_2_string(v), *(v + 1));
	return EVM_VAL_UNDEFINED;
}

//net.connect(options[, connectListener])
//net.connect(port[, host][, connectListener])
static evm_val_t evm_module_net_connect(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	return EVM_VAL_UNDEFINED;
}

//net.createConnection(options[, connectListener])
//net.createConnection(port[, host][, connectListener])
static evm_val_t evm_module_net_createConnection(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	return EVM_VAL_UNDEFINED;
}

//net.createServer([options][, connectionListener])
static evm_val_t evm_module_net_createServer(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
	evm_val_t *server_obj = NULL;
    _net_sock_t *server_sock;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0 ) {
        evm_print("Failed to create socket\r\n");
        return EVM_VAL_UNDEFINED;
    }
    
    server_obj = evm_object_create(e, GC_OBJECT, 3, 0);
    if( !server_obj ) {
        closesocket(sockfd);
        return EVM_VAL_UNDEFINED;
    }
    evm_prop_append(e, server_obj, "close", evm_mk_native((intptr_t)evm_module_net_server_close));
    evm_prop_append(e, server_obj, "listen", evm_mk_native((intptr_t)evm_module_net_server_listen));
    evm_prop_append(e, server_obj, "on", evm_mk_native((intptr_t)evm_module_net_server_on));

    server_sock = evm_malloc(sizeof(_net_sock_t));
    if( !server_sock ) {
        evm_set_err(e, ec_memory, "Insufficient external memory");
        return EVM_VAL_UNDEFINED;
    }
    server_sock->sockfd = sockfd;

    evm_object_set_ext_data(server_obj, (intptr_t)server_sock);
    return *server_obj;
}

//new net.Socket([options])
static evm_val_t evm_module_net_socket_new(evm_t *e, evm_val_t *p, int argc, evm_val_t *v)
{
    evm_val_t *obj = evm_object_create(e, GC_DICT, 9, 0);
    if( obj ) {
        evm_prop_append(e, obj, "connect", evm_mk_native((intptr_t)evm_module_net_socket_connect));
        evm_prop_append(e, obj, "destroy", evm_mk_native((intptr_t)evm_module_net_socket_destroy));
        evm_prop_append(e, obj, "end", evm_mk_native((intptr_t)evm_module_net_socket_end));
        evm_prop_append(e, obj, "pause", evm_mk_native((intptr_t)evm_module_net_socket_pause));
        evm_prop_append(e, obj, "resume", evm_mk_native((intptr_t)evm_module_net_socket_resume));
        evm_prop_append(e, obj, "setKeepAlive", evm_mk_native((intptr_t)evm_module_net_socket_setKeepAlive));
        evm_prop_append(e, obj, "setTimeout", evm_mk_native((intptr_t)evm_module_net_socket_setTimeout));
        evm_prop_append(e, obj, "write", evm_mk_native((intptr_t)evm_module_net_socket_write));
        evm_prop_append(e, obj, "on", evm_mk_native((intptr_t)evm_module_net_socket_on));

        _net_sock_t *sock = evm_malloc(sizeof(_net_sock_t));
        if( sock ) {
            evm_object_set_ext_data(obj, (intptr_t)sock);
        }
        return *obj;
    }
	return EVM_VAL_UNDEFINED;
}

static evm_object_native_t _net_socket_native = {
    .creator = evm_module_net_socket_new,
};

evm_err_t evm_module_net(evm_t *e) {
    evm_val_t *socket = evm_native_function_create(e, &_net_socket_native, 0);
    if( !socket )
        return e->err;

	evm_builtin_t builtin[] = {
		{"connect", evm_mk_native((intptr_t)evm_module_net_connect)},
        {"createConnection", evm_mk_native((intptr_t)evm_module_net_createConnection)},
        {"createServer", evm_mk_native((intptr_t)evm_module_net_createServer)},
        {"Socket", *socket},
		{NULL, NULL}
	};
	evm_module_create(e, "net", builtin);

    _net_listener_registry = evm_list_create(e, GC_LIST, _NET_LISTENER_DEFAULT_SIZE);
	return e->err;
}
//#endif
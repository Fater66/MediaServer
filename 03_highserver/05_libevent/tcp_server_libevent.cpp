#include <iostream>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>

#include <arpa/inet.h>

#define PORT 8111

void on_read_cb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *input = NULL;
    struct evbuffer *output = NULL;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);
    evbuffer_add_buffer(output, input);
}

//回调函数
void on_accept_cb(struct evconnlistener *listener,
                  evutil_socket_t fd,
                  struct sockaddr *addr,
                  int socklen)
{
    struct event_base *base = NULL;
    struct bufferevent *bev = NULL;
    base = evconnlistener_get_base(listener);

    bev = bufferevent_socket_new(base, fd, 0);

    bufferevent_enable(bev, EV_READ | EV_WRITE);
    bufferevent_setcb(bev, on_read_cb, NULL, NULL, NULL);
}
int main(int argc, char *argv[])
{
    struct event_base *base = NULL;
    struct sockaddr_in serveraddr;
    // struct evconnlistener *listener = NULL;
    struct evconnlistener *listener;
    //分配并且返回一个新的具有默认设置的 event_base。
    //函数会检测环境变量,返回一个到 event_base 的指针。
    //如果发生错误,则返回 NULL。
    base = event_base_new();

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    // listener = evconnlistener_new_bind(base, on_accept_cb, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,(struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listener = evconnlistener_new_bind(base, on_accept_cb, NULL, LEV_OPT_REUSEABLE, 10, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    event_base_dispatch(base);
    return 0;
}
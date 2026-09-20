#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

struct socket_t;
struct rpc_registered_buffers;
typedef std::shared_ptr<socket_t> socket_ptr;

static constexpr size_t MAX_CHUNK_SIZE = 1024ull * 1024ull * 1024ull; // 1 GiB
static constexpr size_t RPC_CONN_CAPS_SIZE = 24;

struct socket_t {
    ~socket_t();

    bool send_data(const void * data, size_t size);
    bool recv_data(void * data, size_t size);
    // Must be called at every message boundary: the RDMA transport coalesces
    // writes into fixed-size frames and posts the trailing partial frame only
    // here. No-op on TCP.
    bool flush();

    bool supports_registered_exchange() const;
    // Buffers must remain allocated until the registration is released.
    std::shared_ptr<rpc_registered_buffers> register_buffers(void * send, void * recv, size_t size);
    // Both peers must call this. A null registration negotiates the buffered fallback.
    bool exchange_registered(const std::shared_ptr<rpc_registered_buffers> & buffers, size_t size, bool & exchanged);

    socket_ptr accept();

    void get_caps(uint8_t * local_caps);
    void update_caps(const uint8_t * remote_caps);

    static socket_ptr create_server(const char * host, int port);
    static socket_ptr connect(const char * host, int port);

private:
    struct impl;
    explicit socket_t(std::unique_ptr<impl> p);
    std::unique_ptr<impl> pimpl;
};

bool rpc_transport_init();
void rpc_transport_shutdown();

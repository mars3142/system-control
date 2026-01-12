#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    void dns_server_start(const char *ap_ip);
    void dns_set_ap_ip(const char *ip);

#ifdef __cplusplus
}
#endif

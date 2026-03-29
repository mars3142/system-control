#include "iris/iris_internal.h"
#include "esp_openthread_lock.h"

#if defined(CONFIG_IRIS_ENABLED)

static const char *TAG = "Iris";

/**
 * @brief Derive the Thread ML-EID (mesh-local address) for a device from its
 *        EUI-64. In a Thread network the mesh-local prefix is known from the
 *        network data; the IID is formed from the EUI-64 via EUI-64 → IID
 *        conversion (RFC 4291 modified EUI-64, toggle bit 6).
 *
 * This is a simplification — in production firmware the address should be
 * looked up from the Thread network data / neighbor table via otThreadGetNextNeighborInfo().
 */
bool eui64_to_ml_eid(const uint8_t eui64[IRIS_EUI64_LEN], otIp6Address *addr)
{
    esp_openthread_lock_acquire(portMAX_DELAY);

    otInstance *inst = esp_openthread_get_instance();
    if (!inst) {
        esp_openthread_lock_release();
        return false;
    }

    const otMeshLocalPrefix *prefix = otThreadGetMeshLocalPrefix(inst);
    if (!prefix) {
        esp_openthread_lock_release();
        return false;
    }

    memcpy(addr->mFields.m8, prefix->m8, 8);
    // EUI-64 → IID: copy bytes, toggle universal/local bit
    addr->mFields.m8[8]  = eui64[0] ^ 0x02;
    addr->mFields.m8[9]  = eui64[1];
    addr->mFields.m8[10] = eui64[2];
    addr->mFields.m8[11] = 0xFF;
    addr->mFields.m8[12] = 0xFE;
    addr->mFields.m8[13] = eui64[5];
    addr->mFields.m8[14] = eui64[6];
    addr->mFields.m8[15] = eui64[7];

    esp_openthread_lock_release();
    return true;
}

/**
 * @brief Context for the blocking CoAP GET helper.
 */
typedef struct {
    SemaphoreHandle_t done;
    char             *buf;
    size_t            buf_len;
    bool              success;
} coap_get_ctx_t;

static void coap_get_response_handler(void *ctx, otMessage *msg,
                                       const otMessageInfo *info, otError err)
{
    (void)info;
    coap_get_ctx_t *c = (coap_get_ctx_t *)ctx;
    if (err == OT_ERROR_NONE && msg) {
        uint16_t len = otMessageGetLength(msg) - otMessageGetOffset(msg);
        if (len >= c->buf_len) len = (uint16_t)(c->buf_len - 1);
        otMessageRead(msg, otMessageGetOffset(msg), c->buf, len);
        c->buf[len]  = '\0';
        c->success   = true;
    }
    xSemaphoreGive(c->done);
}

/**
 * @brief Simple blocking CoAP GET helper.
 *        Sends a GET request and waits up to 3 s for a response.
 *        Returns the JSON payload in @p out_buf (null-terminated).
 */
bool coap_get(const otIp6Address *addr, const char *resource,
              char *out_buf, size_t out_len)
{
    esp_openthread_lock_acquire(portMAX_DELAY);

    otInstance *inst = esp_openthread_get_instance();
    if (!inst) {
        esp_openthread_lock_release();
        return false;
    }

    otMessage *msg = otCoapNewMessage(inst, NULL);
    if (!msg) {
        esp_openthread_lock_release();
        return false;
    }

    otCoapMessageInit(msg, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);
    if (otCoapMessageAppendUriPathOptions(msg, resource) != OT_ERROR_NONE) {
        otMessageFree(msg);
        esp_openthread_lock_release();
        return false;
    }

    coap_get_ctx_t ctx = {
        .done    = xSemaphoreCreateBinary(),
        .buf     = out_buf,
        .buf_len = out_len,
        .success = false,
    };

    otMessageInfo info = {};
    info.mPeerAddr = *addr;
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    otError err = otCoapSendRequest(inst, msg, &info, coap_get_response_handler, &ctx);

    esp_openthread_lock_release();

    if (err != OT_ERROR_NONE) {
        vSemaphoreDelete(ctx.done);
        return false;
    }

    bool got = xSemaphoreTake(ctx.done, pdMS_TO_TICKS(3000));
    vSemaphoreDelete(ctx.done);
    return got && ctx.success;
}

bool coap_post(const otIp6Address *addr, const char *resource,
               const char *payload)
{
    esp_openthread_lock_acquire(portMAX_DELAY);

    otInstance *inst = esp_openthread_get_instance();
    if (!inst) {
        esp_openthread_lock_release();
        return false;
    }

    otMessage *msg = otCoapNewMessage(inst, NULL);
    if (!msg) {
        esp_openthread_lock_release();
        return false;
    }

    otCoapMessageInit(msg, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_POST);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);
    otCoapMessageAppendUriPathOptions(msg, resource);
    otCoapMessageAppendContentFormatOption(msg, OT_COAP_OPTION_CONTENT_FORMAT_JSON);
    otCoapMessageSetPayloadMarker(msg);
    otMessageAppend(msg, payload, (uint16_t)strlen(payload));

    otMessageInfo info = {};
    info.mPeerAddr = *addr;
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    otError err = otCoapSendRequest(inst, msg, &info, NULL, NULL);

    esp_openthread_lock_release();

    if (err != OT_ERROR_NONE) {
        ESP_LOGW(TAG, "CoAP POST failed: %d", err);
        return false;
    }
    return true;
}

#endif /* CONFIG_IRIS_ENABLED */

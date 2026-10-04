/**
 * @file nfc_tag.cc
 * @brief NFC tag service backed by the Zectrix board's tag chip
 */

#include "common/nfc_tag.h"

#include "boards/zectrix/zectrix_nfc.h"

#include <esp_log.h>

// Provided by the board support code.
extern "C" ZectrixNfc* ZectrixGetNfc();

namespace {
const char* kTag = "NfcTag";
}

bool nfc_tag_available() {
    return ZectrixGetNfc() != nullptr;
}

bool nfc_tag_write_uri(const std::string& url) {
    ZectrixNfc* nfc = ZectrixGetNfc();
    if (nfc == nullptr) {
        ESP_LOGW(kTag, "NFC unavailable; tag not updated");
        return false;
    }
    const bool was_powered = nfc->IsPowered();
    if (!was_powered && !nfc->PowerOn()) {
        ESP_LOGW(kTag, "NFC power-on failed; tag not updated");
        return false;
    }
    const esp_err_t err = nfc->WriteUriNdef(url);
    if (!was_powered) nfc->PowerOff();

    if (err != ESP_OK) {
        ESP_LOGE(kTag, "NFC tag write failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

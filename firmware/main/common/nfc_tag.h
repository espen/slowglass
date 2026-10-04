/**
 * @file nfc_tag.h
 * @brief Platform service: the device's passive NFC tag
 *
 * Thin wrapper over the board's NFC chip so widgets (e.g. the MakePlans
 * door sign) can keep the tag serving a URL without including board
 * headers. On hardware without a tag these are safe no-ops returning false.
 */

#ifndef COMMON_NFC_TAG_H
#define COMMON_NFC_TAG_H

#include <string>

/** True when NFC tag hardware is present and initialized. */
bool nfc_tag_available();

/**
 * @brief Write the tag's EEPROM to serve url as an NDEF URI record.
 *
 * Powers the chip on for the write and back off if it was off. The RF side
 * is phone-powered afterwards, so a written tag works through deep sleep.
 * Returns true on success.
 */
bool nfc_tag_write_uri(const std::string& url);

#endif  // COMMON_NFC_TAG_H

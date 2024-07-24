/*
 *   Copyright 2024 M Hightower
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
/*

Status Works - kinda - I don't have any devices that provide SFDP data with
optional parameters that have Product ID with Bank:MFG ID

*/
#include <Arduino.h>

#include <JedecJep106Lib.h>

#ifndef ETS_PRINTF
#define ETS_PRINTF ets_uart_printf
#endif
#ifndef NOINLINE
#define NOINLINE __attribute__((noinline))
#endif
#ifndef CONSOLE_PRINTF
#define CONSOLE_PRINTF(fmt, ...) Serial.printf_P(PSTR(fmt), ##__VA_ARGS__)
#endif

constexpr uint32_t kSfdpSignature = 0x50444653u; //'SFDP'


// The sum of bits for a valid MFG ID will be an odd value.
uint32_t parity8bits(uint32_t mfg_id) {
  size_t sum = 0;
  for (size_t i = 0; i < 8; i++) {
    if (0 != ((mfg_id >> i) & 1u)) sum++;
  }
  return sum & 1u;  // 1 => parity check passed
}

////////////////////////////////////////////////////////////////////////////////
// return 0 on no SFDP or too old OR the bank:mfg_id on success
struct BankMsgId {
  uint32_t mfg_id:8;
  uint32_t bank:8;
  uint32_t reserved:16;
};

BankMsgId getSfdpMfgId() {

  union SFDP_Hdr {
    struct {
      uint32_t signature:32;
      uint32_t rev_minor:8;
      uint32_t rev_major:8;
      uint32_t num_parm_hdrs:8;
      uint32_t access_protocol:8;
    };
    uint32_t u32[2];
  } sfdp_hdr;

  union SFDP_Param {
    struct {
      uint32_t id_lsb:8;
      uint32_t rev_minor:8;
      uint32_t rev_major:8;
      uint32_t num_dw:8;
      uint32_t tbl_ptr:24;
      uint32_t id_msb:8;
    };
    uint32_t u32[2];
  } sfdp_param;

  BankMsgId bankMfgId;
  bankMfgId.bank = 0u;
  bankMfgId.mfg_id = 0u;

  SpiOpResult ok0;
  size_t sz = sizeof(sfdp_hdr);
  size_t addr = 0u;

  ok0 = spi0_flash_read_sfdp(addr, &sfdp_hdr.u32[0], sz);
  if (SPI_RESULT_OK == ok0 && kSfdpSignature == sfdp_hdr.signature) {
    for (size_t i = 0u; i < (sfdp_hdr.num_parm_hdrs + 1u); i++) {
      addr += sz;
      sz = sizeof(sfdp_param);
      ok0 = spi0_flash_read_sfdp(addr, &sfdp_param.u32[0], sz);
      if (SPI_RESULT_OK == ok0) {
        if (1u > sfdp_param.rev_major && 6u > sfdp_param.rev_minor) {
          continue;
        }
        if (0xFFu == sfdp_param.id_msb && 0x00u == sfdp_param.id_lsb) {
          continue;
        } else {
          if (0x00u != sfdp_param.id_lsb && 1u == parity8bits(sfdp_param.id_lsb)) {
            bankMfgId.mfg_id = sfdp_param.id_lsb;
            bankMfgId.bank = sfdp_param.id_msb;
            return bankMfgId;
          }
        }
        // ETS_PRINTF("\nParameter Header #%u\n", i + 1);
        // ETS_PRINTF("  %-18s %d\n", "Num dwords", sfdp_param.num_dw);
        // ETS_PRINTF("  %-18s %u.%u\n", "Revision", sfdp_param.rev_major, sfdp_param.rev_minor);
        // ETS_PRINTF("  %-18s 0x%02X.%02X\n", "ID MSB.LSB", sfdp_param.id_msb, sfdp_param.id_lsb);
        // ETS_PRINTF("  %-18s 0x%08X\n", "TBL PTR", sfdp_param.tbl_ptr);
      }
    }
  }
  return bankMfgId;
}


void setup() {
  // Turn off bright LEDs
  pinMode(2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  delay(200);

  CONSOLE_PRINTF("\n\n\nSPI Flash Chip Info:\n");
  uint32_t chip_id = ESP.getFlashChipId();
  uint32_t mfg_id = chip_id & 0xFFu;
  uint32_t device_id = 0xFFFFu & (chip_id >> 8);

    uint32_t save_ps = xt_rsil(15);
    uint32_t flash_freq = SPI0C;
    uint32_t gpmux = GPMUX;
    xt_wsr_ps(save_ps);

  CONSOLE_PRINTF("  %-16s 0x%08X\n", "Flash Chip ID:", chip_id);
  CONSOLE_PRINTF("  %-16s %u\n", "Flash Size:", 1u << (device_id >> 8));
  CONSOLE_PRINTF("  %-16s 0x%08X\n", "GPMUX:", (gpmux & SPI0_CLK_EQU_SYS_CLK));
  flash_freq = 80 / ((flash_freq & 0x0Fu) + 1);
  CONSOLE_PRINTF("  %-16s %u MHz\n", "Freq:", flash_freq);

  BankMsgId bankMfgId = getSfdpMfgId();
  if (bankMfgId.bank) { // bank numbers start with 1
    const char *ven_str = Jep106GetManufacturerName(bankMfgId.mfg_id, bankMfgId.bank);
    if (NULL == ven_str) ven_str = "unknown, new vendor?";
    CONSOLE_PRINTF("  Bank:mfg_id %2u:0x%02X, %s\n", bankMfgId.bank, bankMfgId.mfg_id, ven_str);
  } else {
    size_t sum = parity8bits(mfg_id);
    if (sum) {
      CONSOLE_PRINTF("\nPossible Manufacturers:\n");
      const size_t max = Jep106GetManufacturerBankLimit();
      for (size_t j = 0; j < max; j++) {
        const char * ven_str = Jep106GetManufacturerName(mfg_id, j);
        if (ven_str) {
          CONSOLE_PRINTF("  Bank %2u, %s\n", (j + 1), ven_str);
        }
      }
    } else {
      vendor = "Invalid value (parity error)";
      CONSOLE_PRINTF("  %-16s 0x%02X, '%s'\n", "Manufacturer:", mfg_id, vendor);
    }
  }
}

void loop() {

}

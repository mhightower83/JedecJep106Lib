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
#ifndef JEDECJEP106LIB_H_
#define JEDECJEP106LIB_H_


// Build with -DJEDECJEP106LIB_IRAM_ATTRIB=1 if code is needed in IRAM

// Manufacturer ID, mfg_id, or Code
// bank or ContinuationBytes = { 0, 1, 2, ... 14 } zero based count for bank
//    number. The number of 0x7F that occurred before the mfg_id was seen.
const char* Jep106GetManufacturerName(uint8_t mfg_id, uint8_t bank);

// The number of mfg_id banks to search through.
size_t Jep106GetManufacturerBankLimit(void);

#endif

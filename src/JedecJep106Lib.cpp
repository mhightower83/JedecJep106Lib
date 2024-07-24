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
////////////////////////////////////////////////////////////////////////////////
//
// Wrapper for building "JedecJep106Lib.c" with the Arduino IDE
//
// File `JedecJep106Lib.c` was fetched on April 17, 2024 from
//   https://github.com/tianocore/edk2/blob/master/MdePkg/Library/JedecJep106Lib/JedecJep106Lib.c
//
// const char* Jep106GetManufacturerName(uint8_t Code, uint8_t ContinuationBytes);
// size_t Jep106GetManufacturerBankLimit(void);
//
// An ongoing issue/question:
//   How do we determine which bank a given SPI Flash Chip belongs to?
//
#include <Arduino.h>

// Defines needed to build "JedecJep106Lib.c"
//
#if JEDECJEP106LIB_IRAM_ATTRIB
#define EFIAPI IRAM_ATTR
#else
#define EFIAPI
#endif
#define GLOBAL_REMOVE_IF_UNREFERENCED
#define IN
#define STATIC static
#define CONST const
typedef uint8_t UINT8;
typedef char CHAR8;
typedef uint32_t UINTN;
#define ARRAY_SIZE( a ) ( sizeof( a ) / sizeof( uintptr_t) )

// Changes for "JedecJep106Lib.c"
// The original code in "JedecJep106Lib.c" must be bracketed with
// #ifdef JEDECJEP106LIB_C
// ...
// #endif
//
// and the original include lines must be commented out:
//   #include <Library/BaseLib.h>
//   #include <Library/DebugLib.h>
//
#define JEDECJEP106LIB_C
#include "JedecJep106Lib.c"


////////////////////////////////////////////////////////////////////////////////
//
size_t EFIAPI Jep106GetManufacturerBankLimit(void) {
  return ARRAY_SIZE (Jep106Manufacturers);
}

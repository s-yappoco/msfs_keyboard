#pragma once

// キーボードUID定義（ツールからはき出されたID）
#define VIAL_KEYBOARD_UID {0x7D, 0x94, 0x86, 0x3D, 0x23, 0x22, 0x5D, 0x89}

// セキュリティ アンロックキーの設定[0,0][1,1]
#define VIAL_UNLOCK_COMBO_ROWS { 0, 1 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

// PS5向け設定と書き換えモード設定で同じエリアを見られるようにする・・
//#ifndef VIA_EEPROM_LAYOUT_OPTIONS_SIZE
//#    define VIA_EEPROM_LAYOUT_OPTIONS_SIZE 1
//#endif
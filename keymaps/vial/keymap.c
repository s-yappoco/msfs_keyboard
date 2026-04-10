// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

////////////////// SIM rateの状況をLED3に出すための処理
// LEDのみの制御なので、実際のsim rate制御はゲーム内へキーコードを贈る必要がある。
// マクロで以下の色変更キーコードとゲームへ送信するキーコードを組み合わせておくと便利
// ミッションを始める前にKC_SIMRSTを発行して、基本速度に戻しておく

///////////////////////////////////////////////////////////////////// LED制御定義関連
// 点灯色定義
// 各レイヤー用の点灯色定義 (4番目のLEDのみを上書き) 
// RGBLIGHT_LAYER_SEGMENTS({LEDの開始位置, 個数, HSV色})
const rgblight_segment_t PROGMEM layer1_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_RED});
const rgblight_segment_t PROGMEM layer2_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_BLUE});
const rgblight_segment_t PROGMEM layer3_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_GREEN});
const rgblight_segment_t PROGMEM layer4_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_YELLOW});
const rgblight_segment_t PROGMEM layer5_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_MAGENTA});
const rgblight_segment_t PROGMEM layer6_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_CYAN});
const rgblight_segment_t PROGMEM layer7_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_ORANGE});
const rgblight_segment_t PROGMEM layer8_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_PURPLE});
const rgblight_segment_t PROGMEM layer9_indicator[]  = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_SPRINGGREEN});
const rgblight_segment_t PROGMEM layer10_indicator[] = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_TURQUOISE});
const rgblight_segment_t PROGMEM layer11_indicator[] = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_TEAL});
const rgblight_segment_t PROGMEM layer12_indicator[] = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_AZURE});
const rgblight_segment_t PROGMEM layer13_indicator[] = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_PURPLE}); 
const rgblight_segment_t PROGMEM layer14_indicator[] = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_CORAL}); 
const rgblight_segment_t PROGMEM layer15_indicator[] = RGBLIGHT_LAYER_SEGMENTS({4, 1, HSV_GOLD});

// SimRate インジケーター用 点灯色定義　全部で9色　(3番目のLEDのみを上書き) 
// RGBLIGHT_LAYER_SEGMENTS({LEDの開始位置, 個数, HSV色})
const rgblight_segment_t PROGMEM layer1_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_BLUE});        // -4: 極低速 (青)
const rgblight_segment_t PROGMEM layer2_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_CYAN});        // -3: 低速 (水色)
const rgblight_segment_t PROGMEM layer3_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_GREEN});       // -2: 中低速 (緑)
const rgblight_segment_t PROGMEM layer4_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_SPRINGGREEN}); // -1: 微低速 (黄緑)
const rgblight_segment_t PROGMEM layer5_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_WHITE});       //  0: 標準 (1x)
const rgblight_segment_t PROGMEM layer6_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_YELLOW});      //  1: 微高速 (黄)
const rgblight_segment_t PROGMEM layer7_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_ORANGE});      //  2: 中高速 (橙)
const rgblight_segment_t PROGMEM layer8_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_RED});         //  3: 高速 (赤)
const rgblight_segment_t PROGMEM layer9_simrateled[] = RGBLIGHT_LAYER_SEGMENTS({3, 1, HSV_MAGENTA});     //  4: 最高速 (ピンク/マゼンタ)


// sim_rateによる色変更用 カスタムキーコードの定義
enum my_keycodes {
    KC_SIMUP = QK_KB_0,			// SIMレートを1段階上げる（LEDの色設定のみ）
    KC_SIMDN,					// SIMレートを1段階下げる（LEDの色設定のみ）
    KC_SIMRST,					// SIMレートを基本速度に戻す（LEDの色設定のみ）
};

// --- 速度状態の管理変数 ---
static signed char sim_rate_level = 0;		// -15～15とする。0が通常速度


// RGB LIGHT制御 レイヤー定義
// RGB LIGHTはレイヤーでの制御となっていて、重ねて点灯制御できる。
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
	// キーアサインレイヤー毎にLED4を点灯させるためのレイヤー登録（15色）
    layer1_indicator,  // Index 0
    layer2_indicator,  // Index 1
    layer3_indicator,  // Index 2
    layer4_indicator,  // Index 3
    layer5_indicator,  // Index 4
    layer6_indicator,  // Index 5
    layer7_indicator,  // Index 6
    layer8_indicator,  // Index 7
    layer9_indicator,  // Index 8
    layer10_indicator, // Index 9
    layer11_indicator, // Index 10
    layer12_indicator, // Index 11
    layer13_indicator, // Index 12
    layer14_indicator, // Index 13
    layer15_indicator, // Index 14
	
	// SIMレート速度毎にLED3を点灯させるためのレイヤー登録（9色）
	layer1_simrateled, // Index 15
	layer2_simrateled, // Index 16
	layer3_simrateled, // Index 17
	layer4_simrateled, // Index 18
	layer5_simrateled, // Index 19
	layer6_simrateled, // Index 20
	layer7_simrateled, // Index 21
	layer8_simrateled, // Index 22
	layer9_simrateled  // Index 23	
);


///////////////////////////////////////////////////////////////////// キーマップ定義
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	
	[0] = LAYOUT(
        // 左側1列目 (SW1-SW6): ご指定の修正案
        LCTL(KC_1),     // SW1: AP (LC + 1) 
        LCTL(KC_F5),    // SW2: HDG (LC + F5)
        LCTL(KC_F7),    // SW3: NAV (LC + F7) 
        LCTL(KC_F3),    // SW4: APR (LC + F3)
        LCTL(KC_F8),    // SW5: VS (LC + F8) 
        LCTL(KC_F9),    // SW6: FLC (LC + F9)


        // 左側2列目 (SW8-SW13): ご指定の修正案 + 標準補完
        LCTL(KC_F),     // SW8: FD (L-Ctrl + F)
        LCTL(KC_F2),    // SW9: ALT (LC + F2)
        LCTL(KC_5),     // SW10: VNV (予備 LC + 5)
        LCTL(KC_6),     // SW11: BC (予備 LC + 6)
        LSFT(KC_R),     // SW12: ATHR (L-Shift + R)
        KC_N,           // SW13: NAV周波数スワップ (N)
		
        // シフトキー (SW7) レイヤー切り替え
        MO(1),

        // エンコーダ左側押し込み (RSW1S1-RSW4S1)
        LALT(KC_N),     // RSW1: NAV 1/2切り替え
        LCTL(KC_S),     // RSW2: ALT Sync
        LCTL(KC_F8),    // RSW3: VS (予備)
        LCTL(KC_END),   // RSW4: HDG Sync

        // 右側上半分 (FMS系: 衝突回避L-Alt+文字)
        LALT(KC_D),     // SW16: Direct-To
        LALT(KC_L),     // SW17: FPL
        KC_BSPC,        // SW21: CLR
        LALT(KC_M),     // SW25: MENU
        LALT(KC_P),     // SW26: PROC
        KC_ENT,         // SW22: ENT

        // 下段8つ (リクエスト配置 + 標準フラップアサイン)
        LALT(KC_UP),    // SW14: SIM + 
        LALT(KC_DOWN),  // SW18: SIM - 
        LALT(KC_I),     // SW23: CDI
        LSFT(KC_B),     // SW27: FLP - (フラップを上げる/格納)
        LSFT(KC_SPC),   // SW15: DFT (視点リセット)
        LSFT(KC_KP_1),  // SW19: PFD拡大
        LSFT(KC_KP_2),  // SW24: MFD拡大
        LSFT(KC_V),     // SW28: FLP + (フラップを下げる/展開)

        // エンコーダ右側押し込み
        LALT(KC_S),     // RSW5: Map Cursor
        LALT(KC_B),     // RSW6: BARO Set (B)
		LALT(KC_ENT),   // RSW7: FMS Cursor

        // ジョイスティック
        KC_ENT,         // ジョイスティック押し込み
        LCTL(KC_UP), 	// 上
		LCTL(KC_DOWN), 	// 下
		LCTL(KC_RIGHT), // 右
		LCTL(KC_LEFT),  // 左  

        // 書き込みスイッチ
        QK_BOOT         
    ),
 	[1] = LAYOUT(
        // レイヤー1: カスタムキーコードチェック
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_SIMUP, KC_SIMDN, KC_SIMRST, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        TO(2)			// キーチェック用レイヤーへ以降
    ),
	[2] = LAYOUT(
		// レイヤー2： キーチェック用
        // 左側1列目 (SW1-SW6)
        KC_1, KC_2, KC_3, KC_4, KC_5, KC_6,         // 左側2列目 (SW8-SW13)
        KC_7, KC_8, KC_9, KC_0, KC_A, KC_B,
        // シフトキー (SW7)レイヤー切り替え
        KC_C,
        // エンコーダ左側押し込み (RSW1S1-RSW4S1)
        KC_D, KC_E, KC_F, KC_G,
        // 右側上半分 (SW16, SW17, SW21, SW25, SW26, SW22)
        KC_H, KC_I, KC_J, KC_K, KC_L, KC_M,
        // 下段8つ (SW14, SW18, SW23, SW27, SW15, SW19, SW24, SW28)
        KC_N, KC_O, KC_P, KC_Q, KC_R, KC_S, KC_T, KC_U,
        // エンコーダ右側押し込み (RSW5S1-RSW7S1)
        KC_V, KC_W, KC_X,
        // ジョイスティック押し込み (JSTKSW)
        KC_Y,
		// ジョイスティック上下左右（実際はアナログだが仮想実装）
		KC_1, KC_2, KC_3, KC_4, 	
        // 書き込みスイッチ (SW20)
        TO(0) // デフォルトレイヤーへ戻る
    )
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t encoder_map[][NUM_ENCODERS][2] = {
    [0] = { 
        // 修飾キーを使わず、PS5が認識する単独キーを割り当て
        ENCODER_CCW_CW(KC_7, KC_8),         		// RSW1: NAV調整 (数字の7/8)
        ENCODER_CCW_CW(KC_KP_MINUS, KC_KP_PLUS), 	// RSW2: ALT調整 (テンキーの-/+) 
        ENCODER_CCW_CW(KC_KP_2, KC_KP_8),   		// RSW3: VS調整 (テンキーの2/8)
        ENCODER_CCW_CW(KC_KP_4, KC_KP_6),   		// RSW4: HDG調整 (テンキーの4/6)
        ENCODER_CCW_CW(KC_LBRC, KC_RBRC),   		// RSW5: Map Zoom ( [ / ] )
        ENCODER_CCW_CW(KC_COMMA, KC_DOT),   		// RSW6: FMS Big    ( , / . ) 
        ENCODER_CCW_CW(KC_F9, KC_F10)       		// RSW7: FMS little ( F9 / F10 )
    },
    [1] = { 
        // レイヤー1（MO1押下時）
        ENCODER_CCW_CW(KC_MINS, KC_EQL),    		// RSW1: NAV kHz ( - / = )
        ENCODER_CCW_CW(KC_PGDN, KC_PGUP),   		// RSW2: ALT 1000ft
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS), 
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS), 
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS), 
        ENCODER_CCW_CW(KC_HOME, KC_END),            // RSW6: CRS
        ENCODER_CCW_CW(KC_F11, KC_F12)              // RSW7: FMS Item ( / / \ )
    },
	[2] = { 
		// キーチェック用
        ENCODER_CCW_CW(KC_1, KC_2), // RSW1
        ENCODER_CCW_CW(KC_3, KC_4), // RSW2
        ENCODER_CCW_CW(KC_5, KC_6), // RSW3
        ENCODER_CCW_CW(KC_7, KC_8), // RSW4
        ENCODER_CCW_CW(KC_9, KC_0), // RSW5
        ENCODER_CCW_CW(KC_A, KC_B), // RSW6
        ENCODER_CCW_CW(KC_C, KC_D)  // RSW7
	}
};
#endif


///////////////////////////////////////////////////////////////////// 関数
// 起動時初期設定関数
void keyboard_post_init_user(void) {
	// カスタムLEDレイヤー定義をQMKのRGBシステムに登録
    rgblight_layers = my_rgb_layers;
	
}

// レイヤーが切り替わった時に呼ばれる関数
// レイヤー切り替わり時にLEDの色を変更します
layer_state_t layer_state_set_user(layer_state_t state) {
	
	// レイヤー0以外がアクティブかどうか
	bool is_not_base_layer = (state & ~0b0000000000000001);
	
    // 各LEDインジケーター層（Index 0-14）に対して、
    // 現在の最高レイヤーがその番号（1-15）と一致するかを判定してセットします。
    rgblight_set_layer_state(0,  layer_state_cmp(state,1));
    rgblight_set_layer_state(1,  layer_state_cmp(state,2));
    rgblight_set_layer_state(2,  layer_state_cmp(state,3));
    rgblight_set_layer_state(3,  layer_state_cmp(state,4));
    rgblight_set_layer_state(4,  layer_state_cmp(state,5));
    rgblight_set_layer_state(5,  layer_state_cmp(state,6));
    rgblight_set_layer_state(6,  layer_state_cmp(state,7));
    rgblight_set_layer_state(7,  layer_state_cmp(state,8));
    rgblight_set_layer_state(8,  layer_state_cmp(state,9));
    rgblight_set_layer_state(9,  layer_state_cmp(state,10));
    rgblight_set_layer_state(10, layer_state_cmp(state,11));
    rgblight_set_layer_state(11, layer_state_cmp(state,12));
    rgblight_set_layer_state(12, layer_state_cmp(state,13));
    rgblight_set_layer_state(13, layer_state_cmp(state,14));
    rgblight_set_layer_state(14, layer_state_cmp(state,15));
	
	// デフォルトレイヤー以外のいずれかのLayerがONのとき、
	// SIMレートレベル0（基本速度）の白色LEDを点灯
	//rgblight_set_layer_state(19, (sim_rate_level == 4) && (state > 1));
    rgblight_set_layer_state(19, (sim_rate_level == 0) && is_not_base_layer);
	
    return state;
}

// LED3にSIM Rateの状態を出力する関数
void update_sim_rate_led(void) {
	
	// 現在のレイヤー状態を取得
    layer_state_t current_state = layer_state;
	// レイヤー0以外がアクティブかどうか
	bool is_not_base_layer = (current_state & ~0b0000000000000001);	

	// 現在のSIMレートレベルに合わせてLED3点灯レイヤーをセットします。
    rgblight_set_layer_state(15, (sim_rate_level <= -4));
    rgblight_set_layer_state(16, (sim_rate_level == -3));
    rgblight_set_layer_state(17, (sim_rate_level == -2));
    rgblight_set_layer_state(18, (sim_rate_level == -1));
	
    rgblight_set_layer_state(19, (sim_rate_level == 0)&& is_not_base_layer);
	
    rgblight_set_layer_state(20, (sim_rate_level == 1));
    rgblight_set_layer_state(21, (sim_rate_level == 2));
	rgblight_set_layer_state(22, (sim_rate_level == 3));
    rgblight_set_layer_state(23, (sim_rate_level >= 4));
}

// ユーザー定義キーコードの処理（LED3変更用）
// --- キー入力時のsimrateレベル操作 ---
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_SIMUP:				// ユーザー定義SIMrate速度upキーコードがタップされた
            if (record->event.pressed) {
                sim_rate_level++;
                if (sim_rate_level > 15) sim_rate_level = 15;	// SIM rate 最大15とする
                update_sim_rate_led();	// LED更新関数呼び出し
            }
            return false;

        case KC_SIMDN:					// ユーザー定義SIMrate速度downキーコードがタップされた
            if (record->event.pressed) {
                sim_rate_level--;
                if (sim_rate_level < -15) sim_rate_level = -15; // SIM rate 最低-15とする
                update_sim_rate_led();  // LED更新関数呼び出し
            }
            return false;

        case KC_SIMRST:					// ユーザー定義SIMrate速度downキーコードがタップされた
            if (record->event.pressed) {
                sim_rate_level = 0; 	// デフォルト(白)
                update_sim_rate_led();  // LED更新関数呼び出し
            }
            return false;
            
        default:
            return true;
    }
}


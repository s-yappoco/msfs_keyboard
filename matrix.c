#include "matrix.h"
#include "quantum.h"

// ADCを動かすためのヘッダファイル
#include "analog.h"

// アナログスティックの読み取り設定
// 閾値を10bit(0-1023)用に調整
// 中央値512
// ヒステリシスを持たせている

#define STICK_THRESHOLD_HIGH 800
#define STICK_THRESHOLD_LOW  200
#define STICK_HYSTERESIS     50

// キーマトリックス接続ピン設定
static pin_t row_pins[] = {GP0, GP1, GP2, GP3};
static pin_t col_pins[] = {GP4, GP5, GP6, GP7, GP8};

// マトリックス初期化関数
void matrix_init_custom(void) {
    // 全てのピンを入力プルダウンに初期化
    for (uint8_t i = 0; i < 4; i++) {
        setPinInputLow(row_pins[i]);
    }
    for (uint8_t i = 0; i < 5; i++) {
        setPinInputLow(col_pins[i]);
    }	
}

// カスタムマトリックス読み取り関数を定義
// 日本式dualマトリックスを採用
// 基本はrow2col方式とし、row4,col5であるが、rowは逆方向にもあるので、0～7まで存在する。
//
// 実際にはスイッチのついていない以下のマトリックスは
// アナログスティックの読み取りに無理矢理割り当てるため、読み飛ばす処理を追加している
// UP [7, 0] DOWN [1, 3] LEFT [3, 3] RIGHT [2, 3]
	
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    // --- 前半スキャン: Row(出力High) -> Col(入力) ---
    for (uint8_t i = 0; i < 5; i++) setPinInputLow(col_pins[i]); 

    for (uint8_t r = 0; r < 4; r++) {
        setPinOutput(row_pins[r]);
        writePinHigh(row_pins[r]);
        wait_us(30); 

        matrix_row_t row_data = 0;
        for (uint8_t c = 0; c < 5; c++) {
			// スキップ対象: [1,3], [2,3], [3,3]
            if (c == 3 && (r == 1 || r == 2 || r == 3)) {
                // 現在のマトリクス値を維持（ADC判定側で上書きするため）
                if (current_matrix[r] & (1 << c)) {
                    row_data |= (1 << c);
                }
                continue;
            }
            if (readPin(col_pins[c])) {
                row_data |= (1 << c);
            }
        }

        if (current_matrix[r] != row_data) {
            current_matrix[r] = row_data;
            changed = true;
        }

        writePinLow(row_pins[r]);
        setPinInputLow(row_pins[r]);
        wait_us(30);
    }

    // --- 後半スキャン: Col(出力High) -> Row(入力) ---
    for (uint8_t i = 0; i < 4; i++) setPinInputLow(row_pins[i]); 

    for (uint8_t c = 0; c < 5; c++) {
        setPinOutput(col_pins[c]);
        writePinHigh(col_pins[c]);
        wait_us(30);

        for (uint8_t r = 0; r < 4; r++) {
			// スキップ対象: [3,0] (後半スキャンなので current_matrix[r+4] 側)
            // c=0, r=3 の組み合わせをスキップ
            if (c == 0 && r == 3) {
                continue; 
            }
			
            matrix_row_t old_val = current_matrix[r + 4];
            
            if (readPin(row_pins[r])) {
                current_matrix[r + 4] |= (1 << c); // 押されたらビットを立てる
            } else {
                current_matrix[r + 4] &= ~(1 << c); // 離されたらビットを下ろす(重要!)
            }

            if (old_val != current_matrix[r + 4]) {
                changed = true;
            }
        }

        writePinLow(col_pins[c]);
        setPinInputLow(col_pins[c]);
        wait_us(30);
    }

	///////////////////////////////////////////////////////// アナログスティック読み取り
	// アナログスティックの入力を上下左右の4つのキーコードへ変換する
	// 基板上であいているキーマトリックスへ無理矢理配置する。
	// これで、キーアサインが通常のキースイッチと同様に自由に設定できる
    // UP [7, 0]
    // DOWN [1, 3]
    // LEFT [3, 3]
    // RIGHT [2, 3]
	
	// --- ADC読み取り  ---
    int16_t val_y = analogReadPin(GP26); // 上下
    int16_t val_x = analogReadPin(GP27); // 左右
	
    // X軸をLED2へ デバッグモード
//  rgblight_setrgb_at(val_x >> 2, 0,   0, 2);
	// Y軸をLED3へ デバッグモード
//	rgblight_setrgb_at(val_y >> 2, 0,   0, 3);

    // --- Y軸判定 (UP / DOWN) ---
    // UP [7, 0]
    if (val_y > (STICK_THRESHOLD_HIGH)) {
        if (!(current_matrix[7] & (1 << 0))) {
			current_matrix[7] |= (1 << 0);
			changed = true;
		}
    } else if (val_y < (STICK_THRESHOLD_HIGH - STICK_HYSTERESIS)) {
        if (current_matrix[7] & (1 << 0)) { 
			current_matrix[7] &= ~(1 << 0); 
			changed = true;
		}
    }

    // DOWN [1, 3]
    if (val_y < (STICK_THRESHOLD_LOW)) {
        if (!(current_matrix[1] & (1 << 3))) {
			current_matrix[1] |= (1 << 3); 
			changed = true;
		}
    } else if (val_y > (STICK_THRESHOLD_LOW + STICK_HYSTERESIS)) {
        if (current_matrix[1] & (1 << 3)) {
			current_matrix[1] &= ~(1 << 3);
			changed = true;
		}
	}

    // --- X軸判定 (RIGHT / LEFT) ---
    // LEFT [3, 3]
    if (val_x > (STICK_THRESHOLD_HIGH)) {
        if (!(current_matrix[3] & (1 << 3))) { 
			current_matrix[3] |= (1 << 3);
			changed = true;
		}
    } else if (val_x < (STICK_THRESHOLD_HIGH - STICK_HYSTERESIS)) {
        if (current_matrix[3] & (1 << 3)) {
			current_matrix[3] &= ~(1 << 3);
			changed = true;
		}
    }

    // RIGHT [2, 3]
    if (val_x < (STICK_THRESHOLD_LOW)) {
        if (!(current_matrix[2] & (1 << 3))) {
			current_matrix[2] |= (1 << 3);
			changed = true;
		}
    } else if (val_x > (STICK_THRESHOLD_LOW + STICK_HYSTERESIS)) {
        if (current_matrix[2] & (1 << 3)) {
			current_matrix[2] &= ~(1 << 3);
			changed = true;
		}
    }
    return changed;
}

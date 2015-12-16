/****************************************************************************
 * (C) Tokyo Cosmos Electric, Inc. (TOCOS) - all rights reserved.
 *
 * Condition to use: (refer to detailed conditions in Japanese)
 *   - The full or part of source code is limited to use for TWE (TOCOS
 *     Wireless Engine) as compiled and flash programmed.
 *   - The full or part of source code is prohibited to distribute without
 *     permission from TOCOS.
 *
 * 利用条件:
 *   - 本ソースコードは、別途ソースコードライセンス記述が無い限り東京コスモス電機が著作権を
 *     保有しています。
 *   - 本ソースコードは、無保証・無サポートです。本ソースコードや生成物を用いたいかなる損害
 *     についても東京コスモス電機は保証致しません。不具合等の報告は歓迎いたします。
 *   - 本ソースコードは、東京コスモス電機が販売する TWE シリーズ上で実行する前提で公開
 *     しています。他のマイコン等への移植・流用は一部であっても出来ません。
 *
 ****************************************************************************/

// 詳細は remote_config.h を参照

#include <jendefs.h>

#include "utils.h"

#include "Interactive.h"
#include "EndDevice_Input.h"

#include "remote_config.h"

static void vProcessEvCore(tsEvent *pEv, teEvent eEvent, uint32 u32evarg);
static bool_t bTranmitRespond(uint32 u32AddrDst);

/**
 * アイドル状態。
 *
 * @param E_STATE_IDLE
 * @param pEv
 * @param eEvent
 * @param u32evarg
 */
PRSEV_HANDLER_DEF(E_STATE_IDLE, tsEvent *pEv, teEvent eEvent, uint32 u32evarg) {
  if (eEvent == E_EVENT_START_UP) {
    Interactive_vSetMode(TRUE,0);
    vSerInitMessage();
    vfPrintf(&sSerStream, LB LB "*** Entering Config Mode ***");
    // LED
    sAppData.u8LedState = 1; // ON
    // vfPrintf(&sSerStream, "%d", sizeof(tsFlashApp)); // 1.4.1 で 60 バイト

    // Mac 層の開始
    ToCoNet_vMacStart();
    ToCoNet_Event_SetState(pEv, E_STATE_RUNNING);
  }
}

/**
 * アイドル状態。
 *
 * @param E_STATE_RUNNING
 * @param pEv
 * @param eEvent
 * @param u32evarg
 */
PRSEV_HANDLER_DEF(E_STATE_RUNNING, tsEvent *pEv, teEvent eEvent, uint32 u32evarg) {
  ; // 特に何もしない
}

/**
 * イベント処理関数リスト
 */
static const tsToCoNet_Event_StateHandler asStateFuncTbl[] = {
  PRSEV_HANDLER_TBL_DEF(E_STATE_IDLE),
  PRSEV_HANDLER_TBL_DEF(E_STATE_RUNNING),
  PRSEV_HANDLER_TBL_TRM
};

/**
 * イベント処理関数
 * @param pEv
 * @param eEvent
 * @param u32evarg
 */
static void vProcessEvCore(tsEvent *pEv, teEvent eEvent, uint32 u32evarg) {
  ToCoNet_Event_StateExec(asStateFuncTbl, pEv, eEvent, u32evarg);
}


/**
 * メイン処理
 */
static void cbAppToCoNet_vMain() {
  /* handle serial input */
  vHandleSerialInput();
}

/**
 * ハードイベント（遅延割り込み処理）
 * @param u32DeviceId
 * @param u32ItemBitmap
 */
static void cbAppToCoNet_vHwEvent(uint32 u32DeviceId, uint32 u32ItemBitmap) {
  switch (u32DeviceId) {
  case E_AHI_DEVICE_TICK_TIMER:
    // LED の点滅を管理する
    _C {
      bool_t bLed1 = FALSE, bLed2 = FALSE;
      if (u32TickCount_ms - sAppData.u32LedTick < 1500) {
	if (sAppData.u8LedState & 1) {
	  bLed1 = TRUE;
	}
	if (sAppData.u8LedState & 2) {
	  bLed2 = TRUE;
	}
      }
      vPortSet_TrueAsLo(PORT_OUT1, bLed1);
      vAHI_DoSetDataOut(bLed2 ? 0 : 2, bLed2 ? 2 : 0);
    }
    break;
  default:
    break;
  }
}

/**
 * 受信イベント
 * @param pRx
 */
static void cbAppToCoNet_vRxEvent(tsRxDataApp *pRx) {
  int i;

  if (IS_APPCONF_OPT_VERBOSE()) {
    V_PRINTF(LB"RxPkt: Sr:%08X De:%08X Lq:%03d Ln:%02d Cm:%d Sq:%02x [",
	     pRx->u32SrcAddr,
	     pRx->u32DstAddr,
	     pRx->u8Lqi,
	     pRx->u8Len,
	     pRx->u8Cmd,
	     pRx->u8Seq
	     );
    for (i = 0; i < 16 && i < pRx->u8Len; i++) {
      V_PRINTF("%02X", pRx->auData[i]);
    }
    if (pRx->u8Len > i) {
      V_PRINTF("..");
    }
    V_PUTCHAR(']');
  }

  uint8 *p = pRx->auData;

  if (pRx->u8Cmd == RMTCNF_PKTCMD) {
    // *   OCTET    : パケットバージョン (1)
    uint8 u8pktver = G_OCTET();
    if (u8pktver != RMTCNF_PRTCL_VERSION) {
      V_PRINTF(LB"!PRTCL_VERSION");
      return;
    }

    // *   OCTET    : パケット種別 (0: 要求, 1: 応答, 2:ACK)
    uint8 u8pkttyp = G_OCTET();

    // *   OCTET(4) : アプリケーションバージョン
    uint32 u32ver = G_BE_DWORD();
    if (u32ver != VERSION_U32) {
      V_PRINTF(LB"!VERSION_U32");
      //			return;
    }

    // *   パケット種別 = 応答
    // *   OCTET    : 設定有効化 LQI
    if (pRx->u8Lqi < RMTCNF_MINLQI ) {
      V_PRINTF(LB"!LQI");
      return;
    }

    // 受信パケットに応じて処理を変える
    if (u8pkttyp == RMTCNF_PKTTYPE_REQUEST) {
      V_PRINTF(LB"!INF REQUEST CONF FR %08X", pRx->u32SrcAddr);
      bTranmitRespond(pRx->u32SrcAddr);

      // LED1 点灯
      sAppData.u32LedTick = u32TickCount_ms;
      sAppData.u8LedState = 0x01;
    } else {
      if (u8pkttyp == RMTCNF_PKTTYPE_ACK) {
	V_PRINTF(LB"!INF ACK CONF FR %08X", pRx->u32SrcAddr);

	uint8 u8stat = G_OCTET();
	if (u8stat) {
	  // LED2 点灯
	  sAppData.u8LedState = 0x03;
	  V_PRINTF(LB"SUCCESS %08X", pRx->u32SrcAddr);
	}
      } else {
	V_PRINTF(LB"!PKTTYPE_REQUEST", pRx->u32SrcAddr);
      }
    }
  }
}

/**
 * 送信完了イベント
 * @param u8CbId
 * @param bStatus
 */
static void cbAppToCoNet_vTxEvent(uint8 u8CbId, uint8 bStatus) {
}

/**
 * アプリケーションハンドラー定義
 *
 */
static tsCbHandler sCbHandler = {
  NULL, // cbAppToCoNet_u8HwInt,
  cbAppToCoNet_vHwEvent,
  cbAppToCoNet_vMain,
  NULL, //cbAppToCoNet_vNwkEvent,
  cbAppToCoNet_vRxEvent,
  cbAppToCoNet_vTxEvent
};

/**
 * アプリケーション初期化
 */
void vInitAppConfigMaster() {
  psCbHandler = &sCbHandler;
  pvProcessEv1 = vProcessEvCore;
}


/* *********************************/
/* * 送信関連の手続き              */
/* *********************************/

/**
 * 送信手続き
 * @param pTx
 * @return
 */
static bool_t bTransmit(tsTxDataApp *pTx) {
  // 送信元
  pTx->u32SrcAddr = sToCoNet_AppContext.u16ShortAddress;
  pTx->u8Cmd = RMTCNF_PKTCMD; // 0..7 の値を取る。パケットの種別を分けたい時に使用する

  pTx->u16DelayMax = 20;
  pTx->bAckReq = TRUE; // Ack
  pTx->u8Retry = 0x01; // アプリケーション再送１回

  if (ToCoNet_bMacTxReq(pTx)) {
    //ToCoNet_Tx_vProcessQueue(); // 送信処理をタイマーを待たずに実行する
    return TRUE;
  } else {
    return FALSE;
  }
}

#define TXINIT(c) \
  tsTxDataApp c;			    \
  memset(&sTx, 0, sizeof(c));		    \
  uint8 *q =  c.auData;

/**
 * 設定パケット要求
 * @return
 */
static bool_t bTranmitRespond(uint32 u32AddrDst) {
  TXINIT(sTx);

  sTx.u32DstAddr = u32AddrDst; // 送信先

  // ペイロードの準備
  //  *   OCTET    : パケットバージョン (1)
  S_OCTET(RMTCNF_PRTCL_VERSION);
  //  *   OCTET    : パケット種別 (0: 要求, 1: 応答, 2:ACK)
  S_OCTET(RMTCNF_PKTTYPE_DATA);
  //  *   OCTET(4) : アプリケーションバージョン
  S_BE_DWORD(VERSION_U32);

  // *       [パケット種別 = 応答]
  // *       OCTET    : 設定有効化 LQI
  S_OCTET(RMTCNF_MINLQI);
  // *       OCTET    : データ形式 (0: ベタ転送, 1pkt)
  S_OCTET(RMTCNF_DATATYPE_RAW_SINGLE_PKT);
  // *       OCTET    : データサイズ
  uint8 *q1 = q; // 後から保存する
  S_OCTET(0);
  // *       OCTET(N) : 設定データ
  uint16 u16len = Config_u16Serialize(q, 80, RMTCNF_DATATYPE_RAW_SINGLE_PKT);
  q += u16len;
  if (!u16len) {
    return FALSE;
  }
  *q1 = u16len;

  sTx.u8Len = q - sTx.auData;
  sTx.u8CbId = (sAppData.u16frame_count++) & 0xFF;
  sTx.u8Seq = sTx.u8CbId;

  // 送信
  return bTransmit(&sTx);
}

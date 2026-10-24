# 效能與穩定性調整

## 傳輸與排程

| 用途 | DMA |
|---|---|
| USART1 影像輸出 | DMA1 Channel1，與推論重疊 |
| HM01B0 相機擷取 | DMA1 Channel2，較高優先權 |
| UART4 Wi-Fi TX | DMA1 Channel3，取代逐 byte TX 中斷 |

Wi-Fi 的短 AT 指令／控制訊息會複製到 256-byte 暫存區；較大的影像區塊直接讀取既有封包佇列。封包在收到 SEND OK 前保留，斷線或逾時先停止 DMA，再重試封包。相較原先 2 KB TX 暫存區，少用 1,792 bytes；加上新統計欄位後，驗證 ELF 的靜態 RAM 淨減少 1,688 bytes。

`flow_service_background()` 在主迴圈、影像特徵讀取、SDK cancellation hook 和 `ei_sleep()` 提供協作服務，最快每 5 ms 一次。UART 中斷持續收資料；雷達可先解析，可能列印的 Wi-Fi/UWB 工作則等待影像 DMA 完成，避免破壞 IMG_S/IMG_E 封包。

**這不是搶占式排程。** 模型的 `model_invoke()` 內部沒有服務點，運算期間仍可能有較長服務間隔；請看 `gap_ms`、`infer_block_ms` 與 `rx_drop` 決定下一階段是否需要模型層級的排程調整。一般 printf 文字仍使用輪詢。

## 每秒統計

`src/ei_main.cpp` 的 `FLOW_STATS_ENABLE` 預設為 1。設為 0 可關閉統計列印。訊息放在 INF_E 之後，不改動既有影像／推論解析格式；首次完成的影格用來建立統計起點。

- `[PERF] frames / interval_ms`：此區間成功完成影格數與毫秒數；韌體完成率 = `frames * 1000 / interval_ms`。
- `crop_us`：平均裁切耗時。
- `infer_us`：平均 run_classifier 耗時，包含期間的協作服務；正常模式只輸出最高信心的單筆 `PRED` 摘要，設 `FLOW_VERBOSE_PREDICTIONS=1` 才輸出全部結果。
- `wait_us`：推論返回後，等待影像封包完成的平均耗時；不是影像完整傳送時間。
- `text_us`：平均結果處理、警報入列與結果文字輸出耗時。
- `[HEALTH] gap_ms`：觀察到的最長 Wi-Fi/UWB 服務間隔，累計值。
- `infer_block_ms`／`dma_wait_ms`／`text_block_ms`：觀察到的推論、影像 DMA 等待、結果文字處理最大阻塞時間。
- `rx_peak`／`rx_drop`：雷達／Wi-Fi／UWB 的接收緩衝區最高用量／丟棄 byte 數，累計值。
- `infer_err`／`event_drop`：推論失敗／警報重試到期次數。

微秒計時使用 1 ms SysTick 加上當前硬體 down-counter，處理 pending tick、32-bit 毫秒回捲，避免原先約 71.6 分鐘的乘法溢位。前提是 SysTick 仍以 1 ms 運行，且中斷不會被封鎖超過一個 tick。

## 輸出與失敗處理

- 一筆預測合併成一次格式化；生命徵象保留兩位小數。取消每幀 Starting/Success 除錯文字。
- `ei_printf` 暫存區由 1,024 改為 256 bytes，不再每次整區清零；超長訊息截斷至 255 bytes。
- 浮點輸出採有界限的標準格式化，明確處理 NaN/Inf，移除手寫 log10/pow 逐位轉換。
- 警報入列失敗時，保留當時的 crop 與生命徵象，服務通訊並重試，最多 500 ms。期間暫停新推論，避免影像被覆寫；到期仍無空位則記錄 event_drop 並恢復推論。本地蜂鳴器不依賴網路入列成功。
- 推論失敗後，清除結果結構、停止殘留的相機 DMA、重設 VSYNC 狀態，服務通訊並以 250/500 ms 間隔重試；成功後清除連續失敗計數。連續三次失敗即暫停推論，但通訊仍運作。這不會重建 SDK tensor arena；若仍持續失敗，需重新啟動以回收 SDK 內部可能殘留的配置。

## 驗證

```powershell
python tests/run_image_uart_dma_tests.py
python tests/run_optimization_tests.py
python -m unittest discover -s tests -p test_vital_signs_viewer.py
python tests/build_firmware_check.py
```

前兩項使用 32-bit MinGW 與模擬周邊，測試實際擷取的傳輸／解析／警報程式碼。ARM 檢查會重新編譯修改的六個模組，與既有其他 object 連結到 `mdk_v5/dma_verification/ITRI_Talent_A.elf`，不取代 IDE 的正常輸出。

上板請在 IDE 完整 Rebuild 並燒錄，分別觀察正常推論、Wi-Fi 傳送警報、斷線／重連下的 FPS、gap_ms 與 rx_drop。軟體驗證不能取代 DMA 路由與真實執行時間的硬體驗證。


# StepMania 3.9 永和金城武版(YHANIKI)
   + fork from [stepmania](https://github.com/stepmania/stepmania)
     + [tag v390release](https://github.com/stepmania/stepmania/releases/tag/v390release)
   + 部分功能沿用ddrtime編輯器版本StepMania
     + [STEPMANIA 3.9 編輯器我流版](https://forum.gamer.com.tw/C.php?bsn=16862&snA=1184)
<!-- TOC -->

- [StepMania 3.9 永和金城武版(YHANIKI)](#stepmania-39-%e6%b0%b8%e5%92%8c%e9%87%91%e5%9f%8e%e6%ad%a6%e7%89%88yhaniki)
  - [一般模式](#%e4%b8%80%e8%88%ac%e6%a8%a1%e5%bc%8f)
  - [作譜模式](#%e4%bd%9c%e8%ad%9c%e6%a8%a1%e5%bc%8f)
  - [連線模式](#%e9%80%a3%e7%b7%9a%e6%a8%a1%e5%bc%8f)
  - [其他](#%e5%85%b6%e4%bb%96)
  - [TODO](#todo)

<!-- /TOC -->
## 一般模式
   + 快速鍵
     + F4 更新指定歌曲
     + F5 快速重讀歌曲
     + Ctrl + F5 指定資料夾快速重讀歌曲
   + note流速不因歌曲倍速改變
   + 結算成績畫面顯示歌曲倍速
   + 修正使用AUTOPLAY或譜面無note會崩潰的問題
   + hold結尾改為TNS_MARVELOUS(白光)
   + 影片速度根據歌曲倍速改變
   + combo數字圖層上移

## 作譜模式
   + ddrtime
     + BPM與STOP顯示最小可到小數點第三位  
     + 右上方BEAT與時間顯示可到小數點第六位(特殊用)  
     + 箭頭間距拉伸變更成以0.5倍為單位，範圍可到0.25x~8x  
        若想整數拉伸可用ctrl+pageup與ctrl+pagedown鍵控制  
     + STEP停拍可以變負數  
     + 增加N鍵直接轉換負數功能，直接按N鍵會建立一個BPM標籤  
        CTRL+N會直接轉換BPM為負數，ALT+N會直接轉換STOP為負數  
     + 支援192nd箭頭編輯  
     + 增加直接編輯BPM與STOP功能   
     + 可追朔以前的備份  
        (編輯的歌曲資料夾會多個FileBackup資料夾，並以時間戳記為檔案名稱)  
     +  自動儲存 (可選擇你要儲存的時間間隔，也可以關閉，預設自動儲存時間是5分鐘)  
     +  Reverse逆流編輯環境下可直覺操作  
     +  Reverse逆流編輯直覺操作性可在Preferences下的Reverse Control Intuitive更改  
     + 增加DisplayBPM編輯，可在Edit Song Info下找到
   + 另外增加
     + 影片預覽撥放位置修正
     + 變速結尾bpm錯誤修正
     + 變速長條截斷修正
     + 開啟PlayerOptions, SongOptions 譜面還原問題修改(SONGMAN->Cleanup(); ignore)
     + 多段變速bpm修正
     + 多難度停拍位置修正
     + 測試前緩衝調整選項(Preferences->play mode beats buffer)
     + 顯示歌曲背景功能加上黑色遮罩(alpha0.5)

## 連線模式
   + 快速鍵
     + F1 重讀connect資料夾
     + F4 跳到當前選擇歌曲
     + F5 快速重讀歌曲
     + 同時按左右切換一般模式選歌
   + 指令
     + /list 顯示玩家編號與名稱
     + /host 玩家名稱或玩家編號 更換房主
     + /share 玩家編號 傳送歌曲(過濾影片)
     + /sharefull 玩家編號 傳送歌曲(包含影片)
   + 缺歌檔案傳送功能(/share /sharefull)
   + 修正CyberiaStyle 6連線模式無法顯示玩家名稱問題
   + 修正歌包跳回ALL GROUPS問題
   + 修正歌曲難度排序問題
   + 結算成績畫面傳送玩家百分比
   + 結算成績畫面傳送玩家血量graph
   + 遊戲強制變更為不關門模式
   + StandAloneServer(獨立執行伺服器)

## 其他
   + 修正options玩家變更名稱錯誤問題
   + options選預設charater(options->background options->danceing characters->static)
   + 增加使用右邊數字鍵輸入數字功能
   + options語言ini紀錄儲存
   + 修正ScreenDemonstration無歌曲崩潰問題

## TODO
  + 增加hold結尾判定模式
  + 打擊軌道背板
  + 連線模式 聊天輸入中文
  + 連線模式 大廳顯示歌曲背景
  + 連線模式 顯示玩家狀態
  + 連線模式 玩家聊天時顯示編號
  + 連線模式 選擇曲目相同性判斷(hash)
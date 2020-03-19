
## VS-2019 Migration Notes
    + In order to support modern libraries, we've moved to VS2019 with MSVC140.
        VS2005/2008 will not work now.
    + To ease maintenance, this version of YHANIKI targets only modern windows OSs.
    + A bunch of compiler warnings have been ignored to reduce the workload of this migration.
        They should be dealt with after the ongoing refactoring is completed.
    + LibLua and SDL have been rebuilt with MSVC140 and are included in this repository (in folder ThirdParty_rebuild).
    + Using C++14 now. Currently there are some problems with C++17.
    + Old assets pack won't work with this source. Please contact devs for updated assets pack.


# StepMania 3.9 永和金城武版(YHANIKI)
  + fork from [stepmania](https://github.com/stepmania/stepmania)
    - [tag v390release](https://github.com/stepmania/stepmania/releases/tag/v390release)
  + 部分功能沿用ddrtime編輯器版本StepMania
    - [STEPMANIA 3.9 編輯器我流版](https://forum.gamer.com.tw/C.php?bsn=16862&snA=1184)
<!-- TOC -->

    - [VS-2019 Migration Notes](#vs-2019-migration-notes)
- [StepMania 3.9 永和金城武版(YHANIKI)](#stepmania-39-%e6%b0%b8%e5%92%8c%e9%87%91%e5%9f%8e%e6%ad%a6%e7%89%88yhaniki)
  - [一般模式](#%e4%b8%80%e8%88%ac%e6%a8%a1%e5%bc%8f)
  - [作譜模式](#%e4%bd%9c%e8%ad%9c%e6%a8%a1%e5%bc%8f)
  - [連線模式](#%e9%80%a3%e7%b7%9a%e6%a8%a1%e5%bc%8f)
  - [其他](#%e5%85%b6%e4%bb%96)
  - [TODO](#todo)
  - [偉大貢獻者](#%e5%81%89%e5%a4%a7%e8%b2%a2%e7%8d%bb%e8%80%85)

<!-- /TOC -->
## 一般模式
 | 快速鍵     | 功能                 |
 | ----      | ----                 |
 | F4        | 更新指定歌曲          |
 | F5        | 快速重讀歌曲          |
 | Ctrl + F5 | 指定資料夾快速重讀歌曲 |
 
  + note流速不因歌曲倍速改變
  + 結算成績畫面顯示歌曲倍速
  + 修正使用AUTOPLAY或譜面無note會崩潰的問題
  + hold結尾改為TNS_MARVELOUS(白光)
  + 影片速度根據歌曲倍速改變
  + combo數字圖層上移
  + 遊戲大廳Note數改為顯示總按鍵數(原本為Tap + Hold)
  + 使用Shift+f8顯示AutoPlayCPU、結算分數時為0分
  + 修改NoteSkin:note/note(RV)長條發光模式

## 作譜模式
  + ddrtime
    - BPM與STOP顯示最小可到小數點第三位 
    - 右上方BEAT與時間顯示可到小數點第六位(特殊用) 
    - 箭頭間距拉伸變更成以0.5倍為單位，範圍可到0.25x~8x 
      + 若想整數拉伸可用ctrl+pageup與ctrl+pagedown鍵控制 
    - STOP停拍可以變負數 
    - N鍵轉換負數功能
      + N鍵:建立一個BPM標籤 
      + CTRL+N:轉換BPM為負數
      + ALT+N:轉換STOP為負數 
    - 支援192nd箭頭編輯 
    - 增加直接編輯BPM與STOP功能 
    - 可追朔以前的備份  
      + 編輯的歌曲資料夾會多個FileBackup資料夾，並以時間戳記為檔案名稱 
    - 自動儲存
      + 可以選擇要儲存的時間間隔或關閉，預設自動儲存時間是5分鐘  
    - Reverse逆流編輯環境下可直覺操作  
    - Reverse逆流編輯直覺操作性可在Preferences下的Reverse Control Intuitive更改  
    - 增加DisplayBPM編輯，可在Edit Song Info下找到
  + 另外增加
    - 修正影片預覽撥放位置
    - 修正變速結尾bpm錯誤
    - 修正變速長條截斷問題
    - 修正開啟PlayerOptions, SongOptions 譜面還原問題(SONGMAN->Cleanup(); ignore)
    - 修正多段變速bpm計算錯誤問題
    - 修正多難度停拍位置
    - 測試前緩衝調整選項(Preferences->play mode beats buffer)
    - Preferences加入背景影片亮度調整功能(Screen Filter)
    - Preferences加入預設Reverse功能(Default Scroll Reverse)
    - 測試譜面時顯示歌詞、判定與連擊數
    - 修正部分情況使用長條結尾當作假鍵(3號鍵)時閃退問題
    - 修正add/edit bg change檔案名稱過長顯示跑版問題
    - 使用玩家預設NoteSkin(針對note(RV)使用玩家)

## 連線模式

  | 快速鍵     | 功能             |
  | -----     | -----            |
  | F1        | 重讀connect資料夾 |
  | F4        | 跳到當前選擇歌曲   |
  | F5        | 快速重讀歌曲      |
  | F8        | 開關玩家狀態列    |
  | 同時按左右 | 切換一般模式選歌  |
  
  | 指令                      | 功能                                                  |
  | -----                    | -----                                                 |
  | /list                    | 顯示玩家編號與名稱
  | /host {玩家名稱或玩家編號} | 更換房主                                               |
  | /share [玩家編號]         | 傳送歌曲(過濾影片) 若不加玩家編號則傳送檔案給所有缺檔案玩家 |
  | /sharefull [玩家編號]     | 傳送歌曲(包含影片) 若不加玩家編號則傳送檔案給所有缺檔案玩家 |
  + 缺歌檔案傳送功能(/share /sharefull)
  + 修正CyberiaStyle 6連線模式無法顯示玩家名稱問題
  + 修正歌包跳回ALL GROUPS問題
  + 修正歌曲難度排序問題
  + 結算成績畫面傳送玩家百分比
  + 結算成績畫面傳送玩家血量graph
  + 遊戲強制變更為不關門模式
  + StandAloneServer(獨立執行伺服器)
  + 選擇曲目相同性判斷(判斷譜面按鍵數)
  + 大廳顯示玩家狀態
  + 不是置中的玩家，遊戲中成績列表會顯示在上方
  + 大廳播放完整歌曲

## 其他
   + 修正options玩家變更名稱錯誤問題
   + options選預設charater(options->background options->danceing characters->static)
   + 增加使用右邊數字鍵輸入數字功能
   + options語言ini紀錄儲存
   + 修正ScreenDemonstration無歌曲崩潰問題

## TODO
  + 增加hold結尾判定模式
  + 打擊軌道背板
  + 遊戲輸入中文
  + 連線模式 大廳顯示歌曲背景
  + 連線模式 玩家聊天時顯示編號
  
## 偉大貢獻者
  + MKLUO
  + 151
  + sky
  + Ameto
  + HM
  + Mochi
  + Zelda-YEH
  + Xiuer
  
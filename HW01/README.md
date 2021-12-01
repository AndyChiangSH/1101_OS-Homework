# HW01 - myshell
> 用C寫個自己的shell

## 功能
### 基本功能(70%)

- [x] 1. 印出prompt，例如“目前目錄$ ”(可以自行定義)，接受command
- [x] 2. 接受command並執行command
    - [x] Internal command：包含`export`、`echo`、`pwd`、`cd`。內建command時不得使用fork-exec函式
    - [x] External command：fork child process後，再用exec-like函數執行外部command，執行結果直接output到console上即可
- [x] 3. command執行結束後，再回到1.

### 進階功能(30%)

- [x] 查詢歷史command(10%)
按上/下鍵時，印出prompt及上/下一筆歷史command.
歷史command印出後，按enter鍵可執行該歷史command.
- [x] 支援background execution (&)(10%)
- [x] 支援Output redirection (>)(10%)

## 部署
1. 安裝readline
```
sudo apt-get install libreadline-dev
```

2. gcc編譯
```
gcc myshell.c -o myshell -lreadline
```

3. 開始使用!
```
./myshell
```


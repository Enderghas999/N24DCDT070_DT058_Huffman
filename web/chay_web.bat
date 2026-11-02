@echo off
chcp 65001 > nul
cd /d "%~dp0"
echo =================================================
echo   DT058 - Web Ma Huffman . INT13185
echo   Sinh vien: Dang The Phu . N24DCDT070
echo =================================================
where g++ >nul 2>&1
if errorlevel 1 (
    echo [LOI] Khong tim thay g++. Cai MinGW-w64 hoac MSYS2 roi chay lai.
    pause
    exit /b 1
)
echo [1/3] Bien dich va chay bo kiem tra ...
g++ -std=c++17 -O2 tests\kiem_tra.cpp -o tests\kiem_tra.exe
if errorlevel 1 goto loi
tests\kiem_tra.exe > tests\ket_qua_kiem_tra.txt
type tests\ket_qua_kiem_tra.txt | findstr /C:"TONG KET"
echo [2/3] Bien dich server (co the mat 20-60 giay vi httplib.h lon) ...
g++ -std=c++17 -O2 server.cpp -o huffman_app.exe -lws2_32
if errorlevel 1 goto loi
echo [3/3] Mo http://localhost:8080 ...
start /b cmd /c "timeout /t 2 /nobreak >nul && start http://localhost:8080"
huffman_app.exe
pause
exit /b 0
:loi
echo [LOI] Bien dich that bai, xem thong bao o tren.
pause
exit /b 1

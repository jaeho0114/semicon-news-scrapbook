@echo off
chcp 65001 > nul
cd /d "%~dp0"
echo 반도체 뉴스 스크랩북 서버를 켭니다...
echo 끄려면 이 창을 닫으세요.
start "" http://localhost:8080
node server.js
pause

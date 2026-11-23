@echo off
chcp 65001 > nul
cd /d "%~dp0"
echo 추천 검색어 뉴스를 새로 받아 GitHub 사이트에 올립니다...
node scripts\fetch-news.js || goto :fail
git add news-data.json
git commit -m "추천 검색어 뉴스 갱신"
git push
echo 완료! 1~2분 뒤 사이트에 반영됩니다.
pause
exit /b
:fail
echo 뉴스를 받지 못했습니다. 인터넷 연결을 확인해 주세요.
pause

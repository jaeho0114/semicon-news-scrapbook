// =============================================================
//  server.js — 웹 스크랩북용 작은 로컬 서버 (설치할 패키지 없음)
//  - http://localhost:8080 으로 index.html 을 보여 준다
//  - /rss?q=검색어 로 Google 뉴스 RSS 를 대신 받아 준다
//    (브라우저는 보안 정책(CORS) 때문에 news.google.com 을 직접 읽을 수 없기 때문.
//     C++ 버전에서 WinHTTP 가 하던 일을 여기서 Node.js 가 한다.)
//  실행:  node server.js
// =============================================================
const http = require("http");
const fs = require("fs");
const path = require("path");

const PORT = 8080;

http.createServer(async (req, res) => {
  const url = new URL(req.url, `http://localhost:${PORT}`);

  if (url.pathname === "/rss") {
    const q = url.searchParams.get("q") || "";
    const rssUrl = "https://news.google.com/rss/search?q=" + encodeURIComponent(q) + "&hl=ko&gl=KR&ceid=KR:ko";
    try {
      const r = await fetch(rssUrl);
      res.writeHead(r.status, { "Content-Type": "application/xml; charset=utf-8" });
      res.end(await r.text());
    } catch (e) {
      res.writeHead(502, { "Content-Type": "text/plain; charset=utf-8" });
      res.end("Google 뉴스에 접속하지 못했습니다.");
    }
    return;
  }

  if (url.pathname === "/" || url.pathname === "/index.html") {
    res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
    fs.createReadStream(path.join(__dirname, "index.html")).pipe(res);
    return;
  }

  res.writeHead(404);
  res.end("Not found");
}).listen(PORT, () => console.log(`반도체 뉴스 스크랩북: http://localhost:${PORT}`));

// =============================================================
//  fetch-news.js — 추천 검색어의 Google 뉴스를 받아 news-data.json 으로 저장
//  GitHub Pages 는 서버가 없어서, 미리 뉴스를 받아 사이트에 같이 올려 두는 방식.
//  실행:  뉴스갱신.bat 더블클릭  (또는 node scripts/fetch-news.js 후 git push)
// =============================================================
const fs = require("fs");
const path = require("path");

const KEYWORDS = ["반도체", "HBM", "삼성전자 반도체", "SK하이닉스", "TSMC", "엔비디아", "파운드리", "마이크론"];
const PER_KEYWORD = 20;

function getTag(block, tag) {
  const m = block.match(new RegExp(`<${tag}[^>]*>([\\s\\S]*?)</${tag}>`));
  return m ? decode(m[1].trim()) : "";
}
function decode(s) {
  return s.replace(/<!\[CDATA\[([\s\S]*?)\]\]>/g, "$1")
    .replace(/&lt;/g, "<").replace(/&gt;/g, ">").replace(/&quot;/g, '"')
    .replace(/&#39;|&apos;/g, "'").replace(/&amp;/g, "&");
}

async function fetchKeyword(keyword) {
  const url = "https://news.google.com/rss/search?q=" + encodeURIComponent(keyword) + "&hl=ko&gl=KR&ceid=KR:ko";
  const res = await fetch(url);
  if (!res.ok) throw new Error(`${keyword}: HTTP ${res.status}`);
  const xml = await res.text();
  const items = [...xml.matchAll(/<item>([\s\S]*?)<\/item>/g)].map(m => ({
    title: getTag(m[1], "title"),
    source: getTag(m[1], "source"),
    pubDate: getTag(m[1], "pubDate"),
    link: getTag(m[1], "link"),
  })).filter(i => i.title && i.pubDate);
  items.sort((a, b) => Date.parse(b.pubDate) - Date.parse(a.pubDate));
  return items.slice(0, PER_KEYWORD);
}

(async () => {
  const data = { updated: new Date().toISOString(), keywords: {} };
  for (const keyword of KEYWORDS) {
    try {
      data.keywords[keyword] = await fetchKeyword(keyword);
      console.log(`${keyword}: ${data.keywords[keyword].length}건`);
    } catch (e) {
      console.error(e.message);
    }
    await new Promise(r => setTimeout(r, 1500));   // Google 이 너무 빠른 요청을 막지 않도록 잠깐 쉼
  }
  // 이번에 못 받은 검색어는 지난번 파일의 내용을 그대로 쓴다
  const out = path.join(__dirname, "..", "news-data.json");
  if (fs.existsSync(out)) {
    const old = JSON.parse(fs.readFileSync(out, "utf8"));
    for (const k of KEYWORDS) if (!data.keywords[k] && old.keywords?.[k]) data.keywords[k] = old.keywords[k];
  }
  if (Object.keys(data.keywords).length === 0) process.exit(1);
  fs.writeFileSync(out, JSON.stringify(data));
  console.log("저장:", out);
})();

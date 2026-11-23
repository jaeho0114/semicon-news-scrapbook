// =============================================================
//  NewsGoogle.cpp — 8번 메뉴: Google 뉴스에서 기사 가져오기
//
//  동작 순서
//   1) 검색어를 URL 에 넣을 수 있는 형태로 바꾼다 (UTF-8 + %인코딩)
//   2) WinHTTP(윈도우 기본 기능)로 Google 뉴스 RSS 주소에 접속해 XML 글을 받는다
//   3) XML 에서 <item> 마다 <title> · <source> · <pubDate> 를 잘라 낸다
//   4) 최신순으로 10개를 보여 주고, 사용자가 고른 기사를 스크랩북에 넣는다
//
//  Google 뉴스 RSS 는 API 키가 필요 없다 → GitHub 에 올려도 숨길 비밀번호가 없다.
// =============================================================
#include "News.h"

#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cctype>

#pragma comment(lib, "winhttp.lib")     // WinHTTP 라이브러리를 링크한다 (프로젝트 설정을 따로 안 바꿔도 됨)

using namespace std;

const int MAX_RESULTS = 10;             // 한 번에 보여 줄 기사 수

// RSS 에서 꺼낸 기사 한 건 (스크랩하기 전 임시 보관용)
struct GoogleItem {
    string title;
    string source;      // 언론사
    string date;        // YYYY-MM-DD (한국 시간)
    time_t time;        // 정렬용 시각
};

// ---------- 1. 글자 인코딩 변환 ----------
// 인터넷(Google)은 UTF-8, 이 프로그램의 콘솔 · 파일은 윈도우 기본 한글 코드(CP949)를 쓴다.
// 그래서 보낼 때는 CP949 → UTF-8, 받을 때는 UTF-8 → CP949 로 바꿔야 한글이 깨지지 않는다.
static string convertCodePage(const string& text, UINT fromCode, UINT toCode) {
    if (text.empty()) return "";
    int wideLength = MultiByteToWideChar(fromCode, 0, text.c_str(), (int)text.size(), nullptr, 0);
    wstring wide(wideLength, L'\0');
    MultiByteToWideChar(fromCode, 0, text.c_str(), (int)text.size(), &wide[0], wideLength);

    int length = WideCharToMultiByte(toCode, 0, wide.c_str(), wideLength, nullptr, 0, nullptr, nullptr);
    string result(length, '\0');
    WideCharToMultiByte(toCode, 0, wide.c_str(), wideLength, &result[0], length, nullptr, nullptr);
    return result;                      // CP949 에 없는 글자(이모지 등)는 '?' 로 바뀐다
}

static string utf8ToLocal(const string& s) { return convertCodePage(s, CP_UTF8, CP_ACP); }
static string localToUtf8(const string& s) { return convertCodePage(s, CP_ACP, CP_UTF8); }

// 검색어를 URL 에 넣을 수 있게 바꾼다. 예: "반도체 HBM" → "%EB%B0%98%EB%8F%84%EC%B2%B4%20HBM"
static string urlEncode(const string& utf8Text) {
    const char* hex = "0123456789ABCDEF";
    string result;
    for (unsigned char c : utf8Text) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += (char)c;
        } else {
            result += '%';
            result += hex[c / 16];
            result += hex[c % 16];
        }
    }
    return result;
}

// ---------- 2. 인터넷에서 글 받아오기 (WinHTTP) ----------
// host 의 path 주소를 https 로 요청해서 받은 글을 body 에 담는다. 성공하면 true.
static bool httpGet(const wstring& host, const wstring& path, string& body) {
    bool success = false;
    HINTERNET session = WinHttpOpen(L"SemiconNewsScrapbook/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET connect = session ? WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0) : nullptr;
    HINTERNET request = connect ? WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                     WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : nullptr;

    if (request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
                && WinHttpReceiveResponse(request, nullptr)) {
        DWORD statusCode = 0, size = sizeof(statusCode);
        WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);
        if (statusCode == 200) {
            DWORD available = 0;
            while (WinHttpQueryDataAvailable(request, &available) && available > 0) {   // 받은 글을 조금씩 이어 붙인다
                string chunk(available, '\0');
                DWORD read = 0;
                if (!WinHttpReadData(request, &chunk[0], available, &read)) break;
                body.append(chunk, 0, read);
            }
            success = !body.empty();
        }
    }
    if (request) WinHttpCloseHandle(request);   // 연 순서의 반대로 닫는다
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
    return success;
}

// ---------- 3. XML 에서 값 꺼내기 ----------
// <tag ...>값</tag> 에서 "값" 만 잘라 낸다. 없으면 빈 문자열.
static string getTagValue(const string& xml, const string& tag) {
    size_t start = xml.find("<" + tag);
    if (start == string::npos) return "";
    start = xml.find('>', start);
    size_t end = xml.find("</" + tag + ">", start);
    if (start == string::npos || end == string::npos) return "";
    return xml.substr(start + 1, end - start - 1);
}

// XML 의 특수 문자 표기(&amp; 등)를 원래 글자로 되돌린다.
static string decodeEntities(string s) {
    const string codes[][2] = { {"&lt;", "<"}, {"&gt;", ">"}, {"&quot;", "\""}, {"&#39;", "'"}, {"&apos;", "'"}, {"&amp;", "&"} };
    for (const auto& code : codes) {
        size_t pos = 0;
        while ((pos = s.find(code[0], pos)) != string::npos) {
            s.replace(pos, code[0].size(), code[1]);
            pos += code[1].size();
        }
    }
    return s;
}

// "Wed, 04 Nov 2026 03:44:15 GMT" (영국 시간) → 한국 시간(+9시간)의 "2026-11-04"
static bool parsePubDate(const string& pubDate, string& date, time_t& when) {
    const string months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    stringstream ss(pubDate);
    string weekday, monthText;
    int day, year, hour, minute, second;
    char colon1, colon2;
    if (!(ss >> weekday >> day >> monthText >> year >> hour >> colon1 >> minute >> colon2 >> second)) return false;

    tm t = {};
    t.tm_mon = -1;
    for (int i = 0; i < 12; i++) {
        if (monthText == months[i]) t.tm_mon = i;
    }
    if (t.tm_mon < 0) return false;
    t.tm_year = year - 1900;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;

    when = _mkgmtime(&t) + 9 * 60 * 60;     // GMT → 한국 시간 (자정 근처 기사도 날짜가 맞게)
    tm korea = {};
    gmtime_s(&korea, &when);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &korea);
    date = buffer;
    return true;
}

// RSS 글 전체에서 <item> 을 하나씩 잘라 GoogleItem 목록으로 만든다.
static vector<GoogleItem> parseRss(const string& xml) {
    vector<GoogleItem> items;
    size_t pos = 0;
    while ((pos = xml.find("<item>", pos)) != string::npos) {
        size_t end = xml.find("</item>", pos);
        if (end == string::npos) break;
        string block = xml.substr(pos, end - pos);
        pos = end;

        GoogleItem item;
        item.title  = utf8ToLocal(decodeEntities(getTagValue(block, "title")));
        item.source = utf8ToLocal(decodeEntities(getTagValue(block, "source")));
        if (item.title.empty() || !parsePubDate(getTagValue(block, "pubDate"), item.date, item.time)) continue;

        // 제목 끝의 " - 언론사" 는 떼어 낸다 (언론사는 따로 보여 주므로)
        string suffix = " - " + item.source;
        if (!item.source.empty() && item.title.size() > suffix.size()
            && item.title.compare(item.title.size() - suffix.size(), suffix.size(), suffix) == 0) {
            item.title.erase(item.title.size() - suffix.size());
        }
        items.push_back(item);
    }
    return items;
}

// ---------- 4. 회사 자동 판별 ----------
// 제목에 회사 이름(또는 줄임말)이 있으면 그 회사로 추천한다. 여러 개면 먼저 나온 것.
string guessCompany(const string& title) {
    const string aliases[][2] = {
        {"삼성", "삼성전자"}, {"하이닉스", "SK하이닉스"}, {"SK", "SK하이닉스"}, {"TSMC", "TSMC"},
        {"인텔", "인텔"}, {"Intel", "인텔"}, {"엔비디아", "엔비디아"}, {"NVIDIA", "엔비디아"}, {"마이크론", "마이크론"},
    };
    size_t bestPosition = string::npos;
    string bestCompany = "기타";
    for (const auto& alias : aliases) {
        size_t found = title.find(alias[0]);
        if (found != string::npos && found < bestPosition) {
            bestPosition = found;
            bestCompany = alias[1];
        }
    }
    return bestCompany;
}

// ---------- 5. 8번 메뉴 ----------
void fetchGoogleNews(vector<News>& list) {
    cout << "\n[Google 뉴스에서 가져오기]\n";
    string keyword = readLine("  검색어 (예: 반도체 HBM): ");

    string path = "/rss/search?q=" + urlEncode(localToUtf8(keyword)) + "&hl=ko&gl=KR&ceid=KR:ko";
    cout << "  Google 뉴스에 접속하는 중...\n";

    string xml;
    if (!httpGet(L"news.google.com", wstring(path.begin(), path.end()), xml)) {   // path 는 영문·기호뿐이라 그대로 wstring 으로
        cout << "  → 뉴스를 가져오지 못했습니다. 인터넷 연결을 확인하고 다시 시도해 주세요.\n";
        return;
    }

    vector<GoogleItem> items = parseRss(xml);
    sort(items.begin(), items.end(), [](const GoogleItem& a, const GoogleItem& b) {
        return a.time > b.time;             // 최신 기사부터
    });
    if ((int)items.size() > MAX_RESULTS) items.resize(MAX_RESULTS);

    if (items.empty()) {
        cout << "  → '" << keyword << "' 검색 결과가 없습니다. 다른 검색어를 써 보세요.\n";
        return;
    }

    cout << "\n'" << keyword << "' 최신 뉴스 " << items.size() << "건\n";
    for (size_t i = 0; i < items.size(); i++) {
        cout << "[" << i + 1 << "] " << items[i].date << " | " << items[i].source << "\n";
        cout << "    " << items[i].title << "\n";
    }

    // 원하는 만큼 골라서 스크랩한다. 0 을 누르면 끝.
    while (true) {
        int number = readInt("\n  스크랩할 번호 (0 = 끝내기): ", 0, (int)items.size());
        if (number == 0) break;
        const GoogleItem& picked = items[number - 1];

        bool alreadySaved = false;
        for (const News& n : list) {
            if (n.title == removeBar(picked.title)) alreadySaved = true;
        }
        if (alreadySaved) {
            cout << "  → 이미 스크랩한 기사입니다.\n";
            continue;
        }

        News n;
        n.date  = picked.date;
        n.title = removeBar(picked.title);
        cout << "  제목: " << n.title << "\n";

        string guess = guessCompany(n.title);
        string answer = readLine("  관련 회사를 '" + guess + "'(으)로 할까요? (y/n): ");
        n.company    = (answer == "y" || answer == "Y") ? guess : chooseCompany();
        n.memo       = removeBar(readLine("  한 줄 메모: "));
        n.importance = readInt("  중요도 (1~5): ", 1, 5);

        list.push_back(n);
        cout << "  → 스크랩했습니다. (총 " << list.size() << "건)\n";
    }
}

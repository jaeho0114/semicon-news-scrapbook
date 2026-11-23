// =============================================================
//  News.h — 반도체 뉴스 스크랩북의 "설계도"
//  - 뉴스 한 건을 나타내는 구조체(News)와 상수
//  - 모든 함수의 선언(이름 · 매개변수 · 반환형)
//  다른 .cpp 파일들은 이 헤더를 #include 해서 서로의 함수를 쓴다.
// =============================================================
#pragma once                    // 같은 헤더가 여러 번 포함되어도 한 번만 읽게 한다

#include <string>
#include <vector>

// ---------- 데이터 구조 ----------
struct News {
    std::string date;      // 날짜 (YYYY-MM-DD)
    std::string title;     // 기사 제목
    std::string company;   // 관련 회사
    std::string memo;      // 한 줄 메모
    int importance;        // 중요도 1~5
};

// ---------- 상수 ----------
// 헤더에 둔 const 변수는 .cpp 파일마다 따로 만들어지므로 여러 파일에서 포함해도 충돌하지 않는다.
const std::string DATA_FILE = "news_scrap.txt";
const std::vector<std::string> COMPANIES = { "삼성전자", "SK하이닉스", "TSMC", "인텔", "엔비디아", "마이크론", "기타" };

// ---------- NewsInput.cpp : 입력 · 검사 ----------
std::string readLine(const std::string& prompt);                       // 한 줄 입력 (빈 줄이면 다시)
int         readInt(const std::string& prompt, int minValue, int maxValue);   // 범위 안의 정수만
bool        isValidDate(const std::string& date);                      // YYYY-MM-DD + 실제 있는 날짜인지
std::string chooseCompany();                                           // 회사 번호로 고르기
std::string removeBar(std::string s);                                  // 저장 구분자 '|' → '/'

// ---------- NewsFeature.cpp : 출력 도우미 ----------
std::string makeStars(int count);                                      // 3 → ★★★☆☆
void        printNews(const News& n, int number);
void        printList(const std::vector<News>& list);
std::string toLowerAscii(std::string s);                               // 영어 대문자만 소문자로
bool        containsKeyword(const std::string& text, const std::string& keyword);

// ---------- NewsFeature.cpp : 메뉴 기능 ----------
void addNews(std::vector<News>& list);              // 1. 추가
void showAll(const std::vector<News>& list);        // 2. 전체 목록 (최신순)
void searchByCompany(const std::vector<News>& list);// 3. 회사별 검색
void searchByKeyword(const std::vector<News>& list);// 4. 키워드 검색
void showByImportance(const std::vector<News>& list);// 5. 중요도순
void showStatistics(const std::vector<News>& list); // 6. 회사별 통계
void deleteNews(std::vector<News>& list);           // 7. 삭제

// ---------- NewsGoogle.cpp : Google 뉴스 가져오기 ----------
void        fetchGoogleNews(std::vector<News>& list);   // 8. 검색어로 최신 뉴스를 받아 골라서 스크랩
std::string guessCompany(const std::string& title);    // 제목에서 관련 회사 추측

// ---------- NewsFile.cpp : 파일 저장 · 불러오기 ----------
void loadFromFile(std::vector<News>& list);
void saveToFile(const std::vector<News>& list);

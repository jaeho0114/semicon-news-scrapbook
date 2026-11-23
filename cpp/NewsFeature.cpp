// =============================================================
//  NewsFeature.cpp — 메뉴 1~7 의 기능과 출력 도우미
//  추가 · 목록 · 회사별 검색 · 키워드 검색 · 중요도순 · 통계 · 삭제
// =============================================================
#include "News.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

// ---------- 출력 도우미 ----------

// 중요도를 별(★☆)로 바꾼다. 예: 3 → ★★★☆☆
string makeStars(int count) {
    string stars;
    for (int i = 1; i <= 5; i++) {
        stars += (i <= count) ? "★" : "☆";
    }
    return stars;
}

void printNews(const News& n, int number) {
    cout << "[" << number << "] " << n.date << " | " << n.company << " | " << makeStars(n.importance) << "\n";
    cout << "    제목: " << n.title << "\n";
    cout << "    메모: " << n.memo << "\n";
}

void printList(const vector<News>& list) {
    for (size_t i = 0; i < list.size(); i++) {
        printNews(list[i], (int)i + 1);
    }
}

// 영어 대문자만 소문자로 바꾼다 (한글은 그대로). "HBM" 과 "hbm" 을 같은 검색어로 보기 위해.
// 한글 한 글자는 2바이트(CP949)라서, 첫 바이트가 128 이상이면 두 번째 바이트까지 건너뛴다.
string toLowerAscii(string s) {
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 128) { i++; continue; }    // 한글 글자 → 두 바이트를 통째로 건너뜀
        if (c >= 'A' && c <= 'Z') s[i] = (char)(c - 'A' + 'a');
    }
    return s;
}

bool containsKeyword(const string& text, const string& keyword) {
    return toLowerAscii(text).find(toLowerAscii(keyword)) != string::npos;
}

// ---------- 메뉴 기능 ----------

// 1. 뉴스 추가
void addNews(vector<News>& list) {
    cout << "\n[뉴스 추가]\n";
    News n;

    while (true) {
        n.date = readLine("  날짜 (예: 2026-11-04): ");
        if (isValidDate(n.date)) break;
        cout << "  → 날짜는 YYYY-MM-DD 형식의 올바른 날짜로 입력해 주세요.\n";
    }
    n.title      = removeBar(readLine("  기사 제목: "));
    n.company    = chooseCompany();
    n.memo       = removeBar(readLine("  한 줄 메모: "));
    n.importance = readInt("  중요도 (1~5): ", 1, 5);

    list.push_back(n);
    cout << "  → 저장했습니다. (총 " << list.size() << "건)\n";
}

// 2. 전체 목록 보기 (최신 날짜순)
void showAll(const vector<News>& list) {
    cout << "\n[전체 목록] 총 " << list.size() << "건\n";
    if (list.empty()) {
        cout << "  아직 스크랩한 뉴스가 없습니다. 1번 메뉴로 추가해 보세요.\n";
        return;
    }
    // 원본은 그대로 두고, 날짜 최신순으로 정렬한 복사본을 보여 준다.
    vector<News> sorted = list;
    sort(sorted.begin(), sorted.end(), [](const News& a, const News& b) {
        return a.date > b.date;           // YYYY-MM-DD 는 문자열 비교로 날짜 순서가 맞다
    });
    printList(sorted);
}

// 3. 회사별 검색
void searchByCompany(const vector<News>& list) {
    cout << "\n[회사별 검색]\n";
    string company = chooseCompany();

    vector<News> result;
    for (const News& n : list) {
        if (n.company == company) result.push_back(n);
    }
    cout << "\n'" << company << "' 관련 뉴스 " << result.size() << "건\n";
    if (result.empty()) {
        cout << "  해당 회사의 뉴스가 없습니다.\n";
        return;
    }
    printList(result);
}

// 4. 키워드 검색 (제목 · 메모)
void searchByKeyword(const vector<News>& list) {
    cout << "\n[키워드 검색]\n";
    string keyword = readLine("  검색어 (예: HBM, 파운드리): ");

    vector<News> result;
    for (const News& n : list) {
        if (containsKeyword(n.title, keyword) || containsKeyword(n.memo, keyword)) {
            result.push_back(n);
        }
    }
    cout << "\n'" << keyword << "' 검색 결과 " << result.size() << "건\n";
    if (result.empty()) {
        cout << "  검색 결과가 없습니다. 다른 검색어를 써 보세요.\n";
        return;
    }
    printList(result);
}

// 5. 중요도순 보기
void showByImportance(const vector<News>& list) {
    cout << "\n[중요도순 보기]\n";
    if (list.empty()) {
        cout << "  아직 스크랩한 뉴스가 없습니다.\n";
        return;
    }
    int minImportance = readInt("  최소 중요도 (1~5, 1이면 전체): ", 1, 5);

    vector<News> result;
    for (const News& n : list) {
        if (n.importance >= minImportance) result.push_back(n);
    }
    // 중요도 높은 순, 같으면 최신 날짜 순
    sort(result.begin(), result.end(), [](const News& a, const News& b) {
        if (a.importance != b.importance) return a.importance > b.importance;
        return a.date > b.date;
    });

    cout << "\n중요도 " << minImportance << " 이상 뉴스 " << result.size() << "건\n";
    if (result.empty()) {
        cout << "  조건에 맞는 뉴스가 없습니다.\n";
        return;
    }
    printList(result);
}

// 6. 회사별 통계
void showStatistics(const vector<News>& list) {
    cout << "\n[회사별 통계] 총 " << list.size() << "건\n";
    if (list.empty()) {
        cout << "  아직 스크랩한 뉴스가 없습니다.\n";
        return;
    }

    vector<int> counts(COMPANIES.size(), 0);
    vector<int> importanceSum(COMPANIES.size(), 0);
    for (const News& n : list) {
        for (size_t i = 0; i < COMPANIES.size(); i++) {
            if (n.company == COMPANIES[i]) {
                counts[i]++;
                importanceSum[i] += n.importance;
            }
        }
    }

    int maxIndex = 0;
    for (size_t i = 0; i < COMPANIES.size(); i++) {
        if (counts[i] == 0) continue;
        double average = (double)importanceSum[i] / counts[i];
        cout << "  " << COMPANIES[i] << "\t: " << string(counts[i], '#') << " " << counts[i] << "건"
             << " (평균 중요도 " << fixed << setprecision(1) << average << ")\n";
        if (counts[i] > counts[maxIndex]) maxIndex = (int)i;
    }
    cout << "\n  ▶ 가장 많이 스크랩한 회사: " << COMPANIES[maxIndex] << " (" << counts[maxIndex] << "건)\n";
}

// 7. 뉴스 삭제
void deleteNews(vector<News>& list) {
    cout << "\n[뉴스 삭제]\n";
    if (list.empty()) {
        cout << "  삭제할 뉴스가 없습니다.\n";
        return;
    }
    printList(list);
    int number = readInt("  삭제할 번호 (0 = 취소): ", 0, (int)list.size());
    if (number == 0) {
        cout << "  → 삭제를 취소했습니다.\n";
        return;
    }
    cout << "  '" << list[number - 1].title << "'\n";
    string answer = readLine("  정말 삭제할까요? (y/n): ");
    if (answer == "y" || answer == "Y") {
        list.erase(list.begin() + (number - 1));
        cout << "  → 삭제했습니다. (남은 뉴스 " << list.size() << "건)\n";
    } else {
        cout << "  → 삭제를 취소했습니다.\n";
    }
}

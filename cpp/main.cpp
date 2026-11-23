// =============================================================
//  반도체 뉴스 스크랩북 (Semiconductor News Scrapbook)
//  - 읽은 반도체 뉴스를 날짜·제목·회사·메모·중요도와 함께 기록하고
//    검색 · 정렬 · 통계로 다시 볼 수 있는 콘솔 프로그램
//
//  파일 구성
//    News.h          구조체 · 상수 · 함수 선언 (설계도)
//    NewsInput.cpp   입력 · 검사
//    NewsFeature.cpp 메뉴 1~7 기능 + 출력
//    NewsGoogle.cpp  Google 뉴스 가져오기 (8번 메뉴)
//    NewsFile.cpp    파일 저장 · 불러오기
//    main.cpp        메뉴를 보여 주고 기능을 고르는 곳 (이 파일)
// =============================================================
#include "News.h"

#include <iostream>

using namespace std;

int main() {
    vector<News> scrapbook;
    loadFromFile(scrapbook);                // 지난번에 저장한 뉴스 불러오기

    cout << "==========================================\n";
    cout << "        반도체 뉴스 스크랩북 v1.0\n";
    cout << "==========================================\n";
    cout << "저장된 뉴스 " << scrapbook.size() << "건을 불러왔습니다.\n";

    while (true) {
        cout << "\n------------- 메뉴 -------------\n";
        cout << " 1. 뉴스 추가\n";
        cout << " 2. 전체 목록 보기\n";
        cout << " 3. 회사별 검색\n";
        cout << " 4. 키워드 검색 (제목·메모)\n";
        cout << " 5. 중요도순으로 보기\n";
        cout << " 6. 회사별 통계\n";
        cout << " 7. 뉴스 삭제\n";
        cout << " 8. Google 뉴스에서 가져오기\n";
        cout << " 0. 저장하고 종료\n";
        cout << "--------------------------------\n";

        int menu = readInt("메뉴 선택: ", 0, 8);

        switch (menu) {
        case 1: addNews(scrapbook);          break;
        case 2: showAll(scrapbook);          break;
        case 3: searchByCompany(scrapbook);  break;
        case 4: searchByKeyword(scrapbook);  break;
        case 5: showByImportance(scrapbook); break;
        case 6: showStatistics(scrapbook);   break;
        case 7: deleteNews(scrapbook);       break;
        case 8: fetchGoogleNews(scrapbook);  break;
        case 0:
            saveToFile(scrapbook);
            cout << "뉴스 " << scrapbook.size() << "건을 " << DATA_FILE << " 에 저장했습니다. 프로그램을 종료합니다.\n";
            return 0;
        }
    }
}

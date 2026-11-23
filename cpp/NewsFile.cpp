// =============================================================
//  NewsFile.cpp — 파일 저장 · 불러오기
//  저장 형식: 한 줄에 뉴스 하나 → 날짜|제목|회사|메모|중요도
// =============================================================
#include "News.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void saveToFile(const vector<News>& list) {
    ofstream fout(DATA_FILE);
    if (!fout) {
        cout << "  → 파일을 저장하지 못했습니다.\n";
        return;
    }
    for (const News& n : list) {
        fout << n.date << '|' << n.title << '|' << n.company << '|' << n.memo << '|' << n.importance << '\n';
    }
}

void loadFromFile(vector<News>& list) {
    ifstream fin(DATA_FILE);
    if (!fin) return;                       // 처음 실행이면 파일이 없다 → 빈 스크랩북으로 시작

    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        News n;
        string importanceText;
        if (getline(ss, n.date, '|') && getline(ss, n.title, '|') && getline(ss, n.company, '|')
            && getline(ss, n.memo, '|') && getline(ss, importanceText)) {
            try {
                n.importance = stoi(importanceText);
            } catch (...) {
                continue;                   // 망가진 줄은 건너뛴다
            }
            if (n.importance >= 1 && n.importance <= 5) list.push_back(n);
        }
    }
}

// =============================================================
//  NewsInput.cpp — 사용자 입력을 받고 검사하는 함수들
//  잘못된 값이 들어오면 프로그램이 멈추지 않고 다시 묻는다.
// =============================================================
#include "News.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

using namespace std;

// 안내 문구를 보여 주고 한 줄을 입력받는다. 빈 줄이면 다시 묻는다.
string readLine(const string& prompt) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {          // 입력이 끝나 버린 경우(EOF) 안전하게 종료
            cout << "\n입력이 끝나 프로그램을 종료합니다.\n";
            exit(0);
        }
        if (!line.empty()) return line;
        cout << "  → 빈 값은 입력할 수 없습니다. 다시 입력해 주세요.\n";
    }
}

// minValue ~ maxValue 범위의 정수만 받는다. 문자나 범위 밖 숫자는 다시 묻는다.
int readInt(const string& prompt, int minValue, int maxValue) {
    while (true) {
        string line = readLine(prompt);
        stringstream ss(line);
        int value;
        char extra;
        if (ss >> value && !(ss >> extra) && value >= minValue && value <= maxValue) {
            return value;
        }
        cout << "  → " << minValue << " ~ " << maxValue << " 사이의 숫자를 입력해 주세요.\n";
    }
}

// 날짜가 YYYY-MM-DD 형식이고 월·일이 올바른 범위인지 검사한다.
bool isValidDate(const string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }
    int year  = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day   = stoi(date.substr(8, 2));
    if (month < 1 || month > 12) return false;

    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leapYear) daysInMonth[1] = 29;
    return day >= 1 && day <= daysInMonth[month - 1];
}

// 회사 목록을 번호로 보여 주고 하나를 고르게 한다.
string chooseCompany() {
    cout << "  회사 목록: ";
    for (size_t i = 0; i < COMPANIES.size(); i++) {
        cout << i + 1 << "." << COMPANIES[i] << "  ";
    }
    cout << "\n";
    int choice = readInt("  회사 번호: ", 1, (int)COMPANIES.size());
    return COMPANIES[choice - 1];
}

// 파일에서 칸을 나누는 '|' 가 입력에 들어 있으면 '/' 로 바꾼다.
string removeBar(string s) {
    replace(s.begin(), s.end(), '|', '/');
    return s;
}

# LSP_TECH
## 비밀병기 모음집
---
0. replace
0. tree
0. pthread 5 초제한
0. diff
설계과제
new attempt
int filter(const struct dirent *entry) {
    if (strstr(entry->d_name, filename) != NULL) {
        return 1; // 부분 문자열이 포함된 경우 True 반환
    }
    return 0; // 부분 문자열이 포함되지 않은 경우 False 반환
}

// main.c
#include <stdio.h>

// PassOrFail 함수 정의: 50점 이상이면 1, 아니면 0 반환
int PassOrFail(int score) {
    if (score >= 50) {
        return 1;
    }
    else {
        return 0;s
    }
}

int main(void) {
    int myExpectedScore = 72;  // 본인의 예상 점수 입력 (예: 72점)
    int result = PassOrFail(myExpectedScore);

    if (result) {
        printf("재시험 없습니다!\n");
    }
    else {
        printf("우리는 망했다…  재시험이다…\n");
    }

    return 0;
}

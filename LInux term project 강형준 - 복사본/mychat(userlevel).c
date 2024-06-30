#include <stdio.h>      // c를 기본적으로 작성하기 위해서 표준 입출력 함수를 사용하기 위해 포함
#include <unistd.h>     // 유닉스 표준 시스템 콜 정의를 사용하기 위해 포함
#include <sys/syscall.h> // 우리가 정의한 시스템 콜 함수를 사용하기 위해 포함한다
#include <errno.h>      // 시스템 에러 번호를 정의하고 있는 헤더 파일 포함
#include <string.h>     // 사용자에게 ip, message, port 등 문자열 처리 함수를 사용하기 위해 포함
#include <stdlib.h>     // 표준 라이브러리 함수, 여기서는 atoi 함수 사용을 위해 포함

#define __NR_mychat 449 // mychat 시스템 콜 번호는 449번이었으므로 똑같이 정의해준다.

int main(int argc, char *argv[]) {
    // 사용자로부터 IP 주소, 메시지, 포트 번호를 입력받아야 하므로 인자 개수 확인
    if (argc != 4) {
        printf("Usage: %s <IP address> <message> <port>\n", argv[0]);
        return 1; // 올바르지 않은 인자 개수인 경우 에러 메시지 출력 후 종료시킨다.
    }

    char *ip = argv[1]; // 첫 번째 인자로 IP 주소 저장
    char *msg = argv[2]; // 두 번째 인자로 메시지 저장
    int port = atoi(argv[3]); // 세 번째 인자를 정수형으로 변환하여 포트 번호 저장

    // 정의한 시스템 콜함수 'mychat' 호출한다. 인자로는 IP 주소, 메시지, 포트 번호를 전달한다.
    long res = syscall(__NR_mychat, ip, msg, port);
    // syscall 호출 결과 확인한다. 만약 성공적으로 실행되었다면 0을 반환한다. 실패한 경우 에러 메시지를 반환한다.
    if (res == 0) {
        // 성공적으로 실행된 경우 메시지 출력되는 메시지
        printf("System call 'mychat' executed successfully.\n");
    } else {
        // 실패한 경우 에러 메시지와 함께 에러 넘버가 출력된다.
        printf("System call 'mychat' failed with error: %ld (errno: %d)\n", res, errno);
    }

    return 0; // 프로그램을 정상적으로 종료한다.
}
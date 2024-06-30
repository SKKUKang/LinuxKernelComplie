#include <linux/kernel.h>     // 커널 관련 함수와 매크로를 사용하기 위해 포함한다.
#include <linux/syscalls.h>   // 시스템 콜 정의를 위해 포함
#include <linux/net.h>        // UDP 통신을 활용해야 하므로 네트워크 관련 구조체와 함수 포함
#include <linux/in.h>         // 인터넷 프로토콜 관련 구조체와 매크로 포함
#include <linux/string.h>     // 메시지 전달을 위해 문자열 처리 함수 포함
#include <linux/errno.h>      // 에러 시 에러코드를 불러오기 위해 에러 정의 포함
#include <linux/slab.h>       // 커널 메모리를 할당하여 전송하므로 함수 포함
#include <linux/socket.h>     // 소켓을 사용하기 위한 매크로 포함
#include <linux/uaccess.h>    // 사용자 공간 데이터 접근 함수 포함
#include <net/sock.h>         // 소켓 관련 핵심 함수 포함
#include <linux/inet.h>       // IP와 인터넷 주소 처리 함수 포함

#define MAX_MSG_LEN 1024      // 최대 메시지 길이 정의
#define ESOCKET -1            // 소켓 생성 실패 시 반환할 에러 코드
#define ESEND -3              // 메시지 전송 실패 시 반환할 에러 코드

// 시스템 콜 함수 정의한다. 인자가 3개이므로 SYSCALL_DEFINE3를 사용한다. C언어와 달리 함수명도 써주고, 인자타입과 인자 명을 쉼표로 구분해준다.
SYSCALL_DEFINE3(mychat, char __user *, ip, char __user *, msg, int, option) {  //char이 아닌 char __user *로 선언해준다.
    char *kernel_ip;         // 커널 공간에 할당할 IP 주소 버퍼
    char *kernel_msg;        // 커널 공간에 할당할 메시지 버퍼
    int ret = 0;             // 반환 값 초기화

    // 커널 메모리를 할당해주는 함수이다. 인자로 할당할 크기와 할당 방식을 넣어준다. GFP_KERNEL은 커널 메모리 할당 방식 중 하나이다.
    kernel_ip = kmalloc(INET_ADDRSTRLEN, GFP_KERNEL);
    if (!kernel_ip) {       //IP에 할당된 메모리가 없다면
        return -ENOMEM;      // ENOMEM은 메모리 할당 실패 시 반환할 에러 코드이다.
    }
    kernel_msg = kmalloc(MAX_MSG_LEN, GFP_KERNEL);
    if (!kernel_msg) {          // 메시지에 할당된 메모리가 없다면
        kfree(kernel_ip);    // 할당된 메모리 해제한다.
        return -ENOMEM;      // 메모리 할당 실패 시 에러 반환한다.
    }

    // 유저 공간에서 커널 공간으로 데이터를 복사해준다. 복사에 실패하면 에러를 반환한다.
    if (copy_from_user(kernel_ip, ip, INET_ADDRSTRLEN) != 0) { 
        ret = -EFAULT;       // 복사 실패 시 에러 반환
        goto cleanup;        // 리소스 정리로 이동한다.  메모리 해제를 위해 goto문을 사용한다.
    }
    if (copy_from_user(kernel_msg, msg, MAX_MSG_LEN) != 0) {
        ret = -EFAULT;       // 마찬가지로 복사 실패 시 에러 반환
        goto cleanup;        // 리소스 정리로 이동
    }

    // 커널 내부 소켓 API를 사용하기 위한 변수 선언한다.
    struct socket *sock;             // 소켓 구조체 포인터
    struct sockaddr_in dest_addr;    // 목적지 주소 구조체
    struct msghdr msg_hdr;           // 메시지 헤더 구조체
    struct kvec iov;                 // I/O 벡터 구조체는 커널 공간의 데이터를 사용하기 위한 구조체이다.

    // UDP 소켓 생성
    ret = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock); //AF_INET은 IPv4 주소 체계, SOCK_DGRAM, IPPROTO_UDP는 UDP 프로토콜을 사용한다. &sock은 소켓 구조체의 주소를 넘겨준다.
    if (ret < 0) {
        ret = ESOCKET;               // 소켓 생성 실패 시 에러 반환
        goto cleanup;                // 리소스 정리로 이동
    }

    // 목적지 주소 설정
    memset(&dest_addr, 0, sizeof(dest_addr)); // 주소 구조체 초기화해준다. &dest_addr는 주소 구조체의 주소를 넘겨주고 0으로 초기화한다. 해당 크기도 명시해준다.
    dest_addr.sin_family = AF_INET;           // 주소 패밀리 설정
    ret = in4_pton(kernel_ip, -1, (u8 *)&dest_addr.sin_addr.s_addr, -1, NULL); // IP 주소를 변환한다. 
    if (ret == 0) {
        ret = -EADDRNOTAVAIL;         // 주소 변환 실패 시 에러 반환
        goto cleanup_sock;            // 소켓 정리로 이동
    }
    dest_addr.sin_port = htons(option); // 포트 번호를 htons 함수를 사용하여 네트워크 바이트 순서로 변환한다.

    // 메시지 헤더 초기화
    memset(&msg_hdr, 0, sizeof(msg_hdr)); // 메시지 헤더 구조체 초기화한다.
    msg_hdr.msg_name = &dest_addr;       // 목적지 주소 설정
    msg_hdr.msg_namelen = sizeof(dest_addr); // 주소 길이 설정

    // I/O 벡터 초기화
    iov.iov_base = kernel_msg;           // 메시지 설정
    iov.iov_len = strlen(kernel_msg);    // 메시지 길이 설정

    // 메시지 전송
    ret = kernel_sendmsg(sock, &msg_hdr, &iov, 1, iov.iov_len); // 메시지 전송 함수를 호출한다.
    if (ret < 0) {
        ret = ESEND;                     // 전송 실패 시 에러 반환
        goto cleanup_sock;               // 소켓 정리로 이동
    }

    ret = 0; // 성공적으로 전송 완료 시 0 반환

cleanup_sock:
    sock_release(sock);                  // 소켓 자원을 해제한다.
cleanup:
    kfree(kernel_ip);                    // 할당된 메모리 해제
    kfree(kernel_msg);                   // 할당된 메모리 해제
    return ret;                          // 최종 반환 값 반환한다.
}

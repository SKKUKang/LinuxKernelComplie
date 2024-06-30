#include <iostream> // 입출력 관련 헤더 파일
#include <string> // 문자열 관련 헤더 파일
#include <cstring> // C 스타일 문자열 처리 헤더 파일
#include <cstdlib> // 일반 유틸리티 함수 헤더 파일
#include <unistd.h> // POSIX 운영체제 API 헤더 파일
#include <sys/socket.h> // 소켓 관련 함수와 데이터 구조 헤더 파일
#include <netinet/in.h> // 인터넷 주소 구조체 헤더 파일

#define BUF_SIZE 1024 // 버퍼 크기를 1024로 정의

void error_handling(const std::string &message); // 에러 핸들링 함수 선언

int main(int argc, char *argv[]) {
    int serv_sock; // 서버 소켓 파일 디스크립터
    struct sockaddr_in serv_addr, clnt_addr; // 서버와 클라이언트 주소 정보 구조체
    socklen_t clnt_addr_size; // 클라이언트 주소 정보 크기
    char message[BUF_SIZE]; // 메시지를 저장할 배열
    int str_len; // 수신한 메시지의 길이

    // 명령줄 인자 검사
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>\n";
        exit(1);
    }

    // 소켓 생성
    serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("UDP socket creation error");

    // 서버 소켓 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr)); // 주소 정보 구조체 초기화
    serv_addr.sin_family = AF_INET; // 주소 체계를 IPv4로 설정
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버의 IP 주소 설정
    serv_addr.sin_port = htons(std::stoi(argv[1])); // 포트 번호 설정

    // 소켓에 주소 할당
    if (bind(serv_sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    clnt_addr_size = sizeof(clnt_addr);
    // 클라이언트로부터 첫 번째 메시지 수신
    str_len = recvfrom(serv_sock, message, BUF_SIZE, 0,
                       reinterpret_cast<struct sockaddr*>(&clnt_addr), &clnt_addr_size);
    if (str_len == -1)
        error_handling("recvfrom() error");
    message[str_len] = 0; // 문자열 끝에 널 문자 추가
    std::cout << "Connected Client 1" << std::endl;
    std::cout << "Received message from client: " << message << std::endl;


    // 소켓 닫기
    close(serv_sock);
    return 0;
}

// 에러 핸들링 함수
void error_handling(const std::string &message) {
    std::cerr << message << std::endl;
    exit(1);
}

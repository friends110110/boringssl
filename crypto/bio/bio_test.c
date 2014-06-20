/* TODO(fork): license */

#define _BSD_SOURCE

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/err.h>


static int test_socket_connect() {
  int listening_sock = socket(AF_INET, SOCK_STREAM, 0);
  int sock;
  struct sockaddr_in sin;
  socklen_t sockaddr_len = sizeof(sin);
  static const char kTestMessage[] = "test";
  char hostname[80], buf[5];
  BIO *bio;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  if (!inet_aton("127.0.0.1", &sin.sin_addr)) {
    perror("inet_aton");
    return 0;
  }

  if (bind(listening_sock, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
    perror("bind");
    return 0;
  }

  if (listen(listening_sock, 1)) {
    perror("listen");
    return 0;
  }

  if (getsockname(listening_sock, (struct sockaddr *)&sin, &sockaddr_len) ||
      sockaddr_len != sizeof(sin)) {
    perror("getsockname");
    return 0;
  }

  snprintf(hostname, sizeof(hostname), "%s:%d", "127.0.0.1",
           ntohs(sin.sin_port));
  bio = BIO_new_connect(hostname);
  if (!bio) {
    fprintf(stderr, "BIO_new_connect failed.\n");
    return 0;
  }

  if (BIO_write(bio, kTestMessage, sizeof(kTestMessage)) !=
      sizeof(kTestMessage)) {
    fprintf(stderr, "BIO_write failed.\n");
    BIO_print_errors_fp(stderr);
    return 0;
  }

  sock = accept(listening_sock, (struct sockaddr *) &sin, &sockaddr_len);
  if (sock < 0) {
    perror("accept");
    return 0;
  }

  if (read(sock, buf, sizeof(buf)) != sizeof(kTestMessage)) {
    perror("read");
    return 0;
  }

  if (memcmp(buf, kTestMessage, sizeof(kTestMessage))) {
    return 0;
  }

  close(sock);
  close(listening_sock);
  BIO_free(bio);

  return 1;
}

int main() {
  ERR_load_crypto_strings();

  if (!test_socket_connect()) {
    return 1;
  }

  printf("PASS\n");
  return 0;
}
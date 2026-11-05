/*
 * AutoSpare Explorer - tiny HTTP server written in C
 * Build: gcc server.c -o server
 * Run:   ./server
 * Open:  http://localhost:8080
 *
 * Serves files from this project folder on Windows, Linux, and macOS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
typedef int socket_length_t;
#define CLOSE_SOCKET closesocket
#else
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int socket_t;
typedef socklen_t socket_length_t;
#define CLOSE_SOCKET close
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

#ifndef PORT
#define PORT 8080
#endif
#define BUFFER_SIZE 8192

static int send_all(socket_t client, const char *data, size_t length) {
    size_t sent = 0;
    while (sent < length) {
        int result = send(client, data + sent, (int)(length - sent), 0);
        if (result <= 0) return 0;
        sent += (size_t)result;
    }
    return 1;
}

#ifdef _WIN32
static void report_socket_error(const char *operation) {
    fprintf(stderr, "%s failed (Winsock error %d).\n", operation, WSAGetLastError());
}
#else
static void report_socket_error(const char *operation) {
    perror(operation);
}
#endif

static const char *content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html; charset=utf-8";
    if (strcmp(ext, ".css") == 0) return "text/css; charset=utf-8";
    if (strcmp(ext, ".js") == 0) return "application/javascript; charset=utf-8";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    return "text/plain; charset=utf-8";
}

static void send_error(socket_t client, int code, const char *status, const char *message) {
    char body[512], header[1024];
    int body_len = snprintf(body, sizeof(body),
        "<!doctype html><html><body style='font-family:Arial;padding:40px'>"
        "<h1>%d %s</h1><p>%s</p><a href='/'>Back home</a></body></html>",
        code, status, message);
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\nContent-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %d\r\nConnection: close\r\n\r\n",
        code, status, body_len);
    if (header_len > 0 && body_len > 0) {
        send_all(client, header, (size_t)header_len);
        send_all(client, body, (size_t)body_len);
    }
}

static void handle_client(socket_t client) {
    char request[BUFFER_SIZE] = {0};
    int received = recv(client, request, (int)sizeof(request) - 1, 0);
    if (received <= 0) return;

    char method[16] = {0}, url[2048] = {0};
    if (sscanf(request, "%15s %2047s", method, url) != 2) {
        send_error(client, 400, "Bad Request", "Could not read the HTTP request.");
        return;
    }
    if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        send_error(client, 405, "Method Not Allowed", "This demo server supports GET and HEAD.");
        return;
    }

    /* Remove query string and map the root URL to index.html. */
    char *query = strchr(url, '?');
    if (query) *query = '\0';
    if (strcmp(url, "/") == 0) strcpy(url, "/index.html");

    /* Basic path traversal protection. */
    if (strstr(url, "..") || strchr(url, '\\')) {
        send_error(client, 403, "Forbidden", "Invalid file path.");
        return;
    }

    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), ".%s", url) >= (int)sizeof(path)) {
        send_error(client, 414, "URI Too Long", "The requested path is too long.");
        return;
    }

    FILE *file = fopen(path, "rb");
    if (!file) {
        send_error(client, 404, "Not Found", "The requested file was not found.");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char header[1024];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n"
        "Cache-Control: no-cache\r\nConnection: close\r\n\r\n",
        content_type(path), file_size);
    if (header_len <= 0 || !send_all(client, header, (size_t)header_len)) {
        fclose(file);
        return;
    }

    if (strcmp(method, "GET") == 0) {
        char chunk[BUFFER_SIZE];
        size_t n;
        while ((n = fread(chunk, 1, sizeof(chunk), file)) > 0) {
            if (!send_all(client, chunk, n)) break;
        }
    }
    fclose(file);
}

int main(void) {
#ifdef _WIN32
    WSADATA winsock_data;
    if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0) {
        report_socket_error("WSAStartup");
        return 1;
    }
#endif

    socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        report_socket_error("socket");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        report_socket_error("bind");
        CLOSE_SOCKET(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    if (listen(server_fd, 10) == SOCKET_ERROR) {
        report_socket_error("listen");
        CLOSE_SOCKET(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("\n  AUTO SPARE EXPLORER\n  C HTTP server running at http://localhost:%d\n"
           "  Press Ctrl+C to stop.\n\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socket_length_t client_len = sizeof(client_addr);
        socket_t client = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client == INVALID_SOCKET) {
#ifdef _WIN32
            if (WSAGetLastError() == WSAEINTR) continue;
#else
            if (errno == EINTR) continue;
#endif
            report_socket_error("accept");
            continue;
        }
        handle_client(client);
        CLOSE_SOCKET(client);
    }
    CLOSE_SOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

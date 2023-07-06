#pragma once

#ifdef __CYGWIN__
#  undef _WIN32
#endif

#include <errno.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <android-base/errors.h>
#include <android-base/macros.h>
#include <android-base/off64_t.h>
#include <android-base/unique_fd.h>
#include <android-base/utf8.h>

#include <unistd.h>
#include <string.h>

#if defined(_WIN32)
struct AdbCloser {
    static void Close(int fd);
};

using unique_fd = android::base::unique_fd_impl<AdbCloser>;
#else
using unique_fd = android::base::unique_fd;
#endif

using android::base::borrowed_fd;

template <typename T>
int adb_close(const android::base::unique_fd_impl<T>&) __attribute__((__unavailable__("adb_close called on unique_fd")));


#if defined(_WIN32)
char* adb_strerror(int err);
#define strerror adb_strerror
#endif

#pragma once

#ifdef __CYGWIN__
#  undef _WIN32
#endif

#include <errno.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <android-base/errors.h>
#include <android-base/macros.h>
#include <android-base/off64_t.h>
#include <android-base/unique_fd.h>
#include <android-base/utf8.h>
#include <unistd.h>
#include <ctype.h>
#include <direct.h>
#include <dirent.h>

#include <fcntl.h>
#include <io.h>
#include <mswsock.h>
#include <process.h>
#include <stdint.h>
#include <sys/stat.h>
#include <utime.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <memory>   // unique_ptr
#include <string>

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <wchar.h>

#if defined(_WIN32)
struct AdbCloser {
    static void Close(int fd);
};

using unique_fd = android::base::unique_fd_impl<AdbCloser>;
#else
using unique_fd = android::base::unique_fd;
#endif

using android::base::borrowed_fd;

template <typename T>
int adb_close(const android::base::unique_fd_impl<T>&) __attribute__((__unavailable__("adb_close called on unique_fd")));


#if defined(_WIN32)
char* adb_strerror(int err);
#define strerror adb_strerror
#endif

int errno_to_wire(int error);
int errno_from_wire(int error);

int network_loopback_client(int port, int type, std::string* error);
int network_loopback_server(int port, int type, std::string* error, bool prefer_ipv4);

#ifdef _WIN32
// stat is broken on Win32: stat on a path with a trailing slash or backslash will always fail with
// ENOENT.
int adb_stat(const char* path, struct adb_stat* buf);
struct adb_stat : public stat {};

#undef stat
#define stat adb_stat

// Windows doesn't have lstat.
#define lstat adb_stat

// mingw doesn't define S_IFLNK or S_ISLNK.
#define S_IFLNK 0120000
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)

// mingw defines S_IFBLK to a different value from bionic.
#undef S_IFBLK
#define S_IFBLK 0060000
#undef S_ISBLK
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#endif

#if defined(__APPLE__)
static inline void* mempcpy(void* dst, const void* src, size_t n) {
    return static_cast<char*>(memcpy(dst, src, n)) + n;
}
#endif
#define OS_PATH_SEPARATORS "\\/"
#define OS_PATH_SEPARATOR '\\'
#define OS_PATH_SEPARATOR_STR "\\"
#define ENV_PATH_SEPARATOR_STR ";"

// Add any additional function declarations or definitions specific to the win32 platform


static inline bool adb_is_separator(char c) {
    return c == '\\' || c == '/';
}

extern int adb_thread_setname(const std::string& name);

static inline void close_on_exec(borrowed_fd fd) {
    // Implementation here if necessary
}

extern int adb_unlink(const char* path);
#undef unlink
#define unlink ___xxx_unlink

extern int adb_mkdir(const std::string& path, int mode);
#undef mkdir
#define mkdir ___xxx_mkdir

extern int adb_rename(const char* oldpath, const char* newpath);

// See the comments for the !defined(_WIN32) versions of adb_*().
extern int adb_open(const char* path, int options);
extern int adb_creat(const char* path, mode_t mode);
extern int adb_read(borrowed_fd fd, void* buf, size_t len);
extern int adb_pread(borrowed_fd fd, void* buf, size_t len, off64_t offset);
extern int adb_write(borrowed_fd fd, const void* buf, size_t len);
extern int adb_pwrite(borrowed_fd fd, const void* buf, size_t len, off64_t offset);
extern off64_t adb_lseek(borrowed_fd fd, off64_t pos, int where);
extern int adb_shutdown(borrowed_fd fd, int direction = SHUT_RDWR);
extern int adb_close(int fd);
extern int adb_register_socket(int s);
extern HANDLE adb_get_os_handle(borrowed_fd fd);

int network_connect(const std::string& host, int port, int type, int timeout, std::string* error);

std::optional<ssize_t> network_peek(borrowed_fd fd);

extern int adb_socket_accept(borrowed_fd serverfd, struct sockaddr* addr, socklen_t* addrlen);

#undef accept
#define accept ___xxx_accept

int adb_getsockname(borrowed_fd fd, struct sockaddr* sockaddr, socklen_t* addrlen);

int adb_socket_get_local_port(borrowed_fd fd);

extern int adb_setsockopt(borrowed_fd fd, int level, int optname, const void* optval, socklen_t optlen);

#undef setsockopt
#define setsockopt ___xxx_setsockopt

extern int adb_socket(int domain, int type, int protocol);

extern int adb_bind(borrowed_fd fd, const sockaddr* addr, int namelen);

extern int adb_socketpair(int sv[2]);

struct adb_msghdr {
    void* msg_name;
    socklen_t msg_namelen;
    struct adb_iovec* msg_iov;
    size_t msg_iovlen;
    void* msg_control;
    size_t msg_controllen;
    int msg_flags;
};

ssize_t adb_sendmsg(borrowed_fd fd, const adb_msghdr* msg, int flags);
ssize_t adb_recvmsg(borrowed_fd fd, adb_msghdr* msg, int flags);

using adb_cmsghdr = struct cmsghdr;

extern adb_cmsghdr* adb_CMSG_FIRSTHDR(adb_msghdr* msgh);
extern adb_cmsghdr* adb_CMSG_NXTHDR(adb_msghdr* msgh, adb_cmsghdr* cmsg);
extern unsigned char* adb_CMSG_DATA(adb_cmsghdr* cmsg);

struct adb_pollfd {
    int fd;
    short events;
    short revents;
};
extern int adb_poll(adb_pollfd* fds, size_t nfds, int timeout);
#define poll ___xxx_poll

static inline int adb_is_absolute_host_path(const char* path) {
    return isalpha(path[0]) && path[1] == ':' && path[2] == '\\';
}

// UTF-8 versions of POSIX APIs.
extern DIR* adb_opendir(const char* dirname);
extern struct dirent* adb_readdir(DIR* dir);
extern int adb_closedir(DIR* dir);

extern int adb_utime(const char*, const struct utimbuf*);
extern int adb_chmod(const char*, mode_t);

extern int adb_fputs(const char* buf, FILE* stream);
extern int adb_fputc(int ch, FILE* stream);
extern int adb_putchar(int ch);
extern int adb_puts(const char* buf);
extern size_t adb_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

extern FILE* adb_fopen(const char* f, const char* m);

extern char* adb_getenv(const char* name);

extern char* adb_getcwd(char* buf, size_t size);
#pragma once


class Process {
public:
    constexpr explicit Process(HANDLE h = nullptr) : h_(h) {}
    constexpr Process(Process&& other) : h_(std::exchange(other.h_, nullptr)) {}
    ~Process() { close(); }
    constexpr explicit operator bool() const { return h_ != nullptr; }

    void wait() {
        if (*this) {
            ::WaitForSingleObject(h_, INFINITE);
            close();
        }
    }

    void kill() {
        if (*this) {
            ::TerminateProcess(h_, -1);
        }
    }

private:
    void close() {
        if (*this) {
            ::CloseHandle(h_);
            h_ = nullptr;
        }
    }

    HANDLE h_;
};

Process adb_launch_process(std::string_view executable, std::vector<std::string> args,
    std::initializer_list<int> fds_to_inherit = {});

class NarrowArgs {
public:
    NarrowArgs(int argc, wchar_t** argv);
    ~NarrowArgs();

    inline char** data() {
        return narrow_args;
    }

private:
    char** narrow_args;
};

static inline int unix_open(std::string_view path, int options, ...) {
    std::string zero_terminated(path.begin(), path.end());
    if ((options & O_CREAT) == 0) {
        return TEMP_FAILURE_RETRY(open(zero_terminated.c_str(), options));
    }
    else {
        int mode;
        va_list args;
        va_start(args, options);
        mode = va_arg(args, int);
        va_end(args);
        return TEMP_FAILURE_RETRY(open(zero_terminated.c_str(), options, mode));
    }
}

static inline int adb_open_mode(const char* pathname, int options, int mode) {
    return TEMP_FAILURE_RETRY(open(pathname, options, mode));
}

inline int network_local_client(const char* name, int namespace_id, int type, std::string* error) {
    return _fd_set_error_str(socket_local_client(name, namespace_id, type), error);
}

inline int network_local_server(const char* name, int namespace_id, int type, std::string* error) {
    return _fd_set_error_str(socket_local_server(name, namespace_id, type), error);
}

int network_connect(const std::string& host, int port, int type, int timeout, std::string* error);

inline std::optional<ssize_t> network_peek(borrowed_fd fd) {
    ssize_t ret = recv(fd.get(), nullptr, 0, MSG_PEEK | MSG_TRUNC);
    return ret == -1 ? std::nullopt : std::make_optional(ret);
}

static inline int adb_socket_accept(borrowed_fd serverfd, struct sockaddr* addr,
    socklen_t* addrlen) {
    int fd;

    fd = TEMP_FAILURE_RETRY(accept(serverfd.get(), addr, addrlen));
    if (fd >= 0) close_on_exec(fd);

    return fd;
}

std::string adb_version();

uint32_t calculate_apacket_checksum(const apacket* p);

enum class ConnectionState {
    kCsOffline,
    kCsBootloader,
    kCsDevice,
    kCsHost,
    kCsRecovery,
    kCsRescue,
    kCsNoPerm,
    kCsSideload,
    kCsUnauthorized,
    kCsAuthorizing,
    kCsConnecting
};

std::string to_string(ConnectionState state);

apacket* get_apacket(void);

void put_apacket(apacket* p);

void handle_online(atransport* t);

std::string get_connection_string();

void send_tls_request(atransport* t);

void send_connect(atransport* t);
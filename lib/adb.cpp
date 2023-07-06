

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <direct.h>
#include <dirent.h>
#include <errno.h>
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
#include "adb_sysdeps.h"

int adb_unlink(const char* path) {
    if (unlink(path) == -1) {
        // Handle error if necessary
        return -1;
    }
    return 0;
}

int adb_mkdir(const std::string& path, int mode) {
    if (mkdir(path.c_str(), mode) == -1) {
        // Handle error if necessary
        return -1;
    }
    return 0;
}

int adb_rename(const char* oldpath, const char* newpath) {
    if (rename(oldpath, newpath) == -1) {
        // Handle error if necessary
        return -1;
    }
    return 0;
}

int adb_open(const char* path, int options) {
    int fd = open(path, options);
    if (fd == -1) {
        // Handle error if necessary
        return -1;
    }
    return fd;
}

int adb_creat(const char* path, mode_t mode) {
    int fd = creat(path, mode);
    if (fd == -1) {
        // Handle error if necessary
        return -1;
    }
    return fd;
}


int adb_read(borrowed_fd fd, void* buf, size_t len) {
    ssize_t bytesRead = read(fd.get(), buf, len);
    if (bytesRead == -1) {
        // Handle error if necessary
        return -1;
    }
    return bytesRead;
}

int adb_pread(borrowed_fd fd, void* buf, size_t len, off64_t offset) {
    ssize_t bytesRead = pread(fd.get(), buf, len, offset);
    if (bytesRead == -1) {
        // Handle error if necessary
        return -1;
    }
    return bytesRead;
}

int adb_write(borrowed_fd fd, const void* buf, size_t len) {
    ssize_t bytesWritten = write(fd.get(), buf, len);
    if (bytesWritten == -1) {
        // Handle error if necessary
        return -1;
    }
    return bytesWritten;
}

int adb_pwrite(borrowed_fd fd, const void* buf, size_t len, off64_t offset) {
    ssize_t bytesWritten = pwrite(fd.get(), buf, len, offset);
    if (bytesWritten == -1) {
        // Handle error if necessary
        return -1;
    }
    return bytesWritten;
}

off64_t adb_lseek(borrowed_fd fd, off64_t pos, int where) {
    off64_t newPosition = lseek(fd.get(), pos, where);
    if (newPosition == -1) {
        // Handle error if necessary
        return -1;
    }
    return newPosition;
}

int adb_shutdown(borrowed_fd fd, int direction) {
    if (shutdown(fd.get(), direction) == -1) {
        // Handle error if necessary
        return -1;
    }
    return 0;
}

int adb_close(int fd) {
    if (close(fd) == -1) {
        // Handle error if necessary
        return -1;
    }
    return 0;
}

int adb_register_socket(int s) {

    // Register the socket 's' and return a unique identifier
    return 0;
}

HANDLE adb_get_os_handle(borrowed_fd fd) {
    return nullptr;
}


std::string adb_version() {
    // Don't change the format of this --- it's parsed by ddmlib.
    return android::base::StringPrintf(
        "Android Debug Bridge version %d.%d.%d\n"
        "Version %s-%s\n"
        "Installed as %s\n"
        "Running on %s\n",
        ADB_VERSION_MAJOR, ADB_VERSION_MINOR, ADB_SERVER_VERSION, PLATFORM_TOOLS_VERSION,
        android::build::GetBuildNumber().c_str(), android::base::GetExecutablePath().c_str(),
        GetOSVersion().c_str());
}

uint32_t calculate_apacket_checksum(const apacket* p) {
    uint32_t sum = 0;
    for (size_t i = 0; i < p->msg.data_length; ++i) {
        sum += static_cast<uint8_t>(p->payload[i]);
    }
    return sum;
}

std::string to_string(ConnectionState state) {
    switch (state) {
    case ConnectionState::kCsOffline:
        return "offline";
    case ConnectionState::kCsBootloader:
        return "bootloader";
    case ConnectionState::kCsDevice:
        return "device";
    case ConnectionState::kCsHost:
        return "host";
    case ConnectionState::kCsRecovery:
        return "recovery";
    case ConnectionState::kCsRescue:
        return "rescue";
    case ConnectionState::kCsNoPerm:
        return UsbNoPermissionsShortHelpText();
    case ConnectionState::kCsSideload:
        return "sideload";
    case ConnectionState::kCsUnauthorized:
        return "unauthorized";
    case ConnectionState::kCsAuthorizing:
        return "authorizing";
    case ConnectionState::kCsConnecting:
        return "connecting";
    default:
        return "unknown";
    }
}

apacket* get_apacket(void) {
    apacket* p = new apacket();
    if (p == nullptr) {
        LOG(FATAL) << "failed to allocate an apacket";
    }

    memset(&p->msg, 0, sizeof(p->msg));
    return p;
}

void put_apacket(apacket* p) {
    delete p;
}

void handle_online(atransport* t) {
    D("adb: online");
    t->online = 1;
#if ADB_HOST
    t->SetConnectionEstablished(true);
    #elifdefined(__ANDROID__)
        IncrementActiveConnections();
#endif



    void handle_offline(atransport * t) {
        if (t->GetConnectionState() == ConnectionState::kCsOffline) {
            LOG(INFO) << t->serial_name() << ": already offline";
            return;
        }

        LOG(INFO) << t->serial_name() << ": offline";

#if !ADB_HOST && defined(__ANDROID__)
        DecrementActiveConnections();
#endif

        t->SetConnectionState(ConnectionState::kCsOffline);

        // Close the associated usb
        t->online = 0;

        // This is necessary to avoid a race condition that occurred when a transport closes
        // while a client socket is still active.
        close_all_sockets(t);

        t->RunDisconnects();
    }

#if DEBUG_PACKETS
    void print_packet(const char* label, apacket * p) {
        const char* tag;
        unsigned count;

        switch (p->msg.command) {
        case A_SYNC:
            tag = "SYNC";
            break;
        case A_CNXN:
            tag = "CNXN";
            break;
        case A_OPEN:
            tag = "OPEN";
            break;
        case A_OKAY:
            tag = "OKAY";
            break;
        case A_CLSE:
            tag = "CLSE";
            break;
        case A_WRTE:
            tag = "WRTE";
            break;
        case A_AUTH:
            tag = "AUTH";
            break;
        case A_STLS:
            tag = "STLS";
            break;
        default:
            tag = "????";
            break;
        }

        fprintf(stderr, "%s: %s %08x %08x %04x \"",
            label, tag, p->msg.arg0, p->msg.arg1, p->msg.data_length);
        count = p->msg.data_length;
        const char* x = p->payload.data();
        if (count > DUMPMAX) {
            count = DUMPMAX;
            tag = "\n";
        }
        else {
            tag = "\"\n";
        }
        while (count-- > 0) {
            if ((*x >= ' ') && (*x < 127)) {
                fputc(*x, stderr);
            }
            else {
                fputc('.', stderr);
            }
            x++;
        }
        fputs(tag, stderr);
    }
#endif

#include "adb_connection.h"

    std::string get_connection_string() {
        std::vector<std::string> connection_properties;

#if !ADB_HOST
        static const char* cnxn_props[] = {
            "ro.product.name",
            "ro.product.model",
            "ro.product.device",
        };

        for (const auto& prop : cnxn_props) {
            std::string value = std::string(prop) + "=" + android::base::GetProperty(prop, "");
            connection_properties.push_back(value);
        }
#endif

        connection_properties.push_back(android::base::StringPrintf(
            "features=%s", FeatureSetToString(supported_features()).c_str()));

        return android::base::StringPrintf(
            "%s::%s", adb_device_banner,
            android::base::Join(connection_properties, ';').c_str());
    }

    void send_tls_request(atransport * t) {
        D("Calling send_tls_request");
        apacket* p = get_apacket();
        p->msg.command = A_STLS;
        p->msg.arg0 = A_STLS_VERSION;
        p->msg.data_length = 0;
        send_packet(p, t);
    }

    void send_connect(atransport * t) {
        D("Calling send_connect");
        apacket* cp = get_apacket();
        cp->msg.command = A_CNXN;
        // Send the max supported version, but because the transport is
        // initialized to A_VERSION_MIN, this will be compatible with every
        // device.
        cp->msg.arg0 = A_VERSION;
        cp->msg.arg1 = t->get_max_payload();

        std::string connection_str = get_connection_string();
        // Connect and auth packets are limited to MAX_PAYLOAD_V1 because we don't
        // yet know how much data the other size is willing to accept.
        if (connection_str.length() > MAX_PAYLOAD_V1) {
            LOG(FATAL) << "Connection banner is too long (length = "
                << connection_str.length() << ")";
        }

        cp->payload.assign(connection_str.begin(), connection_str.end());
        cp->msg.data_length = cp->payload.size();

        send_packet(cp, t);
    }
}
void parse_banner(const std::string& banner, atransport* t) {
    D("parse_banner: %s", banner.c_str());

    // The format is something like:
    // "device::ro.product.name=x;ro.product.model=y;ro.product.device=z;".
    std::vector<std::string> pieces = android::base::Split(banner, ":");

    // Reset the features list or else if the server sends no features we may
    // keep the existing feature set (http://b/24405971).
    t->SetFeatures("");

    if (pieces.size() > 2) {
        const std::string& props = pieces[2];
        for (const auto& prop : android::base::Split(props, ";")) {
            // The list of properties was traditionally ;-terminated rather than ;-separated.
            if (prop.empty()) continue;

            std::vector<std::string> key_value = android::base::Split(prop, "=");
            if (key_value.size() != 2) continue;

            const std::string& key = key_value[0];
            const std::string& value = key_value[1];
            if (key == "ro.product.name") {
                t->product = value;
            }
            else if (key == "ro.product.model") {
                t->model = value;
            }
            else if (key == "ro.product.device") {
                t->device = value;
            }
            else if (key == "features") {
                t->SetFeatures(value);
            }
        }
    }

    const std::string& type = pieces[0];
    if (type == "bootloader") {
        D("setting connection_state to kCsBootloader");
        t->SetConnectionState(kCsBootloader);
    }
    else if (type == "device") {
        D("setting connection_state to kCsDevice");
        t->SetConnectionState(kCsDevice);
    }
    else if (type == "recovery") {
        D("setting connection_state to kCsRecovery");
        t->SetConnectionState(kCsRecovery);
    }
    else if (type == "sideload") {
        D("setting connection_state to kCsSideload");
        t->SetConnectionState(kCsSideload);
    }
    else if (type == "rescue") {
        D("setting connection_state to kCsRescue");
        t->SetConnectionState(kCsRescue);
    }
    else {
        D("setting connection_state to kCsHost");
        t->SetConnectionState(kCsHost);
    }
}

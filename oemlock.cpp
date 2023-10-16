#include <optional>
#include <sstream>

#include <OemLockClient.h>
#include <android/hardware/oemlock/1.0/IOemLock.h>
#include <sysexits.h>

using android::sp;

using aidl::android::hardware::oemlock::OemLockSecureStatus;

using android::hal::OemLockClient;
using android::hal::OemLockVersion;

static void usage(FILE* where, int /* argc */, char* argv[]) {
    fprintf(where,
            "%s - command-line wrapper for the oemlock HAL.\n"
            "\n"
            "Usage:\n"
            "  %s COMMAND\n"
            "\n"
            "Commands:\n"
            "  hal-info                         - Show info about oemlock HAL used.\n"
            "  is-oem-unlock-allowed-by-carrier - Returns 0 only if OEM unlock is allowed by the carrier.\n"
            "  is-oem-unlock-allowed-by-device  - Returns 0 only if OEM unlock ia allowed by the device.\n",
            argv[0], argv[0]);
}

static int handle_return(const std::optional<bool>& ret, const char* errStr) {
    if (!ret.has_value()) {
        fprintf(stderr, errStr, "");
        return EX_SOFTWARE;
    }
    if (ret.value()) {
        printf("%d\n", ret.value());
        return EX_OK;
    }
    printf("%d\n", ret.value());
    return EX_SOFTWARE;
}

static constexpr auto ToString(OemLockVersion ver) {
    switch (ver) {
        case OemLockVersion::OEMLOCK_V1_0:
            return "android.hardware.oemlock@1.0::IOemLock";
        case OemLockVersion::OEMLOCK_AIDL:
            return "android.hardware.oemlock@aidl::IOemLock";
    }
}

static int do_hal_info(const OemLockClient* module) {
    fprintf(stdout, "HAL Version: %s\n", ToString(module->GetVersion()));
    return EX_OK;
}

static int do_is_oem_unlock_allowed_by_carrier(OemLockClient* module) {
    const auto ret = module->IsOemUnlockAllowedByCarrier();
    return handle_return(ret, "Error calling isOemUnlockAllowedByCarrier()\n");
}

static int do_is_oem_unlock_allowed_by_device(OemLockClient* module) {
    const auto ret = module->IsOemUnlockAllowedByDevice();
    return handle_return(ret, "Error calling isOemUnlockAllowedByDevice()\n");
}

int main(int argc, char* argv[]) {
    const auto client = android::hal::OemLockClient::WaitForService();
    if (client == nullptr) {
        fprintf(stderr, "Failed to get oemlock module.\n");
        return EX_SOFTWARE;
    }

    if (argc < 2) {
        usage(stderr, argc, argv);
        return EX_USAGE;
    }

    if (strcmp(argv[1], "hal-info") == 0) {
        return do_hal_info(client.get());
    } else if (strcmp(argv[1], "is-oem-unlock-allowed-by-carrier") == 0) {
        return do_is_oem_unlock_allowed_by_carrier(client.get());
    } else if (strcmp(argv[1], "is-oem-unlock-allowed-by-device") == 0) {
        return do_is_oem_unlock_allowed_by_device(client.get());
    }

    // Parameter not matched, print usage
    usage(stderr, argc, argv);
    return EX_USAGE;
}

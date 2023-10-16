#ifndef __OEMLOCK_OEMLOCKCLIENT_H_
#define __OEMLOCK_OEMLOCKCLIENT_H_

#include <aidl/android/hardware/oemlock/OemLockSecureStatus.h>

#include <stdint.h>

#include <memory>
#include <optional>

namespace android::hal {

enum class OemLockVersion { OEMLOCK_V1_0, OEMLOCK_AIDL };

class OemLockClient {
public:
    using OemLockSecureStatus = aidl::android::hardware::oemlock::OemLockSecureStatus;
    virtual ~OemLockClient() = default;
    virtual OemLockVersion GetVersion() const = 0;
    // Check if OEM unlock is allowed by the carrier. Return empty optional if the RPC call failed.
    [[nodiscard]] virtual std::optional<bool> IsOemUnlockAllowedByCarrier() const = 0;

    // Check if OEM unlock is allowed by the device. Return empty optional if the RPC call failed.
    [[nodiscard]] virtual std::optional<bool> IsOemUnlockAllowedByDevice() const = 0;

    [[nodiscard]] static std::unique_ptr<OemLockClient> WaitForService();
};

}  // namespace android::hal

#endif //__OEMLOCK_OEMLOCKCLIENT_H_

#include <OemLockClient.h>

#include <aidl/android/hardware/oemlock/IOemLock.h>
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/hardware/oemlock/1.0/IOemLock.h>
#include "utils/StrongPointer.h"

using android::hardware::oemlock::V1_0::OemLockStatus;

namespace android::hal {
class OemLockClientAidl final : public OemLockClient {
    using IOemLock = ::aidl::android::hardware::oemlock::IOemLock;

  public:
    OemLockClientAidl(std::shared_ptr<IOemLock> module) : module_(module) {}

    OemLockVersion GetVersion() const override { return OemLockVersion::OEMLOCK_AIDL; }

    ~OemLockClientAidl() = default;
    std::optional<bool> IsOemUnlockAllowedByCarrier() const {
        bool ret = false;
        const auto status = module_->isOemUnlockAllowedByCarrier(&ret);
        if (!status.isOk()) {
            LOG(ERROR) << __FUNCTION__ << "()"
                       << " failed " << status.getDescription();
            return {};
        }
        return ret;
    }

    std::optional<bool> IsOemUnlockAllowedByDevice() const {
        bool ret = false;
        const auto status = module_->isOemUnlockAllowedByDevice(&ret);
        if (!status.isOk()) {
            LOG(ERROR) << __FUNCTION__ << "()"
                       << " failed " << status.getDescription();
            return {};
        }
        return ret;
    }

private:
    const std::shared_ptr<IOemLock> module_;
};

using namespace android::hardware::oemlock;

class OemLockClientHIDL final : public OemLockClient {
  public:
    OemLockClientHIDL(android::sp<V1_0::IOemLock> module_v1)
        : module_v1_(module_v1) {
        CHECK(module_v1_ != nullptr);
    }
    OemLockVersion GetVersion() const override { return OemLockVersion::OEMLOCK_V1_0; }
    std::optional<bool> IsOemUnlockAllowedByCarrier() const {
        bool allowed;
        OemLockStatus status;
        const auto ret = module_v1_->isOemUnlockAllowedByCarrier([&](OemLockStatus s, bool a) {
            status = s;
            allowed = a;
        });
        if (!ret.isOk()) {
            LOG(ERROR) << __FUNCTION__ << "()"
                       << " failed " << ret.description();
            return {};
        }
        if (status == V1_0::OemLockStatus::FAILED) {
            return {};
        }
        return allowed;
    }

    std::optional<bool> IsOemUnlockAllowedByDevice() const {
        bool allowed;
        OemLockStatus status;
        const auto ret = module_v1_->isOemUnlockAllowedByDevice([&](OemLockStatus s, bool a) {
            status = s;
            allowed = a;
        });
        if (!ret.isOk()) {
            LOG(ERROR) << __FUNCTION__ << "()"
                       << " failed " << ret.description();
            return {};
        }
        if (status == V1_0::OemLockStatus::FAILED) {
            return {};
        }
        return allowed;
    }

private:
    android::sp<V1_0::IOemLock> module_v1_;
};

std::unique_ptr<OemLockClient> OemLockClient::WaitForService() {
    const auto instance_name =
            std::string(::aidl::android::hardware::oemlock::IOemLock::descriptor) + "/default";

    if (AServiceManager_isDeclared(instance_name.c_str())) {
        auto module = ::aidl::android::hardware::oemlock::IOemLock::fromBinder(
                ndk::SpAIBinder(AServiceManager_waitForService(instance_name.c_str())));
        if (module == nullptr) {
            LOG(ERROR) << "AIDL " << instance_name
                       << " is declared but waitForService returned nullptr.";
            return nullptr;
        }
        LOG(INFO) << "Using AIDL version of IOemLock";
        return std::make_unique<OemLockClientAidl>(module);
    }
    LOG(INFO) << "AIDL IOemLock not available, falling back to HIDL.";

    android::sp<V1_0::IOemLock> v1_0_module;
    v1_0_module = V1_0::IOemLock::getService();
    if (v1_0_module == nullptr) {
        LOG(ERROR) << "Error getting oemlock v1.0 module.";
        return nullptr;
    }
    LOG(INFO) << "Using HIDL version 1.0 of IOemLock";

    return std::make_unique<OemLockClientHIDL>(v1_0_module);
}

}  // namespace android::hal

#ifndef URCHINENGINE_SYSTEMINFO_H
#define URCHINENGINE_SYSTEMINFO_H

#include <string>

#include "pattern/singleton/Singleton.h"

namespace urchin {

    class SystemInfo : public Singleton<SystemInfo> {
        public:
            friend class Singleton<SystemInfo>;

            std::string retrieveOsInfo() const;
            unsigned int retrieveCpuCores() const;
            uint64_t retrieveTotalMemory() const;
            std::string retrieveGraphicsCardNames() const;

            std::string systemHash() const;

        private:
            SystemInfo() = default;
            ~SystemInfo() override = default;

            std::string retrieveCpuHash() const;
    };

}

#endif

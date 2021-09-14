#include <cassert>

#include <io/map/MapSerializer.h>
#include <util/StringUtil.h>

namespace urchin {

    std::string MapSerializer::serialize(const std::map<std::string, std::string>& map) {
        std::string mapString;
        for (const auto& [key, value] : map) {
            mapString += escape(key) + DELIMITER + escape(value) + DELIMITER;
        }
        if (!map.empty()) {
            mapString.pop_back();
        }

        return mapString;
    }

    std::map<std::string, std::string> MapSerializer::deserialize(const std::string& mapString) {
        std::vector<std::string> mapListString;
        std::vector<std::string> splitStrings = StringUtil::split(mapString, DELIMITER);

        bool appendNextElement = false;
        for (const auto& elem : splitStrings) {
            if (appendNextElement) {
                mapListString.back() = mapListString.back() + DELIMITER + elem;
            } else {
                mapListString.emplace_back(elem);
            }

            std::string updatedElem = elem;
            StringUtil::replaceAll(updatedElem, "\\\\", "");
            appendNextElement = !updatedElem.empty() && updatedElem.back() == '\\';
        }

        std::map<std::string, std::string> outputMap;
        assert(mapListString.size() % 2 == 0);

        for (std::size_t i = 0; i < mapListString.size(); i += 2) {
            std::string key = unescape(mapListString[i]);
            std::string value = unescape(mapListString[i + 1]);
            outputMap.emplace(key, value);
        }
        return outputMap;
    }

    std::string MapSerializer::escape(const std::string& str) {
        std::string escapedStr = str;
        StringUtil::replaceAll(escapedStr, "\\", "\\\\");
        StringUtil::replaceAll(escapedStr, std::string(1, DELIMITER), std::string("\\") + DELIMITER);
        return escapedStr;
    }

    std::string MapSerializer::unescape(const std::string& str) {
        std::string unescapedStr = str;
        StringUtil::replaceAll(unescapedStr, std::string("\\") + DELIMITER, std::string(1, DELIMITER));
        StringUtil::replaceAll(unescapedStr, "\\\\", "\\");
        return unescapedStr;
    }

}

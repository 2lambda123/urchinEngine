#include <stdexcept>

#include <io/data/XmlParser.h>
#include <config/FileSystem.h>

namespace urchin {

    XmlParser::XmlParser(const std::string& filename) :
        XmlParser(filename, FileSystem::instance()->getResourcesDirectory()) {

    }

    /**
     * @param workingDirectory Override the default working directory
     */
    XmlParser::XmlParser(const std::string& filename, const std::string& workingDirectory) : doc() {
        this->filenamePath = workingDirectory + filename;
        this->doc = new TiXmlDocument(filenamePath.c_str());

        if (!doc->LoadFile()) {
            delete doc;
            throw std::invalid_argument("Cannot open or load the file " + filenamePath + ".");
        }
    }

    XmlParser::~XmlParser() {
        delete doc;
    }

    std::unique_ptr<XmlChunk> XmlParser::getRootChunk() const {
        const TiXmlNode* rootNode = doc->FirstChild()->NextSibling();
        if (rootNode->Type() == TiXmlNode::ELEMENT) {
            return std::unique_ptr<XmlChunk>(new XmlChunk(doc->FirstChild()->NextSibling()->ToElement()));
        }

        throw std::invalid_argument("Impossible to get root element in file: " + filenamePath);
    }

    /**
     * Returns a XML chunks according to the parameters (info: <parent><chunkName attributeName="attributeValue">value</chunkName></parent>)
     * @param chunkName Name of the tag XML
     * @param attribute Name and value of the attribute
     * @param parent Name of the tag parent of "chunkName"
     * @return XML chunks according to the parameters
     */
    std::vector<std::unique_ptr<XmlChunk>> XmlParser::getChunks(const std::string& chunkName, const DataAttribute& attribute, const XmlChunk* parent) const {
        std::vector<std::unique_ptr<XmlChunk>> chunks;

        const TiXmlNode *firstChild;
        if (!parent) {
            firstChild = doc->FirstChild()->NextSibling()->FirstChild();
        } else {
            firstChild = parent->getChunk()->FirstChild();
        }

        //seek the good chunk
        for (const TiXmlNode *pChild = firstChild; pChild != nullptr; pChild = pChild->NextSibling()) {
            if (pChild->Type() == TiXmlNode::ELEMENT && pChild->Value() == chunkName) {
                if (!attribute.getAttributeName().empty()) {
                    const std::string* attributeValue = pChild->ToElement()->Attribute(attribute.getAttributeName());
                    if (attributeValue && (*attributeValue) == attribute.getAttributeValue()) {
                        chunks.push_back(std::unique_ptr<XmlChunk>(new XmlChunk(pChild->ToElement())));
                    }
                } else {
                    chunks.push_back(std::unique_ptr<XmlChunk>(new XmlChunk(pChild->ToElement())));
                }
            }
        }

        return chunks;
    }

    /**
     * Returns a unique XML chunk according to the parameters (info: <parent><chunkName attributeName="attributeValue">value</chunkName></parent>)
     * @param mandatory Chunk must exist otherwise an exception is throw
     * @param chunkName Name of the tag XML
     * @param attribute Name and value of the attribute
     * @param parent Name of the tag parent of "chunkName"
     * @return Unique XML chunk according to the parameters
     */
    std::unique_ptr<XmlChunk> XmlParser::getUniqueChunk(bool mandatory, const std::string& chunkName, const DataAttribute& attribute, const XmlChunk* parent) const {
        auto chunks = getChunks(chunkName, attribute, parent);

        if (chunks.size() > 1) {
            throw std::invalid_argument("More than one tag found for " + getChunkDescription(chunkName, attribute) + ".");
        } else if (chunks.empty()) {
            if (mandatory) {
                throw std::invalid_argument("The tag " + getChunkDescription(chunkName, attribute) + " wasn't found in file " + filenamePath + ".");
            }

            return std::unique_ptr<XmlChunk>(nullptr);
        }

        return std::move(chunks[0]);
    }

    std::string XmlParser::getChunkDescription(const std::string& chunkName, const DataAttribute& attribute) const {
        if (attribute.getAttributeName().empty()) {
            return chunkName;
        } else {
            return chunkName + " with the attribute " + attribute.getAttributeName() + "=\"" + attribute.getAttributeValue() + "\"";
        }
    }

}

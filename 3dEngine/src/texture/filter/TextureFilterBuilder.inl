template<class T> TextureFilterBuilder<T>::TextureFilterBuilder() :
    textureWidth(1024),
    textureHeight(1024),
    pTextureType(TextureType::DEFAULT),
    pTextureNumberLayer(1),
    pTextureFormat(TextureFormat::RGB_8_INT) {

}

template<class T> TextureFilterBuilder<T>::~TextureFilterBuilder() {

}

template<class T> T *TextureFilterBuilder<T>::textureSize(unsigned int textureWidth, unsigned int textureHeight) {
    this->textureWidth = textureWidth;
    this->textureHeight = textureHeight;
    return _this();
}

template<class T> unsigned int TextureFilterBuilder<T>::getTextureWidth() const {
    return textureWidth;
}

template<class T> unsigned int TextureFilterBuilder<T>::getTextureHeight() const {
    return textureHeight;
}

template<class T> T *TextureFilterBuilder<T>::textureType(TextureType textureType) {
    this->pTextureType = textureType;
    return _this();
}

template<class T> TextureType TextureFilterBuilder<T>::getTextureType() const {
    return pTextureType;
}

template<class T> T *TextureFilterBuilder<T>::textureNumberLayer(unsigned int textureNumberLayer) {
    this->pTextureNumberLayer = textureNumberLayer;
    return _this();
}

template<class T> unsigned int TextureFilterBuilder<T>::getTextureNumberLayer() const {
    return pTextureNumberLayer;
}

template<class T> T *TextureFilterBuilder<T>::textureFormat(TextureFormat textureFormat) {
    this->pTextureFormat = textureFormat;
    return _this();
}

template<class T> TextureFormat TextureFilterBuilder<T>::getTextureFormat() const {
    return pTextureFormat;
}

template<class T> T* TextureFilterBuilder<T>::_this() {
    return static_cast<T*>(this);
}
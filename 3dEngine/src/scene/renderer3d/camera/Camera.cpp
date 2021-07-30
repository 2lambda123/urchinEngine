#include <limits>

#include <scene/renderer3d/camera/Camera.h>

namespace urchin {

    /**
    * @param angle Angle of the field of view (fovy)
    */
    Camera::Camera(float angle, float nearPlane, float farPlane) :
            MOUSE_SENSITIVITY_FACTOR(ConfigService::instance().getFloatValue("camera.mouseSensitivityFactor")),
            mView(Matrix4<float>()),
            mProjection(Matrix4<float>()),
            position(Point3<float>(0.0f, 0.0f, 0.0f)),
            view(Vector3<float>(0.0f, 0.0f, -1.0f)),
            up(Vector3<float>(0.0f, 1.0f, 0.0f)),
            angle(angle),
            nearPlane(nearPlane),
            farPlane(farPlane),
            baseFrustum(Frustum<float>(angle, 1.0f, nearPlane, farPlane)),
            frustum(Frustum<float>(angle, 1.0f, nearPlane, farPlane)),
            maxRotationX(0.995f),
            distance(0.0f),
            bUseMouse(false),
            mouseSensitivityPercentage(1.0f),
            invertYAxis(false),
            sceneWidth(0),
            sceneHeight(0),
            previousMouseX(0.0),
            previousMouseY(0.0) {
        resetPreviousMousePosition();
    }

    void Camera::initialize(unsigned int sceneWidth, unsigned int sceneHeight) {
        initializeOrUpdate(sceneWidth, sceneHeight);
    }

    void Camera::onResize(unsigned int sceneWidth, unsigned int sceneHeight) {
        initializeOrUpdate(sceneWidth, sceneHeight);
    }

    unsigned int Camera::getSceneWidth() const {
        return sceneWidth;
    }

    unsigned int Camera::getSceneHeight() const {
        return sceneHeight;
    }

    void Camera::initializeOrUpdate(unsigned int sceneWidth, unsigned int sceneHeight) {
        this->sceneWidth = sceneWidth;
        this->sceneHeight = sceneHeight;

        //projection matrix
        float fov = 1.0f / std::tan((angle * MathValue::PI_FLOAT) / 360.0f);
        float ratio = (float)sceneWidth / (float)sceneHeight;
        mProjection.setValues(
                fov/ratio, 0.0f, 0.0f, 0.0f,
                0.0f, -fov, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f * ((farPlane + nearPlane) / (nearPlane - farPlane)) - 0.5f, (farPlane * nearPlane) / (nearPlane - farPlane),
                0.0f, 0.0f, -1.0f, 0.0f);

        //frustum
        baseFrustum.buildFrustum(angle, ratio, nearPlane, farPlane);
        frustum = baseFrustum * mView.inverse();
    }

    void Camera::resetPreviousMousePosition(double previousMouseX, double previousMouseY) {
        if (bUseMouse) {
            this->previousMouseX = previousMouseX;
            this->previousMouseY = previousMouseY;
        }
    }

    void Camera::useMouseToMoveCamera(bool use) {
        bUseMouse = use;

        if (bUseMouse) {
            resetPreviousMousePosition();
        }
    }

    bool Camera::isUseMouseToMoveCamera() const {
        return bUseMouse;
    }

    void Camera::setMouseSensitivityPercentage(float mouseSensitivityPercentage) {
        this->mouseSensitivityPercentage = mouseSensitivityPercentage;
    }

    void Camera::setInvertYAxis(bool invertYAxis) {
        this->invertYAxis = invertYAxis;
    }

    /**
    * @param distance Distance between the camera and the rotation point (0 : first person camera | >0 : third person camera)
    */
    void Camera::setDistance(float distance) {
        this->distance = distance;
    }

    bool Camera::isFirstPersonCamera() const {
        return MathFunction::isZero(distance);
    }

    void Camera::setMaxRotationX(float maxRotationX) {
        this->maxRotationX = maxRotationX;
    }

    const Matrix4<float>& Camera::getViewMatrix() const {
        return mView;
    }

    const Matrix4<float>& Camera::getProjectionMatrix() const {
        return mProjection;
    }

    const Point3<float>& Camera::getPosition() const {
        return position;
    }

    const Vector3<float>& Camera::getView() const {
        return view;
    }

    const Vector3<float>& Camera::getUp() const {
        return up;
    }

    const Quaternion<float>& Camera::getOrientation() const {
        return orientation;
    }

    float Camera::getAngle() const {
        return angle;
    }

    float Camera::getNearPlane() const {
        return nearPlane;
    }

    float Camera::getFarPlane() const {
        return farPlane;
    }

    const Frustum<float>& Camera::getFrustum() const {
        return frustum;
    }

    void Camera::moveTo(const Point3<float>& position) {
        this->position = position;

        updateCameraComponents();
    }

    void Camera::moveOnLocalXAxis(float distance) {
        Vector3<float> localXAxis = up.crossProduct(view).normalize();
        position = position.translate(localXAxis * distance);

        updateCameraComponents();
    }

    void Camera::moveOnLocalZAxis(float distance) {
        Vector3<float> localZAxis = view;
        position = position.translate(localZAxis * distance);

        updateCameraComponents();
    }

    void Camera::lookAt(const Vector3<float>& view) {
        this->view = view.normalize();

        updateCameraComponents();
    }

    void Camera::rotate(const Quaternion<float>& rotationDelta) {
        Point3<float> pivot = position;
        if (!isFirstPersonCamera()) {
            pivot = position.translate(view * distance);
        }

        //moves view point
        Point3<float> viewPoint = pivot.translate(rotationDelta.rotateVector(view));
        view = position.vector(viewPoint).normalize();

        //moves up vector
        up = rotationDelta.rotateVector(up);

        //moves position point
        if (!isFirstPersonCamera()) {
            Vector3<float> axis = pivot.vector(position);
            position = pivot.translate(rotationDelta.rotateVector(axis));
        }

        updateCameraComponents();
    }

    bool Camera::onKeyPress(unsigned int) {
        //do nothing
        return true;
    }

    bool Camera::onKeyRelease(unsigned int) {
        //do nothing
        return true;
    }

    bool Camera::onMouseMove(double mouseX, double mouseY) {
        if (bUseMouse) {
            if (previousMouseX == std::numeric_limits<double>::max() && previousMouseY == std::numeric_limits<double>::max()) {
                previousMouseX = mouseX;
                previousMouseY = mouseY;
                return false;
            } else if (mouseX == previousMouseX && mouseY == previousMouseY) {
                return false;
            }

            Vector2<float> mouseDirection;
            mouseDirection.X = (float)(previousMouseX - mouseX) * MOUSE_SENSITIVITY_FACTOR * mouseSensitivityPercentage;
            mouseDirection.Y = (float)(previousMouseY - mouseY) * MOUSE_SENSITIVITY_FACTOR * mouseSensitivityPercentage;
            if (invertYAxis) {
                mouseDirection.Y = - mouseDirection.Y;
            }

            //do not rotate up/down more than "maxRotationX" percent
            float currentRotationX = view.Y + mouseDirection.Y;
            if (currentRotationX > 0.0f && currentRotationX > maxRotationX) {
                mouseDirection.Y -= (currentRotationX - maxRotationX);
            } else if (currentRotationX < 0.0f && currentRotationX < -maxRotationX) {
                mouseDirection.Y -= (currentRotationX + maxRotationX);
            }

            //rotate around the Y and X axis
            rotate(Quaternion<float>(up.crossProduct(view), -mouseDirection.Y));
            rotate(Quaternion<float>(Vector3<float>(0.0f, 1.0f, 0.0f), mouseDirection.X));

            updateCameraComponents();

            previousMouseX = mouseX;
            previousMouseY = mouseY;

            return false;
        }
        return true;
    }

    void Camera::updateCameraComponents() {
        const Vector3<float>& viewUp = view.crossProduct(up).normalize();
        Matrix4<float> rotation(
                viewUp[0], viewUp[1], viewUp[2], 0.0f,
                up[0], up[1], up[2], 0.0f,
                -view[0], -view[1], -view[2], 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
        Matrix4<float> translation(
                1.0f, 0.0f, 0.0f, -position.X,
                0.0f, 1.0f, 0.0f, -position.Y,
                0.0f, 0.0f, 1.0f, -position.Z,
                0.0f, 0.0f, 0.0f, 1.0f);
        mView = rotation * translation;

        orientation = Quaternion<float>(mView.toMatrix3().inverse());

        frustum = baseFrustum * mView.inverse();
    }

}

#pragma once

#include "Vulkan/VulkanInclude.hpp"
#include "Base/Node.hpp"
#include "Math/EngineMath.hpp"
#include "Renderer.hpp"

namespace mge {
    class Camera : public Node {
    public:
        Vector3 position{0.f, 0.f, 0.f};
        Quaternion rotation{};

        float32_t fov{60.f};
        Color8 clearColor{10, 10, 10, 255};
        float32_t nearPlane{.01f};
        float32_t farPlane{1000.f};

        Camera() = default;
        Camera(const Camera&) = delete;
        Camera(Camera&&) noexcept = delete;

        Camera& operator=(const Camera&) = delete;
        Camera& operator=(Camera&&) noexcept = delete;

        void Render() override;

        ~Camera() = default;
    private:
        void AddRenderersToVector(vector<Renderer*>& renderers, Node* parent) const;
    };
}
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <unordered_set>
#include <iostream>
#include "oglw.h"
#include "liblas/liblas.hpp"

#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val > max) ? (max) : (val)))

template <class T>
using uptr = std::unique_ptr<T>;
using namespace OGLW;

struct VoxelVert {
    glm::vec3 position;
    glm::vec3 color;
};

static std::unique_ptr<Mesh<VoxelVert>> voxels(std::vector<VoxelVert>& _vertices) {
    static auto layout = std::shared_ptr<VertexLayout>(new VertexLayout({
        {"position", 3, GL_FLOAT, false, 0, AttributeLocation::position},
        {"color", 3, GL_FLOAT, false, 0, AttributeLocation::color},
    }));

    auto mesh = std::make_unique<Mesh<VoxelVert>>(layout, GL_POINTS);

    mesh->addVertices(std::move(_vertices), {});
    return std::move(mesh);
}

// ------------------------------------------------------------------------------
// OGLW App
class VoxelLidar : public OGLW::App {
    public:
        VoxelLidar() : OGLW::App("OGLW::VoxelLidar", 800, 600) {}
        void update(float _dt) override;
        void render(float _dt) override;
        void init() override;

    private:
        uptr<Shader> m_shader;
        uptr<Mesh<VoxelVert>> m_geometry;

};
OGLWMain(VoxelLidar);

void VoxelLidar::init() {
    int posx = 6830;
    int posy = 2475;

    // camera setup
    m_camera.setPosition({190.0, -490.0, 430.0});

    m_camera.setNear(2.0);
    m_camera.setFar(1000.0);
    m_camera.setFov(55);

    // load shader
    m_shader = uptr<Shader>(new Shader("voxel.glsl"));

    // read LIDAR data
    std::ifstream ifs;
    ifs.open(std::to_string(posx) + "_" + std::to_string(posy) + ".laz", std::ios::in | std::ios::binary);
    liblas::ReaderFactory f;
    liblas::Reader reader = f.CreateWithStream(ifs);
    liblas::Header const& header = reader.GetHeader();

    std::cout << "Signature: " << header.GetFileSignature() << '\n';
    std::cout << "Points count: " << header.GetPointRecordsCount() << '\n';

    struct set_comparator {
        size_t operator()(const glm::vec3& p) const {
            return std::hash<int>()(p.x) ^ std::hash<int>()(p.y) ^ std::hash<int>()(p.z);
        }
    };
    std::unordered_set<glm::vec3, set_comparator> positions;

    int maxHeight = -INT_MAX;
    while (reader.ReadNextPoint()) {
        liblas::Point const& p = reader.GetPoint();

        int x = (int)floor(p.GetX() - (posx * 100));
        int y = (int)floor(p.GetY() - (posy * 100));
        int z = (int)floor(p.GetZ());

        maxHeight = std::max(z, maxHeight);
        positions.insert({x, -z, y});
    }

    std::vector<VoxelVert> vertices;

    for (const auto& p : positions) {
        // TODO: better coloring
        glm::vec3 color = glm::vec3(CLAMP((std::abs(p.y) - 400) / 30, 0.0, 1.0));
        vertices.push_back({p, color});
    }

    std::cout << "Voxel number: " << vertices.size() << std::endl;

    m_geometry = voxels(vertices);
}

void VoxelLidar::update(float _dt) {
    updateFreeFlyCamera(_dt, 'S', 'W', 'A', 'D', 1e-3f, 55.f);
}

void VoxelLidar::render(float _dt) {
    glm::vec3 lightPos = glm::vec3(190.f, -490.f, 430.f);
    glm::mat4 mvp = m_camera.getProjectionMatrix() * m_camera.getViewMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(mvp)));

    RenderState::culling(GL_FALSE);
    RenderState::depthTest(GL_TRUE);
    RenderState::depthWrite(GL_TRUE);

    m_shader->setUniform("mvp", mvp);
    //m_shader->setUniform("view", m_camera.getViewMatrix());
    m_shader->setUniform("normalMatrix", normalMatrix);
    //m_shader->setUniform("viewPos", m_camera.getPosition());

    //m_shader->setUniform("light.position", lightPos);
    //m_shader->setUniform("light.color", glm::vec3(0.6, 0.68, 0.68));
    //m_shader->setUniform("light.ambiant", glm::vec3(0.7));
    //m_shader->setUniform("light.diffuseIntensity", 0.5f);

    m_geometry->draw(*m_shader);

}


#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include "oglw.h"
#include "liblas/liblas.hpp"

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
class TestApp : public OGLW::App {
    public:
        TestApp() : OGLW::App("OGLW::VoxelLidar", 800, 600) {}
        void update(float _dt) override;
        void render(float _dt) override;
        void init() override;

    private:
        uptr<Shader> m_shader;
        uptr<Mesh<VoxelVert>> m_geometry;

};
OGLWMain(TestApp);

void TestApp::init() {
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

    std::vector<VoxelVert> vertices;

    while (reader.ReadNextPoint()) {
        liblas::Point const& p = reader.GetPoint();

        int x = (int)floor(p.GetX() - (posx * 100));
        int y = (int)floor(p.GetY() - (posy * 100));
        int z = (int)floor(p.GetZ());

        vertices.push_back({{x, -z, y}, glm::vec3(z)});
    }

    // TODO: remove duplicates

    m_geometry = voxels(vertices);
}

void TestApp::update(float _dt) {
    updateFreeFlyCamera(_dt, 'S', 'W', 'A', 'D', 1e-3f, 55.f);
}

void TestApp::render(float _dt) {
    //glm::vec3 lightPos = glm::vec3(0.f, 0.f, 0.f);
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
    //m_shader->setUniform("light.specularIntensity", 5.0f);

    m_geometry->draw(*m_shader);

}


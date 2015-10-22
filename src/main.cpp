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

// ------------------------------------------------------------------------------
// OGLW App
class TestApp : public OGLW::App {
    public:
        TestApp() : OGLW::App("OGLW::TestApp", 800, 600) {}
        void update(float _dt) override;
        void render(float _dt) override;
        void init() override;

};
OGLWMain(TestApp);

void TestApp::init() {
    std::ifstream ifs;
    ifs.open("6830_2475.laz", std::ios::in | std::ios::binary);
    liblas::ReaderFactory f;
    liblas::Reader reader = f.CreateWithStream(ifs);
    liblas::Header const& header = reader.GetHeader();

    std::cout << "Compressed: " << (header.Compressed() == true) ? "true":"false";
    std::cout << "Signature: " << header.GetFileSignature() << '\n';
    std::cout << "Points count: " << header.GetPointRecordsCount() << '\n';

    while (reader.ReadNextPoint()) {
        liblas::Point const& p = reader.GetPoint();
        std::cout << p.GetX() << ", " << p.GetY() << ", " << p.GetZ() << "\n";
    }
}

void TestApp::update(float _dt) {}

void TestApp::render(float _dt) {}


#include "cineforge.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {
void require(bool ok, const char* message) {
    if (!ok) { std::cerr << "FAILED: " << message << '\n'; std::exit(EXIT_FAILURE); }
}
}
int main() {
    ventu::cineforge::Settings s;
    require(ventu::cineforge::validate(s).empty(), "default CINEFORGE settings validate");
    auto bad=s; bad.packSize=7;
    require(!ventu::cineforge::validate(bad).empty(), "pack size below eight is rejected");
    const auto dir=std::filesystem::temp_directory_path()/"cineforge-mvp-test";
    std::error_code ec; std::filesystem::remove_all(dir,ec);
    s.sampleRate=48000; s.packSize=8; s.variations=2;
    const auto result=ventu::cineforge::generate(s,dir);
    require(static_cast<bool>(result), "cohesive pack generation succeeds");
    require(result.assets.size()==8, "eight linked assets generated");
    require(std::filesystem::exists(result.previewPath), "combined preview generated");
    require(std::filesystem::exists(result.manifestPath), "manifest generated");
    std::ifstream wav(result.assets.front().dryPath,std::ios::binary);
    std::array<unsigned char,44> header{}; wav.read(reinterpret_cast<char*>(header.data()),44);
    require(header[34]==24 && header[35]==0, "WAV export is 24-bit PCM");
    require(header[24]==0x80 && header[25]==0xbb, "WAV sample rate is 48000 Hz");
    std::ifstream json(result.manifestPath,std::ios::binary);
    const std::string text((std::istreambuf_iterator<char>(json)),{});
    require(text.find("cineforge.pack.v1")!=std::string::npos, "CINEFORGE manifest schema present");
    require(text.find("SubBoom")!=std::string::npos, "manifest records generated family assets");
    std::filesystem::remove_all(dir,ec);
    std::cout << "CINEFORGE MVP tests passed\n";
    return 0;
}

#include "RaZ/Render/Framebuffer.hpp"
#include "RaZ/Render/GraphicObjects.hpp"
#include "RaZ/Render/Renderer.hpp"
#include "RaZ/Render/ShaderProgram.hpp"
#include "RaZ/Render/Texture.hpp"
#include "RaZ/Utils/Logger.hpp"

#include "tracy/Tracy.hpp"
#include "GL/glew.h" // Needed by TracyOpenGL.hpp
#include "tracy/TracyOpenGL.hpp"

#include <ranges>

namespace Raz {

Framebuffer::Framebuffer() {
  ZoneScopedN("Framebuffer::Framebuffer");

  Logger::debug("[Framebuffer] Creating...");
  Renderer::generateFramebuffer(m_index);
  Logger::debug("[Framebuffer] Created (ID: {})", m_index.get());
}

VertexShader Framebuffer::recoverVertexShader() {
  // Creating a triangle large enough to cover the whole render frame:
  //
  //   3 | \                                3 | \
  //     |    \                               |  \
  //   2 |       \                          2 |    \
  //     |          \                         |     \
  //   1 ------------- \                    1 -------\
  //     |           |    \                   |     | \
  //   0 |           |       \              0 |     |   \
  //     |           |          \             |     |    \
  //  -1 -------------------------         -1 -------------
  //    -1     0     1     2     3           -1  0  1  2  3

  static constexpr std::string_view vertSource = R"(
    const vec2 positions[3] = vec2[](
      vec2(-1.0, -1.0),
      vec2( 3.0, -1.0),
      vec2(-1.0,  3.0)
    );

    const vec2 texcoords[3] = vec2[](
      vec2(0.0, 0.0),
      vec2(2.0, 0.0),
      vec2(0.0, 2.0)
    );

    out vec2 fragTexcoords;

    void main() {
      fragTexcoords = texcoords[gl_VertexID];
      gl_Position   = vec4(positions[gl_VertexID], 0.0, 1.0);
    }
  )";

  return VertexShader::loadFromSource(vertSource);
}

void Framebuffer::setDepthBuffer(Texture2DPtr texture) {
  if (texture->getColorspace() != TextureColorspace::DEPTH)
    throw std::invalid_argument("Error: Invalid depth buffer");

  m_depthBuffer = std::move(texture);

  mapBuffers();
}

void Framebuffer::addColorBuffer(Texture2DPtr texture, unsigned int index) {
  if (texture->getColorspace() == TextureColorspace::DEPTH || texture->getColorspace() == TextureColorspace::INVALID)
    throw std::invalid_argument("Error: Invalid color buffer");

  const auto bufferIt = std::ranges::find_if(m_colorBuffers, [&texture, index] (const auto& colorBuffer) noexcept {
    return (texture == colorBuffer.first && index == colorBuffer.second);
  });

  // Adding the color buffer only if it doesn't exist yet
  if (bufferIt == m_colorBuffers.cend())
    m_colorBuffers.emplace_back(std::move(texture), index);

  mapBuffers();
}

void Framebuffer::removeTextureBuffer(const Texture2DPtr& texture) {
  if (texture == m_depthBuffer) {
    m_depthBuffer.reset();
  } else {
    std::erase_if(m_colorBuffers, [&texture] (const auto& buffer) noexcept {
      return (texture == buffer.first);
    });
  }

  mapBuffers();
}

void Framebuffer::clearTextureBuffers() {
  clearDepthBuffer();
  clearColorBuffers();
}

void Framebuffer::resizeBuffers(unsigned int width, unsigned int height) {
  ZoneScopedN("Framebuffer::resizeBuffers");

  if (m_depthBuffer)
    m_depthBuffer->resize(width, height);

  for (const Texture2DPtr& colorBuffer : m_colorBuffers | std::views::keys)
    colorBuffer->resize(width, height);
}

void Framebuffer::mapBuffers() const {
  ZoneScopedN("Framebuffer::mapBuffers");

  Logger::debug("[Framebuffer] Mapping buffers (ID: {})...", m_index.get());

  Renderer::bindFramebuffer(m_index);

  if (m_depthBuffer) {
    Logger::debug("[Framebuffer] Mapping depth buffer...");
    Renderer::setFramebufferTexture2D(FramebufferAttachment::DEPTH, m_depthBuffer->getIndex(), 0, TextureType::TEXTURE_2D);
  }

  if (!m_colorBuffers.empty()) {
    std::vector<DrawBuffer> drawBuffers(m_colorBuffers.size(), DrawBuffer::NONE);

    for (const auto& [colorBuffer, bufferIndex] : m_colorBuffers) {
      Logger::debug("[Framebuffer] Mapping color buffer {}...", bufferIndex);

      const std::size_t colorAttachment = static_cast<unsigned int>(DrawBuffer::COLOR_ATTACHMENT0) + bufferIndex;

      Renderer::setFramebufferTexture2D(static_cast<FramebufferAttachment>(colorAttachment), colorBuffer->getIndex(), 0, TextureType::TEXTURE_2D);

      if (bufferIndex >= drawBuffers.size())
        drawBuffers.resize(bufferIndex + 1, DrawBuffer::NONE);
      drawBuffers[bufferIndex] = static_cast<DrawBuffer>(colorAttachment);
    }

    Renderer::setDrawBuffers(static_cast<unsigned int>(drawBuffers.size()), drawBuffers.data());
  }

  unbind();

  Logger::debug("[Framebuffer] Mapped buffers");
}

void Framebuffer::bind() const {
  Renderer::bindFramebuffer(m_index);
  Renderer::clear(MaskType::COLOR | MaskType::DEPTH | MaskType::STENCIL);
}

void Framebuffer::unbind() const {
  Renderer::unbindFramebuffer();
}

void Framebuffer::display() const {
  ZoneScopedN("Framebuffer::display");
  TracyGpuZone("Framebuffer::display")

  static const VertexArray vao;
  vao.bind();

  Renderer::clear(MaskType::COLOR);
  Renderer::drawArrays(PrimitiveType::TRIANGLES, 3);
}

Framebuffer::~Framebuffer() {
  ZoneScopedN("Framebuffer::~Framebuffer");

  if (!m_index.isValid())
    return;

  Logger::debug("[Framebuffer] Destroying (ID: {})...", m_index.get());
  Renderer::deleteFramebuffer(m_index);
  Logger::debug("[Framebuffer] Destroyed");
}

} // namespace Raz

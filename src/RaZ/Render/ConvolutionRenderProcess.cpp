#include "RaZ/Render/ConvolutionRenderProcess.hpp"
#include "RaZ/Render/RenderPass.hpp"

#include <string_view>

namespace Raz {

namespace {

constexpr std::string_view convolutionSource = {
#include "convolution.frag.embed"
};

} // namespace

ConvolutionRenderProcess::ConvolutionRenderProcess(RenderGraph& renderGraph, const Mat3f& kernel, std::string passName)
  : MonoPassRenderProcess(renderGraph, FragmentShader::loadFromSource(convolutionSource), std::move(passName)) { setKernel(kernel); }

void ConvolutionRenderProcess::setInputBuffer(Texture2DPtr colorBuffer) {
  const Vec2f invBufferSize(1.f / static_cast<float>(colorBuffer->getWidth()), 1.f / static_cast<float>(colorBuffer->getHeight()));
  m_pass.getProgram().setAttribute(invBufferSize, "uniInvBufferSize");
  m_pass.getProgram().sendAttributes();

  MonoPassRenderProcess::setInputBuffer(std::move(colorBuffer), "uniBuffer");
}

void ConvolutionRenderProcess::setOutputBuffer(Texture2DPtr colorBuffer) {
  MonoPassRenderProcess::setOutputBuffer(std::move(colorBuffer), 0);
}

void ConvolutionRenderProcess::setKernel(const Mat3f& kernel) const {
  m_pass.getProgram().setAttribute(std::vector<float>(kernel.getDataPtr(), kernel.getDataPtr() + 9), "uniKernel");
  m_pass.getProgram().sendAttributes();
}

} // namespace Raz
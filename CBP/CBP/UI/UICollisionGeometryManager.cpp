#include "pch.h"

#include "UICollisionGeometryManager.h"

#include "CBP/ColliderData.h"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"
#include "Drivers/gui.h"
#include "drivers/render.h"

namespace CBP
{

    using namespace UICommon;

    UICollisionGeometryManager::UICollisionGeometryManager(
        UIContext& a_parent, const char* a_name)
        :
        UICommon::UIProfileEditorBase<ColliderProfile>(a_name),
        m_parent(a_parent),
        m_zoom(DEFAULT_ZOOM),
        m_dragRotate(0, [this](const ImVec2& a_delta) { m_model->Rotate(a_delta); }),
        m_dragPan(1, [this](const ImVec2& a_delta) { m_model->Pan(a_delta); }),
        m_resList{
            {"512", 512.0f},
            {"1024", 1024.0f},
            {"2048", 2048.0f},
            {"4096", 4096.0f}
    }
    {
    }

    void UICollisionGeometryManager::DrawModel()
    {
        if (m_model.get()) {
            m_model->Draw();
        }
    }

    void UICollisionGeometryManager::OnOpen()
    {
        QueueModelReload();
    }

    void UICollisionGeometryManager::OnClose()
    {
        QueueModelRelease();
    }

    ProfileManager<ColliderProfile>& UICollisionGeometryManager::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<ColliderProfile>();
    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::Initialize(
        const ColliderData* a_data,
        ID3D11Device* a_device,
        ID3D11DeviceContext* a_context)
    {
        LoadGeometry(a_data, a_device);

        m_effect = std::make_unique<DirectX::BasicEffect>(a_device);

        m_effect->SetTextureEnabled(false);

        ApplyEffectSettings(m_effect.get());

        /*void const* shaderByteCode;
        size_t byteCodeLength;

        m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DirectX::ThrowIfFailed(a_device->CreateInputLayout(
            vertexType_t::InputElements, vertexType_t::InputElementCount,
            shaderByteCode, byteCodeLength, m_inputLayout.ReleaseAndGetAddressOf()));*/

        DirectX::ThrowIfFailed(
            DirectX::CreateInputLayoutFromEffect<vertexType_t>(
                a_device, m_effect.get(), m_inputLayout.ReleaseAndGetAddressOf()));

        m_context = a_context;

    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::LoadGeometry(
        const ColliderData* a_data,
        ID3D11Device* a_device)
    {
        vertexVector_t vertices;
        indexVector_t indices;

        auto numVertices = static_cast<std::size_t>(a_data->m_numVertices);
        auto numIndices = static_cast<std::size_t>(a_data->m_numIndices);

        vertices.resize(numVertices);
        indices.resize(numIndices);

        CreateGeometry(a_data, vertices, indices);

        auto vsize = static_cast<UINT>(vertices.size() * sizeof(vertexVector_t::value_type));
        auto isize = static_cast<UINT>(indices.size() * sizeof(indexVector_t::value_type));

        CD3D11_BUFFER_DESC vdesc(vsize, D3D11_BIND_VERTEX_BUFFER);
        CD3D11_BUFFER_DESC idesc(isize, D3D11_BIND_INDEX_BUFFER);

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

        D3D11_SUBRESOURCE_DATA initData = { vertices.data(), vsize, 0 };
        DirectX::ThrowIfFailed(a_device->CreateBuffer(&vdesc, &initData,
            vertexBuffer.ReleaseAndGetAddressOf()));

        initData = { indices.data(), isize, 0 };
        DirectX::ThrowIfFailed(a_device->CreateBuffer(&idesc, &initData,
            indexBuffer.ReleaseAndGetAddressOf()));

        m_indexCount = static_cast<UINT>(numIndices);

        m_numVertices = numVertices;
        m_numIndices = numIndices;

        m_vertexBuffer = vertexBuffer;
        m_indexBuffer = indexBuffer;

    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::ApplyEffectSettings(
        DirectX::BasicEffect* a_effect) const
    {
    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::Draw(
        setStateFunc_t a_setCustomState) const
    {
        m_effect->Apply(m_context.Get());
        m_context->IASetInputLayout(m_inputLayout.Get());

        auto vb = m_vertexBuffer.Get();
        UINT vertexStride = sizeof(vertexType_t);
        UINT vertexOffset = 0;

        m_context->IASetVertexBuffers(0, 1, &vb, &vertexStride, &vertexOffset);
        m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        a_setCustomState();

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_context->DrawIndexed(m_indexCount, 0, 0);

    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::SetLightColor(
        Light a_which,
        int a_index,
        DirectX::XMVECTOR a_color)
    {
        switch (a_which)
        {
        case Light::kAmbient:
            m_effect->SetAmbientLightColor(a_color);
            break;
        case Light::kDiffuse:
            m_effect->SetLightDiffuseColor(a_index, a_color);
            break;
        case Light::kSpecular:
            m_effect->SetLightSpecularColor(a_index, a_color);
            break;
        }
    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::SetMaterialColor(
        DirectX::XMVECTOR a_color)
    {
        m_effect->SetColorAndAlpha(a_color);
    }

    void UICollisionGeometryManager::ShapePosition::CreateGeometry(
        const ColliderData* a_data,
        vertexVector_t& a_vertices,
        indexVector_t& a_indices) const
    {
        auto numVertices = static_cast<std::size_t>(a_data->m_numVertices);
        auto numIndices = static_cast<std::size_t>(a_data->m_numIndices);

        for (std::size_t i = 0; i < numVertices; i++)
        {
            auto& vertex = a_data->m_vertices[i];
            a_vertices[i].position =
                DirectX::XMFLOAT3(vertex.v.x(), vertex.v.y(), vertex.v.z());
        }

        for (std::size_t i = 0; i < numIndices; i++)
        {
            auto index = a_data->m_indices[i];

            a_indices[i] = static_cast<std::uint32_t>(index);
        }

    }

    void UICollisionGeometryManager::ShapeNormal::ApplyEffectSettings(
        DirectX::BasicEffect* a_effect) const
    {
        a_effect->EnableDefaultLighting();
    }

    void UICollisionGeometryManager::ShapeNormal::CreateGeometry(
        const ColliderData* a_data,
        vertexVector_t& a_vertices,
        indexVector_t& a_indices) const
    {
        auto numVertices = static_cast<std::size_t>(a_data->m_numVertices);
        auto numIndices = static_cast<std::size_t>(a_data->m_numIndices);

        stl::vector<DirectX::XMVECTOR> normals;

        normals.resize(numVertices);

        for (std::size_t i = 0; i < numVertices; i++)
        {
            normals[i] = DirectX::g_XMZero;
        }

        for (std::size_t i = 0; i < numIndices; i += 3)
        {
            auto i1 = a_data->m_indices[i];
            auto i2 = a_data->m_indices[i + 1];
            auto i3 = a_data->m_indices[i + 2];

            a_indices[i] = static_cast<std::uint32_t>(i1);
            a_indices[i + 1] = static_cast<std::uint32_t>(i2);
            a_indices[i + 2] = static_cast<std::uint32_t>(i3);

            auto& v1 = a_data->m_vertices[i1];
            auto& v2 = a_data->m_vertices[i2];
            auto& v3 = a_data->m_vertices[i3];

            auto p1 = (v2.v - v1.v);
            auto p2 = (v3.v - v1.v);

            auto p = p1.cross(p2);

            normals[i1] = DirectX::XMVectorAdd(normals[i1], p.mVec128);
            normals[i2] = DirectX::XMVectorAdd(normals[i2], p.mVec128);
            normals[i3] = DirectX::XMVectorAdd(normals[i3], p.mVec128);
        }

        for (std::size_t i = 0; i < numVertices; i++)
        {
            auto& f = a_vertices[i];

            DirectX::XMStoreFloat3(&f.position, a_data->m_vertices[i].v.mVec128);
            DirectX::XMStoreFloat3(&f.normal, DirectX::XMVector3Normalize(normals[i]));

        }

    }

    void UICollisionGeometryManager::ShapePosition::ApplyEffectSettings(
        DirectX::BasicEffect* a_effect) const
    {
    }

    UICollisionGeometryManager::Model::Model(
        const ColliderData* a_data,
        ID3D11Device* a_device,
        ID3D11DeviceContext* a_context,
        const DirectX::XMFLOAT3& a_bufferSize,
        float a_textureResolution)
        :
        m_device(a_device),
        m_context(a_context),
        m_bufferSize(a_bufferSize),
        m_textureResolution(a_textureResolution),
        m_eyePos(DirectX::SimpleMath::Vector3(0.0f, DEFAULT_ZOOM, 0.0f)),
        m_targetPos(DirectX::SimpleMath::Vector3::Zero),
        m_world(DirectX::SimpleMath::Matrix::Identity)
    {
        m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
            DirectX::XMConvertToRadians(DEFAULT_FOV), a_bufferSize.z, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);

        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
            m_eyePos,
            m_targetPos,
            DirectX::SimpleMath::Vector3::UnitZ);

        m_states = std::make_unique<DirectX::CommonStates>(a_device);

        m_shape = std::make_unique<ShapeNormal>();
        m_shape->Initialize(a_data, a_device, a_context);

        m_shape->GetEffect()->SetMatrices(m_world, m_view, m_proj);

        CD3D11_TEXTURE2D_DESC depthBufferDesc(
            DXGI_FORMAT_D24_UNORM_S8_UINT,
            a_textureResolution,
            a_textureResolution,
            1,
            1,
            D3D11_BIND_DEPTH_STENCIL,
            D3D11_USAGE_DEFAULT,
            0,
            1,
            0,
            0
        );

        DirectX::ThrowIfFailed(a_device->CreateTexture2D(&depthBufferDesc, nullptr, m_depthStencilBuffer.ReleaseAndGetAddressOf()));

        CD3D11_TEXTURE2D_DESC textureDesc(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            a_textureResolution,
            a_textureResolution,
            1,
            1,
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
            D3D11_USAGE_DEFAULT,
            0,
            1,
            0,
            0
        );

        DirectX::ThrowIfFailed(a_device->CreateTexture2D(&textureDesc, nullptr, m_renderTargetViewTexture.ReleaseAndGetAddressOf()));

        CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
            D3D11_RTV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0
        );

        DirectX::ThrowIfFailed(a_device->CreateRenderTargetView(m_renderTargetViewTexture.Get(), &renderTargetViewDesc, m_renderTargetView.ReleaseAndGetAddressOf()));

        CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
            D3D11_SRV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0,
            1
        );

        DirectX::ThrowIfFailed(a_device->CreateShaderResourceView(m_renderTargetViewTexture.Get(), &shaderResourceViewDesc, m_shaderResourceView.ReleaseAndGetAddressOf()));

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
            D3D11_DSV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_D24_UNORM_S8_UINT
        );

        DirectX::ThrowIfFailed(a_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    }

    void UICollisionGeometryManager::Model::Draw() const
    {
        D3D11StateBackup _(std::addressof(m_backup), m_context.Get(), true);

        CD3D11_VIEWPORT viewPort(0.0f, 0.0f, m_textureResolution, m_textureResolution);
        m_context->RSSetViewports(1, &viewPort);

        ID3D11RenderTargetView* rtViews[]{ m_renderTargetView.Get() };
        m_context->OMSetRenderTargets(1, rtViews, m_depthStencilView.Get());

        const auto& globalConfig = IConfig::GetGlobal();

        m_shape->Draw([this]
            {
                m_context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
                m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

                const auto& globalConfig = IConfig::GetGlobal();

                m_context->RSSetState(
                    globalConfig.ui.geometry.wireframe ?
                    m_states->Wireframe() :
                    m_states->CullNone());

                /*ID3D11SamplerState* samplerState = m_states->AnisotropicWrap();
                m_context->PSSetSamplers(0, 1, &samplerState);*/

                auto& wcol = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
                const float col[4]{ wcol.x, wcol.y, wcol.z, 0.0f };

                m_context->ClearRenderTargetView(m_renderTargetView.Get(), col);
                m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            });
    }

    void UICollisionGeometryManager::Model::GetMatrices(
        std::unique_ptr<matrices_t>& a_out) const
    {
        if (!a_out.get()) {
            a_out = std::make_unique<matrices_t>(m_world, m_eyePos, m_targetPos, m_proj);
        }
        else {
            a_out->m_world = m_world;
            a_out->m_proj = m_proj;
            a_out->m_eyePos = m_eyePos;
            a_out->m_targetPos = m_targetPos;
        }
    }

    void UICollisionGeometryManager::Model::SetMatrices(
        const std::unique_ptr<matrices_t>& a_in)
    {
        if (!a_in.get())
            return;

        m_world = a_in->m_world;
        m_eyePos = a_in->m_eyePos;
        m_targetPos = a_in->m_targetPos;
        m_proj = a_in->m_proj;

        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
            m_eyePos,
            m_targetPos,
            DirectX::SimpleMath::Vector3::UnitZ);

        m_shape->GetEffect()->SetMatrices(m_world, m_view, m_proj);
    }

    void UICollisionGeometryManager::Model::ResetPos()
    {
        m_eyePos = DirectX::SimpleMath::Vector3(0.0f, DEFAULT_ZOOM, 0.0f);
        m_targetPos = DirectX::SimpleMath::Vector3::Zero;
        m_world = DirectX::SimpleMath::Matrix::Identity;

        UpdateViewMatrix();
        m_shape->GetEffect()->SetWorld(m_world);
    }

    void UICollisionGeometryManager::Model::SetZoom(float a_val)
    {
        m_eyePos.y = a_val;

        UpdateViewMatrix();
    }

    void UICollisionGeometryManager::Model::SetFOV(float a_val)
    {
        m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
            DirectX::XMConvertToRadians(a_val),
            m_bufferSize.z,
            DEFAULT_NEAR_PLANE,
            DEFAULT_FAR_PLANE);

        m_shape->GetEffect()->SetProjection(m_proj);
    }

    void UICollisionGeometryManager::Model::Pan(const ImVec2& a_delta)
    {
        m_eyePos.x += a_delta.x * 0.1f;
        m_eyePos.z += a_delta.y * 0.1f;

        m_targetPos.x = m_eyePos.x;
        m_targetPos.z = m_eyePos.z;

        UpdateViewMatrix();
    }

    void UICollisionGeometryManager::Model::Rotate(const ImVec2& a_delta)
    {
        using namespace DirectX::SimpleMath;

        auto rot = Quaternion::CreateFromYawPitchRoll(
            0.0f,
            -DirectX::XMConvertToRadians(a_delta.y),
            DirectX::XMConvertToRadians(a_delta.x));

        m_world = Matrix::Transform(m_world, rot);

        m_shape->GetEffect()->SetWorld(m_world);
    }

    void UICollisionGeometryManager::Model::UpdateViewMatrix()
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
            m_eyePos,
            m_targetPos,
            DirectX::SimpleMath::Vector3::UnitZ);

        m_shape->GetEffect()->SetView(m_view);
    }

    UICollisionGeometryManager::DragController::DragController(
        std::uint8_t a_button,
        func_t a_onDrag)
        :
        m_dragging(false),
        m_button(a_button),
        m_func(std::move(a_onDrag))
    {
        ASSERT(m_button < std::size(ImGui::GetIO().MouseDown));
    }

    void UICollisionGeometryManager::DragController::Update(bool a_isHovered)
    {
        auto& io = ImGui::GetIO();

        if (a_isHovered)
        {
            if (!m_dragging)
            {
                if (io.MouseDown[m_button])
                {
                    m_dragging = true;
                    m_lastMousePos = io.MousePos;
                    m_avgMouseDir = ImVec2(0.0f, 0.0f);
                }
            }
        }

        if (m_dragging)
        {
            if (!io.MouseDown[m_button])
            {
                m_dragging = false;
            }
            else
            {
                if (io.MousePos.x != m_lastMousePos.x ||
                    io.MousePos.y != m_lastMousePos.y)
                {
                    ImVec2 dir;

                    dir.x = io.MousePos.x - m_lastMousePos.x;
                    dir.y = io.MousePos.y - m_lastMousePos.y;

                    m_lastMousePos = io.MousePos;

                    if (io.KeyShift)
                    {
                        m_avgMouseDir.x = m_avgMouseDir.x * 0.75f + dir.x * 0.25f;
                        m_avgMouseDir.y = m_avgMouseDir.y * 0.75f + dir.y * 0.25f;

                        if (std::fabsf(m_avgMouseDir.x) > std::fabsf(m_avgMouseDir.y))
                        {
                            dir.y = 0.0f;
                        }
                        else if (m_avgMouseDir.x == m_avgMouseDir.y) {
                            return;
                        }
                        else {
                            dir.x = 0.0f;
                        }
                    }

                    m_func(dir);

                }
            }
        }
    }

    UICollisionGeometryManager::ReleaseModelTask::ReleaseModelTask(
        std::unique_ptr<Model>&& a_model)
        :
        m_ref(std::move(a_model))
    {
    }

    WindowLayoutData UICollisionGeometryManager::GetWindowDimensions() const
    {
        return WindowLayoutData(0.0f, 1000.0f, 667.0f, true);
    }

    void UICollisionGeometryManager::DrawCallback(
        const ImDrawList* parent_list,
        const ImDrawCmd* cmd)
    {
        auto gm = static_cast<UICollisionGeometryManager::Model*>(cmd->UserCallbackData);
        gm->Draw();
    }

    void UICollisionGeometryManager::DrawItem(ColliderProfile& a_profile)
    {
        ImGui::Spacing();

        if (ImGui::BeginChild("__render_area", ImVec2(0, 0), true))
        {
            if (m_state.selected)
            {
                if (m_model.get())
                {
                    auto& io = ImGui::GetIO();

                    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

                    if (hovered)
                    {
                        if (io.MouseWheel != 0.0f) {
                            m_zoom = std::max(m_zoom - io.MouseWheel * 0.2f, 0.1f);
                            m_model->SetZoom(m_zoom);
                        }
                    }

                    m_dragRotate.Update(hovered);
                    m_dragPan.Update(hovered);

                    auto pos = ImGui::GetWindowPos();
                    auto windowSize = ImGui::GetWindowSize();
                    auto& bufferSize = m_model->GetBufferSize();

                    pos.x += (windowSize.x - bufferSize.x) / 2.0f;
                    pos.y += (windowSize.y - bufferSize.y) / 2.0f;

                    ImVec2 bottom(pos.x + bufferSize.x, pos.y + bufferSize.y);

                    auto drawList = ImGui::GetWindowDrawList();

                    m_model->Draw();
                    drawList->AddImage(static_cast<void*>(m_model->GetTexture()), pos, bottom);
                }
            }
            else
            {
                if (m_model.get()) {
                    QueueModelRelease();
                }
            }
        }

        ImGui::EndChild();

    }

    void UICollisionGeometryManager::DrawOptions(ColliderProfile& a_profile)
    {
        auto& globalConfig = IConfig::GetGlobal();

        ImGui::Spacing();

        ImGui::PushItemWidth(125.0f);
        DrawResolutionCombo();
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        ImGui::SameLine();
        Checkbox("Wireframe", &globalConfig.ui.geometry.wireframe);

        bool hasModel = (m_model.get() != nullptr);

        ImGui::SameLine();
        if (Checkbox("Lighting", &globalConfig.ui.geometry.lighting)) {
            if (hasModel) {
                m_model->EnableLighting(globalConfig.ui.geometry.lighting);
            }
        }

        auto width = ImGui::GetWindowContentRegionMax().x;

        if (hasModel)
        {
            ImGui::SameLine(width - ImGui::CalcTextSize(m_infoStrings.m_vertices.c_str()).x - 5.0f);
            ImGui::TextWrapped(m_infoStrings.m_vertices.c_str());
        }

        ImGui::PushItemWidth(250.0f);

        if (ColorEdit4("Model", globalConfig.ui.geometry.color.m128_f32, ImGuiColorEditFlags_NoInputs))
        {
            if (hasModel) {
                m_model->SetMaterialColor(globalConfig.ui.geometry.color);
            }
        }

        ImGui::SameLine();
        if (ColorEdit4("Ambient", globalConfig.ui.geometry.ambientLightColor.m128_f32, ImGuiColorEditFlags_NoInputs))
        {
            if (hasModel) {
                m_model->SetLightColor(ShapeBase::Light::kAmbient, 0, globalConfig.ui.geometry.ambientLightColor);
            }
        }

        /*ImGui::SameLine();
        if (ColorEdit4("Diffuse", globalConfig.ui.geometry.diffuseLightColor.m128_f32, ImGuiColorEditFlags_NoInputs)) {
            if (hasModel) {
                m_model->SetLightColor(ShapeBase::Light::kDiffuse, globalConfig.ui.geometry.diffuseLightColor);
            }
        }

        ImGui::SameLine();
        if (ColorEdit4("Specular", globalConfig.ui.geometry.specularLightColor.m128_f32, ImGuiColorEditFlags_NoInputs)) {
            if (hasModel) {
                m_model->SetLightColor(ShapeBase::Light::kSpecular, globalConfig.ui.geometry.specularLightColor);
            }
        }*/

        ImGui::PopItemWidth();

        if (hasModel)
        {
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            ImGui::PushItemWidth(100.0f);

            ImGui::SameLine();
            if (SliderFloat("FOV", &globalConfig.ui.geometry.fov, 1.0f, 180.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            {
                m_model->SetFOV(globalConfig.ui.geometry.fov);
            }

            ImGui::PopItemWidth();

            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            ImGui::SameLine();
            if (ImGui::Button("Reset View"))
            {
                m_zoom = DEFAULT_ZOOM;
                m_model->ResetPos();
            }

            ImGui::SameLine(width - ImGui::CalcTextSize(m_infoStrings.m_indices.c_str()).x - 5.0f);
            ImGui::TextWrapped(m_infoStrings.m_indices.c_str());
        }
    }

    void UICollisionGeometryManager::SetResolution(
        const resolutionDesc_t& a_res)
    {
        auto& globalConfig = IConfig::GetGlobal();

        m_resolution = a_res;
        SetGlobal(globalConfig.ui.geometry.resolution, a_res.second);

        if (m_state.selected) {
            Load(*m_state.selected, true);
        }
    }

    void UICollisionGeometryManager::AutoSelectResolution(
        const char*& a_curSelName)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        for (auto& e : m_resList)
        {
            if (globalConfig.ui.geometry.resolution == e.second) {
                m_resolution = e;
                a_curSelName = e.first.c_str();
                return;
            }
        }

        auto it = m_resList.begin();
        SetResolution(*it);
        a_curSelName = it->first.c_str();
    }

    void UICollisionGeometryManager::DrawResolutionCombo()
    {
        const char* curSelName;
        if (m_resolution) {
            curSelName = m_resolution->first.c_str();
        }
        else {
            AutoSelectResolution(curSelName);
        }

        if (ImGui::BeginCombo("Resolution", curSelName))
        {
            for (auto& e : m_resList)
            {
                bool selected = e == m_resolution;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.first.c_str(), selected))
                {
                    SetResolution(e);
                }
            }

            ImGui::EndCombo();
        }
    }

    void UICollisionGeometryManager::CreateInfoStrings()
    {
        m_infoStrings.m_vertices = "Vertices: ";
        m_infoStrings.m_vertices += std::to_string(m_model->GetNumVertices());

        m_infoStrings.m_indices = "Indices: ";
        m_infoStrings.m_indices += std::to_string(m_model->GetNumIndices());
    }

    bool UICollisionGeometryManager::InitializeProfile(ColliderProfile& a_profile)
    {
        return false;
    }

    bool UICollisionGeometryManager::AllowCreateNew() const
    {
        return false;
    }

    bool UICollisionGeometryManager::AllowSave() const
    {
        return false;
    }

    void UICollisionGeometryManager::OnItemSelected(
        const std::string& a_item)
    {
        Load(a_item);
    }

    void UICollisionGeometryManager::Load(
        const std::string& a_item,
        bool a_force)
    {
        auto& pm = GetProfileManager();

        auto it = pm.Find(a_item);
        if (it != pm.End()) {
            Load(it->second, a_force);
        }
    }

    void UICollisionGeometryManager::Load(
        const ColliderProfile& a_profile,
        bool a_force)
    {
        auto model = m_model.get();

        if (!model || a_force)
        {
            if (model) {
                QueueModelRelease();
            }

            try
            {
                auto rd = DRender::GetSingleton();

                const auto& globalConfig = IConfig::GetGlobal();

                auto tmp = std::make_unique<Model>(
                    a_profile.Data().get(),
                    rd->GetDevice(),
                    rd->GetContext(),
                    rd->GetBufferSize(),
                    globalConfig.ui.geometry.resolution);

                tmp->EnableLighting(globalConfig.ui.geometry.lighting);
                tmp->SetLightColor(ShapeBase::Light::kAmbient, 0, globalConfig.ui.geometry.ambientLightColor);
                tmp->SetMaterialColor(globalConfig.ui.geometry.color);

                tmp->SetMatrices(m_matrixBackup);

                //tmp->SetZoom(m_zoom);
                tmp->SetFOV(globalConfig.ui.geometry.fov);

                m_model = std::move(tmp);

                m_matrixBackup.reset();
            }
            catch (const std::exception& e)
            {
                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while creating model:\n\n%s",
                    e.what()
                );

                return;
            }
            catch (...)
            {
                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while creating model"
                );

                return;
            }
        }
        else
        {
            try
            {
                model->LoadGeometry(a_profile.Data().get());
            }
            catch (const std::exception& e)
            {
                QueueModelRelease();

                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while loading geometry:\n\n%s",
                    e.what()
                );

                return;
            }
            catch (...)
            {
                QueueModelRelease();

                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while loading geometry"
                );

                return;
            }
        }

        CreateInfoStrings();
    }

    void UICollisionGeometryManager::QueueModelRelease()
    {
        if (!m_model.get())
            return;

        m_model->GetMatrices(m_matrixBackup);

        auto& queue = DUI::GetPreRunQueue();

        queue.AddTask<ReleaseModelTask>(std::move(m_model));
    }

    void UICollisionGeometryManager::QueueModelReload()
    {
        auto& queue = DUI::GetPreDrawQueue();

        queue.AddTask([this] {
            if (m_state.selected) { Load(*m_state.selected); } });
    }

    void UICollisionGeometryManager::OnReload(
        const ColliderProfile& a_profile)
    {
        Load(a_profile);
    }

    void UICollisionGeometryManager::OnProfileSave(
        const std::string& a_item)
    {
        if (m_state.selected && StrHelpers::iequal(a_item, *m_state.selected)) {
            Load(a_item);
        }
    }

    bool UICollisionGeometryManager::OnDeleteWarningOverride(
        const std::string& a_key,
        std::string& a_msg)
    {
        if (!IConfig::CountRefsToGeometry(a_key)) {
            return false;
        }

        a_msg = "'%s' is referenced by collision configuration.\n\nDelete anyway?";

        return true;
    }

}
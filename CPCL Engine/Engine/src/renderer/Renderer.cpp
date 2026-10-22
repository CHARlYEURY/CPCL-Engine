#include "renderer/Renderer.h"
#include "Vec4_SSE.h"

void Renderer::begin_frame(uint32_t clear_color)
{
    _fb.Clear(clear_color);
    _zb.clear_Zbuffer();           // profondeur remise à « infini »
}

void Renderer::draw_mesh(const Mesh& mesh,
    const std::vector<Maths::Vec3_SSE>& vertex_normals,
    const Texture& tex,
    const Maths::Matrix4x4_SSE& model,
    const Maths::Matrix4x4_SSE& mvp,
    const Maths::Vec3_SSE& light,
    const Maths::Vec3_SSE& half_vec)
{
    Maths::Vec4_SSE point2;
    for (int fidx = 0; fidx < mesh.faceCount(); fidx++) {
        clip_vertex tri[3];
        for (int k = 0; k < 3; k++) {
            int vi = mesh.faces[fidx * 3 + k];
            int ti = mesh.uvFaces[fidx * 3 + k];
            tri[k].clip = mvp * point2.FromPoint(mesh.verts[vi]);
            tri[k].uv = (ti >= 0) ? mesh.uvs[ti] : Maths::UV(0, 0);
            tri[k].normal = model.TransformDirection(vertex_normals[vi]);
        }

        clip_vertex out[6];
        int n_tris = 0;
        clip_triangle_near(tri[0], tri[1], tri[2], out, &n_tris);

        for (int i = 0; i < n_tris; i++) {
            project_and_draw(&out[i * 3], tex, light, half_vec);
        }
    }
}

void Renderer::project_and_draw(const clip_vertex tri[3], const Texture& tex, const Maths::Vec3_SSE& L, const Maths::Vec3_SSE& H)
{

    const int W = _fb.getWidth();
    const int Ht = _fb.getHeight();
    int sx[3], sy[3];
    float nz[3], iw[3];

    for (int k = 0; k < 3; k++) {
        const float inv = 1.0f / tri[k].clip.reg.m128_f32[3];
        sx[k] = (int)((tri[k].clip.reg.m128_f32[0] * inv + 1.0f) * 0.5f * W);
        sy[k] = (int)((1.0f - (tri[k].clip.reg.m128_f32[1] * inv + 1.0f) * 0.5f) * Ht);
        nz[k] = tri[k].clip.reg.m128_f32[2] * inv;
        iw[k] = inv;
    }

    if (_rtz.edge(sx[0], sy[0], sx[1], sy[1], sx[2], sy[2]) >= 0) return;   // back-face cull

    _rtz.draw_triangle_phong(
        sx[0], sy[0], nz[0], iw[0], tri[0].uv, tri[0].normal,
        sx[1], sy[1], nz[1], iw[1], tri[1].uv, tri[1].normal,
        sx[2], sy[2], nz[2], iw[2], tri[2].uv, tri[2].normal,
        tex, L, H);
}

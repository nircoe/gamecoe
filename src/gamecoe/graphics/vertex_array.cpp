#include <gamecoe/graphics/vertex_array.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/geometry.hpp>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_OPENGL
#include <glad/gl.h>
#endif

namespace gamecoe
{
    namespace graphics
    {
        namespace
        {
#if GAMECOE_USE_OPENGL
            void setup_vertex_attributes([[maybe_unused]] std::uint32_t vao_id, [[maybe_unused]] std::uint32_t vertex_buffer_id,
                                          [[maybe_unused]] std::optional<std::uint32_t> index_buffer_id)
            {
#if GAMECOE_HAS_DSA
                glVertexArrayVertexBuffer(vao_id, 0, vertex_buffer_id, 0, 3 * sizeof(float));
                glVertexArrayAttribFormat(vao_id, 0, 3, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribBinding(vao_id, 0, 0);
                glEnableVertexArrayAttrib(vao_id, 0);

                if (index_buffer_id)
                    glVertexArrayElementBuffer(vao_id, *index_buffer_id);
#else
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
#endif
            }
#endif

            // Leaked if destroy_shape_vertex_arrays() is never called
            // safer than an automatic destructor potentially running after the GL context is already gone.
            vertex_array *g_triangle_va = nullptr;
            vertex_array *g_rectangle_va = nullptr;
            vertex_array *g_box_va = nullptr;

#define GET_OR_CREATE_SHAPE_VA(cache, vertices, vertex_count, indices, index_count, assert_msg) \
    do { \
        if (!(cache)) \
        { \
            auto result = vertex_array::create((vertices), (vertex_count), 3, (indices), (index_count)); \
            GAMECOE_ASSERT_LOG(result.has_value(), (assert_msg)); \
            if (result) \
                (cache) = new vertex_array(std::move(*result)); \
        } \
    } while (0)
        } // namespace

        vertex_array::vertex_array(std::uint32_t id, buffer &&vertex_buffer, std::optional<buffer> &&index_buffer,
                                    std::size_t vertex_count, std::size_t index_count)
            : m_vertex_count(vertex_count), m_index_count(index_count), m_vertex_buffer(std::move(vertex_buffer)),
              m_index_buffer(std::move(index_buffer)), m_id(id) { }

        vertex_array::vertex_array(vertex_array &&other) noexcept
            : m_vertex_count(other.m_vertex_count), m_index_count(other.m_index_count),
              m_vertex_buffer(std::move(other.m_vertex_buffer)), m_index_buffer(std::move(other.m_index_buffer)),
              m_id(other.m_id)
        {
            other.reset();
        }

        vertex_array &vertex_array::operator=(vertex_array &&other) noexcept
        {
            if (this == &other)
                return *this;

            destroy();

            m_id = other.m_id;
            m_vertex_buffer = std::move(other.m_vertex_buffer);
            m_index_buffer = std::move(other.m_index_buffer);
            m_vertex_count = other.m_vertex_count;
            m_index_count = other.m_index_count;

            other.reset();

            return *this;
        }

        void vertex_array::destroy()
        {
#if GAMECOE_USE_OPENGL
            if (m_id != 0)
                glDeleteVertexArrays(1, &m_id);
#endif
        }

        void vertex_array::reset() noexcept
        {
            m_id = 0;
            m_vertex_count = 0;
            m_index_count = 0;
        }

        vertex_array::~vertex_array()
        {
            destroy();
        }

        void vertex_array::bind() const
        {
            glBindVertexArray(m_id);
        }

        void vertex_array::unbind() const
        {
            glBindVertexArray(0);
        }

        std::size_t vertex_array::vertex_count() const
        {
            return m_vertex_count;
        }

        std::size_t vertex_array::index_count() const
        {
            return m_index_count;
        }

        bool vertex_array::has_indices() const
        {
            return m_index_buffer.has_value();
        }

        std::expected<vertex_array, error> vertex_array::create(
            [[maybe_unused]] const float *vertices, [[maybe_unused]] std::size_t vertex_count,
            [[maybe_unused]] std::size_t vertex_size, [[maybe_unused]] const std::uint32_t *indices,
            [[maybe_unused]] std::size_t index_count)
        {
#if GAMECOE_USE_OPENGL
            std::uint32_t id = 0;
#if GAMECOE_HAS_DSA
            glCreateVertexArrays(1, &id);
#else
            glGenVertexArrays(1, &id);
#endif
            if (id == 0)
                return std::unexpected(
                        detail::make_error(
                            error_code::resource_creation_failure,
                            "vertex_array::create(): Could not generate vertex array"));

            struct garbage_collector
            {
                std::uint32_t id = 0;
                ~garbage_collector() { if (id != 0) glDeleteVertexArrays(1, &id); }
            } gc;
            gc.id = id;

            auto vertex_buffer_result = buffer::create_vertex_buffer();
            if (!vertex_buffer_result)
                return std::unexpected(vertex_buffer_result.error());
            buffer vertex_buffer = std::move(*vertex_buffer_result);

            std::optional<buffer> index_buffer;
            if (indices != nullptr)
            {
                auto index_buffer_result = buffer::create_index_buffer();
                if (!index_buffer_result)
                    return std::unexpected(index_buffer_result.error());
                index_buffer.emplace(std::move(*index_buffer_result));
            }

#if !GAMECOE_HAS_DSA
            glBindVertexArray(id);
#endif
            auto vertex_upload = vertex_buffer.upload_data(vertices, vertex_count * vertex_size * sizeof(float));
            if (!vertex_upload)
                return std::unexpected(vertex_upload.error());

            if (index_buffer)
            {
                auto index_upload = index_buffer->upload_data(indices, index_count * sizeof(std::uint32_t));
                if (!index_upload)
                    return std::unexpected(index_upload.error());
            }

            std::optional<std::uint32_t> index_buffer_id;
            if (index_buffer)
                index_buffer_id = index_buffer->id();
            setup_vertex_attributes(id, vertex_buffer.id(), index_buffer_id);

#if !GAMECOE_HAS_DSA
            glBindVertexArray(0);
#endif

            gc.id = 0;
            return vertex_array{id, std::move(vertex_buffer), std::move(index_buffer), vertex_count, index_count};
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "vertex_array::create(): Only OpenGL supported at the moment"));
#endif
        }

        const vertex_array *vertex_array::triangle()
        {
            static constexpr auto vertices = geometry::triangle::verticesFlat();
            GET_OR_CREATE_SHAPE_VA(g_triangle_va, vertices.data(), vertices.size() / 3, nullptr, 0,
                                    "vertex_array::triangle(): failed to create shape vertex array");
            return g_triangle_va;
        }

        const vertex_array *vertex_array::rectangle()
        {
            static constexpr auto vertices = geometry::rectangle::verticesFlat();
            static constexpr auto indices = geometry::rectangle::indices();
            GET_OR_CREATE_SHAPE_VA(g_rectangle_va, vertices.data(), vertices.size() / 3, indices.data(), indices.size(),
                                    "vertex_array::rectangle(): failed to create shape vertex array");
            return g_rectangle_va;
        }

        const vertex_array *vertex_array::box()
        {
            static constexpr auto vertices = geometry::box::verticesFlat();
            static constexpr auto indices = geometry::box::indices();
            GET_OR_CREATE_SHAPE_VA(g_box_va, vertices.data(), vertices.size() / 3, indices.data(), indices.size(),
                                    "vertex_array::box(): failed to create shape vertex array");
            return g_box_va;
        }

        // Must be called before the window/context is destroyed.
        void vertex_array::destroy_shape_vertex_arrays()
        {
            for (vertex_array **cache : {&g_triangle_va, &g_rectangle_va, &g_box_va})
            {
                delete *cache;
                *cache = nullptr;
            }
        }
    } // namespace graphics
} // namespace gamecoe

#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/scene.hpp>

namespace gamecoe
{
    std::atomic<std::uint32_t> GameObject::s_currentId = 0;

    GameObject::GameObject(Scene &scene, const std::string &name, std::optional<std::reference_wrapper<GameObject>> parent) : 
        m_id(++s_currentId), 
        m_name(name), 
        m_parent(parent), 
        m_transform(*this), 
        m_components(), 
        m_initialized(false), 
        m_active(false),
        m_scene(scene)
    {
        if (m_name == "")
            m_name = "GameObject" + std::to_string(m_id);
    }

    void GameObject::initialize()
    {
        m_initialized = true;
        for (auto& [type, component] : m_components)
        {
            component->initialize();
        }

        m_transform.initialize();

        if (m_renderer)
            m_renderer->initialize();
    }

    void GameObject::begin()
    {
        for (auto& [type, component] : m_components)
        {
            component->begin();
        }

        m_transform.begin();
        
        if (m_renderer)
            m_renderer->begin();
    }
      
    void GameObject::activate()
    {
        m_active = true;
        for (auto& [type, component] : m_components)
        {
            component->activate();
        }

        m_transform.activate();
        
        if (m_renderer)
            m_renderer->activate();
    }
        
    void GameObject::deactivate()
    {
        for (auto& [type, component] : m_components)
        {
            component->deactivate();
        }

        m_transform.deactivate();
        
        if (m_renderer)
            m_renderer->deactivate();

        m_active = false;
    }
  
    void GameObject::update()
    {
        for (auto& [type, component] : m_components)
        {
            component->update();
        }

        m_transform.update();
        
        if (m_renderer)
            m_renderer->update();
    }
        
    void GameObject::render()
    {
        if (m_renderer)
            m_renderer->render();
    }

    void GameObject::setRenderer(std::unique_ptr<Renderer> renderer, bool replace)
    {
        if (!renderer)
            detail::invalidArgument("GameObject::setRenderer(): renderer cannot be null");

        if (m_renderer && !replace)
            detail::throwError("GameObject::setRenderer(): The GameObject \"" + m_name + 
                               "\" already has a Renderer, please pass replace=true to replace the Renderer");

        // The GameObject does not have a Renderer
        // OR
        // The GameObject already has a Renderer, but the gamedev explicitly asked to replace it.

        m_renderer = std::move(renderer);
        setUpComponent(m_renderer);
    }

    void GameObject::removeRenderer()
    {
        if (!m_renderer)
        {
#if GAMECOE_USE_LOGCOE
            return logcoe::warning("GameObject::removeRenderer(): The Game Object \"" + m_name + "\" does not have a Renderer");
#else
            return;
#endif
        }

        m_renderer.reset();
    }
    
    bool GameObject::hasRenderer() const
    {
        return m_renderer != nullptr;
    }

    std::uint32_t GameObject::id() const
    {
        return m_id;
    }

    void GameObject::setName(const std::string &name)
    {
        m_name = name;
    }

    const std::string &GameObject::name() const
    {
        return m_name;
    }

    Transform &GameObject::transform()
    {
        return m_transform;
    }
    
    const Transform &GameObject::transform() const
    {
        return m_transform;
    }

    Renderer &GameObject::renderer()
    {
        if (!m_renderer)
            detail::throwError("GameObject::renderer(): The Game Object \"" + m_name + "\" does not have a Renderer");

        return *m_renderer;
    }

    const Renderer &GameObject::renderer() const
    {
        if (!m_renderer)
            detail::throwError("GameObject::renderer(): The Game Object \"" + m_name + "\" does not have a Renderer");

        return *m_renderer;
    }

    void GameObject::setParent(std::optional<std::reference_wrapper<GameObject>> parent)
    {
        m_parent = parent;
    }

    std::optional<std::reference_wrapper<GameObject>> GameObject::parent()
    {
        return m_parent;
    }

    std::optional<std::reference_wrapper<const GameObject>> GameObject::parent() const
    {
        return m_parent ?
                std::optional(std::cref(m_parent->get())) : 
                std::nullopt;
    }

    bool GameObject::hasParent() const
    {
        return m_parent.has_value();
    }

    bool GameObject::active() const
    {
        return m_active;
    }
    
    Scene &GameObject::scene()
    {
        return m_scene;
    }

    const Scene &GameObject::scene() const
    {
        return m_scene;
    }

    Game &GameObject::game()
    {
        return m_scene.game();
    }

    const Game &GameObject::game() const
    {
        return m_scene.game();
    }
} // namespace gamecoe

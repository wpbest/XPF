/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012-2015 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <XPF/GUI/Loading/Theme.hpp>
#include <XPF/GUI/Widgets/ToolTip.hpp>
#include <XPF/GUI/Container.hpp>
#include <XPF/GUI/Animation.hpp>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Widget()
    {
        m_callback.widget = this;

        addSignal<sf::Vector2f>("PositionChanged");
        addSignal<sf::Vector2f>("SizeChanged");
        addSignal("Focused");
        addSignal("Unfocused");
        addSignal("MouseEntered");
        addSignal("MouseLeft");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::~Widget()
    {
        detachTheme();

        if (m_position.x.getImpl()->parentWidget == this)
            m_position.x.getImpl()->parentWidget = nullptr;
        if (m_position.y.getImpl()->parentWidget == this)
            m_position.y.getImpl()->parentWidget = nullptr;
        if (m_size.x.getImpl()->parentWidget == this)
            m_size.x.getImpl()->parentWidget = nullptr;
        if (m_size.y.getImpl()->parentWidget == this)
            m_size.y.getImpl()->parentWidget = nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Widget(const Widget& copy) :
        sf::Drawable     {copy},
        Transformable    {copy},
        SignalWidgetBase {copy},
        enable_shared_from_this<Widget>{copy},
        m_enabled        {copy.m_enabled},
        m_visible        {copy.m_visible},
        m_parent         {copy.m_parent},
        m_opacity        {copy.m_opacity},
        m_mouseHover     {false},
        m_mouseDown      {false},
        m_focused        {false},
        m_allowFocus     {copy.m_allowFocus},
        m_draggableWidget{copy.m_draggableWidget},
        m_containerWidget{copy.m_containerWidget},
        m_font           {copy.m_font}
    {
        m_callback.widget = this;
        m_callback.widgetType = copy.m_callback.widgetType;

        if (copy.m_toolTip != nullptr)
            m_toolTip = copy.m_toolTip->clone();

        if (copy.m_renderer != nullptr)
            m_renderer = copy.m_renderer->clone(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget& Widget::operator= (const Widget& right)
    {
        if (this != &right)
        {
            sf::Drawable::operator=(right);
            Transformable::operator=(right);
            SignalWidgetBase::operator=(right);
            enable_shared_from_this::operator=(right);

            m_enabled             = right.m_enabled;
            m_visible             = right.m_visible;
            m_parent              = right.m_parent;
            m_opacity             = right.m_opacity;
            m_mouseHover          = false;
            m_mouseDown           = false;
            m_focused             = false;
            m_allowFocus          = right.m_allowFocus;
            m_draggableWidget     = right.m_draggableWidget;
            m_containerWidget     = right.m_containerWidget;
            m_font                = right.m_font;
            m_callback.widget     = this;
            m_callback.widgetType = right.m_callback.widgetType;

            if (right.m_toolTip != nullptr)
                m_toolTip = right.m_toolTip->clone();
            else
                m_toolTip = nullptr;

            if (right.m_renderer != nullptr)
                m_renderer = right.m_renderer->clone(this);
            else
                m_renderer = nullptr;

            // Animations can't be copied
            m_showAnimations = {};
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::setPosition(const Layout2d& position)
    {
        if (position.x.getImpl()->parentWidget != this)
        {
            position.x.getImpl()->parentWidget = this;
            position.x.getImpl()->recalculate();
        }
        if (position.y.getImpl()->parentWidget != this)
        {
            position.y.getImpl()->parentWidget = this;
            position.y.getImpl()->recalculate();
        }

        Transformable::setPosition(position);

        m_callback.position = getPosition();
        sendSignal("PositionChanged", getPosition());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::setSize(const Layout2d& size)
    {
        if (size.x.getImpl()->parentWidget != this)
        {
            size.x.getImpl()->parentWidget = this;
            size.x.getImpl()->recalculate();
        }
        if (size.y.getImpl()->parentWidget != this)
        {
            size.y.getImpl()->parentWidget = this;
            size.y.getImpl()->recalculate();
        }

        Transformable::setSize(size);

        m_callback.size = getSize();
        sendSignal("SizeChanged", getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sf::Vector2f Widget::getAbsolutePosition() const
    {
        if (m_parent)
            return m_parent->getAbsolutePosition() + m_parent->getChildWidgetsOffset() + getPosition();
        else
            return getPosition();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::show()
    {
        m_visible = true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::showWithEffect(ShowAnimationType type, sf::Time duration)
    {
        show();

        switch (type)
        {
            case ShowAnimationType::Fade:
            {
                m_showAnimations.push_back(std::make_shared<priv::FadeAnimation>(shared_from_this(), 0.f, getOpacity(), duration));
                setOpacity(0);
                break;
            }
            case ShowAnimationType::Scale:
            {
                m_showAnimations.push_back(std::make_shared<priv::ScaleAnimation>(shared_from_this(), getPosition() + (getSize() / 2.f), getPosition(), sf::Vector2f{0, 0}, getSize(), duration));
                setPosition(getPosition() + (getSize() / 2.f));
                setSize(0, 0);
                break;
            }
            case ShowAnimationType::SlideToRight:
            {
                m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), sf::Vector2f{-getFullSize().x, getPosition().y}, getPosition(), duration));
                setPosition({-getSize().x, getPosition().y});
                break;
            }
            case ShowAnimationType::SlideToLeft:
            {
                if (getParent())
                {
                    m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), sf::Vector2f{getParent()->getSize().x, getPosition().y}, getPosition(), duration));
                    setPosition({getParent()->getSize().x, getPosition().y});
                }
                else
                    sf::err() << "TGUI Warning: showWithEffect(SlideToLeft) does not work before widget has a parent." << std::endl;

                break;
            }
            case ShowAnimationType::SlideToBottom:
            {
                m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), sf::Vector2f{getPosition().x, -getFullSize().x}, getPosition(), duration));
                setPosition({getPosition().x, -getSize().x});
                break;
            }
            case ShowAnimationType::SlideToTop:
            {
                if (getParent())
                {
                    m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), sf::Vector2f{getPosition().x, getParent()->getSize().y}, getPosition(), duration));
                    setPosition({getPosition().x, getParent()->getSize().y});
                }
                else
                    sf::err() << "TGUI Warning: showWithEffect(SlideToTop) does not work before widget has a parent." << std::endl;

                break;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::hide()
    {
        m_visible = false;

        // If the widget is focused then it must be unfocused
        unfocus();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::hideWithEffect(ShowAnimationType type, sf::Time duration)
    {
        auto opacity = getOpacity();
        auto position = getPosition();
        auto size = getSize();

        switch (type)
        {
            case ShowAnimationType::Fade:
            {
                m_showAnimations.push_back(std::make_shared<priv::FadeAnimation>(shared_from_this(), opacity, 0.f, duration, [=](){ hide(); setOpacity(opacity); }));
                break;
            }
            case ShowAnimationType::Scale:
            {
                m_showAnimations.push_back(std::make_shared<priv::ScaleAnimation>(shared_from_this(), position, position + (size / 2.f), size, sf::Vector2f{0, 0}, duration, [=](){ hide(); setPosition(position); setSize(size); }));
                break;
            }
            case ShowAnimationType::SlideToRight:
            {
                if (getParent())
                    m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), position, sf::Vector2f{getParent()->getSize().x, position.y}, duration, [=](){ hide(); setPosition(position); }));
                else
                    sf::err() << "TGUI Warning: showWithEffect(SlideToRight) does not work before widget has a parent." << std::endl;

                break;
            }
            case ShowAnimationType::SlideToLeft:
            {
                m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), position, sf::Vector2f{-getFullSize().x, position.y}, duration, [=](){ hide(); setPosition(position); }));
                break;
            }
            case ShowAnimationType::SlideToBottom:
            {
                if (getParent())
                    m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), position, sf::Vector2f{position.x, getParent()->getSize().y}, duration, [=](){ hide(); setPosition(position); }));
                else
                    sf::err() << "TGUI Warning: showWithEffect(SlideToBottom) does not work before widget has a parent." << std::endl;

                break;
            }
            case ShowAnimationType::SlideToTop:
            {
                m_showAnimations.push_back(std::make_shared<priv::MoveAnimation>(shared_from_this(), position, sf::Vector2f{position.x, -getFullSize().x}, duration, [=](){ hide(); setPosition(position); }));
                break;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::enable()
    {
        m_enabled = true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::disable()
    {
        m_enabled = false;

        // Change the mouse button state.
        m_mouseHover = false;
        m_mouseDown = false;

        // If the widget is focused then it must be unfocused
        unfocus();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::focus()
    {
        if (m_parent)
            m_parent->focusWidget(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::unfocus()
    {
        if (m_focused)
            m_parent->unfocusWidgets();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::setOpacity(float opacity)
    {
        if (opacity < 0)
            m_opacity = 0;
        else if (opacity > 1)
            m_opacity = 1;
        else
            m_opacity = opacity;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::moveToFront()
    {
        m_parent->moveWidgetToFront(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::moveToBack()
    {
        m_parent->moveWidgetToBack(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::setToolTip(Widget::Ptr toolTip)
    {
        m_toolTip = toolTip;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Ptr Widget::getToolTip()
    {
        return m_toolTip;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::setFont(const Font& font)
    {
        m_font = font.getFont();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<sf::Font> Widget::getFont() const
    {
        return m_font;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::detachTheme()
    {
        if (m_theme)
        {
            m_theme->widgetDetached(this);
            m_theme = nullptr;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::setParent(Container* parent)
    {
        m_parent = parent;
        if (m_parent)
        {
            m_position.x.getImpl()->recalculate();
            m_position.y.getImpl()->recalculate();
            m_size.x.getImpl()->recalculate();
            m_size.y.getImpl()->recalculate();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::update(sf::Time elapsedTime)
    {
        m_animationTimeElapsed += elapsedTime;

        for (unsigned int i = 0; i < m_showAnimations.size();)
        {
            if (m_showAnimations[i]->update(elapsedTime))
                m_showAnimations.erase(m_showAnimations.begin() + i);
            else
                i++;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::leftMousePressed(float, float)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::leftMouseReleased(float, float)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::mouseMoved(float, float)
    {
        if (!m_mouseHover)
            mouseEnteredWidget();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::keyPressed(const sf::Event::KeyEvent&)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::textEntered(sf::Uint32)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::mouseWheelMoved(int, int, int)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::widgetFocused()
    {
        sendSignal("Focused");

        // Make sure the parent is also focused
        if (m_parent)
            m_parent->focus();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::widgetUnfocused()
    {
        sendSignal("Unfocused");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::mouseNotOnWidget()
    {
        if (m_mouseHover)
            mouseLeftWidget();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::mouseNoLongerDown()
    {
        m_mouseDown = false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Ptr Widget::askToolTip(sf::Vector2f mousePos)
    {
        if (m_toolTip && mouseOnWidget(mousePos.x, mousePos.y))
            return getToolTip();
        else
            return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::attachTheme(std::shared_ptr<BaseTheme> theme)
    {
        detachTheme();
        m_theme = theme;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::reload(const std::string& primary, const std::string& secondary, bool)
    {
        m_primaryLoadingParameter = primary;
        m_secondaryLoadingParameter = secondary;

        if (m_theme && primary != "")
            m_theme->initWidget(this, primary, secondary);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::mouseEnteredWidget()
    {
        m_mouseHover = true;
        sendSignal("MouseEntered");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Widget::mouseLeftWidget()
    {
        m_mouseHover = false;
        sendSignal("MouseLeft");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void WidgetRenderer::setProperty(std::string property, const std::string&)
    {
        throw Exception{"Could not set property '" + property + "', widget does not has this property."};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void WidgetRenderer::setProperty(std::string property, ObjectConverter&&)
    {
        throw Exception{"Could not set property '" + property + "', widget does not has this property."};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ObjectConverter WidgetRenderer::getProperty(std::string) const
    {
        return {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::map<std::string, ObjectConverter> WidgetRenderer::getPropertyValuePairs() const
    {
        return std::map<std::string, ObjectConverter>{};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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


#include <XPF/GUI/Container.hpp>
#include <XPF/GUI/Widgets/ToolTip.hpp>
#include <XPF/GUI/Widgets/RadioButton.hpp>
#include <XPF/GUI/Loading/WidgetSaver.hpp>
#include <XPF/GUI/Loading/WidgetLoader.hpp>

#include <stack>
#include <cassert>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Container::Container()
    {
        m_containerWidget = true;
        m_allowFocus = true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Container::Container(const Container& containerToCopy) :
        Widget                   {containerToCopy},
        m_focusedWidget          {0}
    {
        // Copy all the widgets
        for (std::size_t i = 0; i < containerToCopy.m_widgets.size(); ++i)
            add(containerToCopy.m_widgets[i]->clone(), containerToCopy.m_objName[i]);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Container& Container::operator= (const Container& right)
    {
        // Make sure it is not the same widget
        if (this != &right)
        {
            Widget::operator=(right);

            // Copy the font and the callback functions
            m_focusedWidget = 0;

            // Remove all the old widgets
            removeAllWidgets();

            // Copy all the widgets
            for (std::size_t i = 0; i < right.m_widgets.size(); ++i)
                add(right.m_widgets[i]->clone(), right.m_objName[i]);
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::setFont(const Font& font)
    {
        Widget::setFont(font);

        for (auto& widget : m_widgets)
            widget->setFont(font);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::add(const Widget::Ptr& widgetPtr, const sf::String& widgetName)
    {
        assert(widgetPtr != nullptr);

        // Let the widget inherit our font if it did not had a font yet
        if (!widgetPtr->getFont() && getFont())
            widgetPtr->setFont(getFont());

        widgetPtr->setParent(this);
        m_widgets.push_back(widgetPtr);
        m_objName.push_back(widgetName);

        if (m_opacity < 1)
            widgetPtr->setOpacity(m_opacity);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Ptr Container::get(const sf::String& widgetName, bool recursive) const
    {
        for (std::size_t i = 0; i < m_objName.size(); ++i)
        {
            if (m_objName[i] == widgetName)
            {
                return m_widgets[i];
            }
            else if (recursive && m_widgets[i]->m_containerWidget)
            {
                Widget::Ptr widget = std::static_pointer_cast<Container>(m_widgets[i])->get(widgetName, true);
                if (widget != nullptr)
                    return widget;
            }
        }

        return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Container::remove(const Widget::Ptr& widget)
    {
        // Loop through every widget
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            // Check if the pointer matches
            if (m_widgets[i] == widget)
            {
                // Unfocus the widget if it was focused
                if (m_focusedWidget == i+1)
                    unfocusWidgets();

                // Change the index of the focused widget if this is needed
                else if (m_focusedWidget > i+1)
                    m_focusedWidget--;

                // Remove the widget
                widget->setParent(nullptr);
                m_widgets.erase(m_widgets.begin() + i);
                m_objName.erase(m_objName.begin() + i);
                return true;
            }
        }

        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::removeAllWidgets()
    {
        for (auto& widget : m_widgets)
            widget->setParent(nullptr);

        // Clear the lists
        m_widgets.clear();
        m_objName.clear();

        // There are no more widgets, so none of the widgets can be focused
        m_focusedWidget = 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Container::setWidgetName(const Widget::Ptr& widget, const std::string& name)
    {
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            if (m_widgets[i] == widget)
            {
                m_objName[i] = name;
                return true;
            }
        }

        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Container::getWidgetName(const Widget::Ptr& widget, std::string& name) const
    {
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            if (m_widgets[i] == widget)
            {
                name = m_objName[i];
                return true;
            }
        }

        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::focusWidget(const Widget::Ptr& widget)
    {
        focusWidget(widget.get());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::focusWidget(Widget *const widget)
    {
        // Loop all the widgets
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            // Search for the widget that has to be focused
            if (m_widgets[i].get() == widget)
            {
                // Only continue when the widget wasn't already focused
                if (m_focusedWidget != i+1)
                {
                    // Unfocus the currently focused widget
                    if (m_focusedWidget)
                    {
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();
                    }

                    // Focus the new widget
                    m_focusedWidget = i+1;
                    widget->m_focused = true;
                    widget->widgetFocused();
                }

                break;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::focusNextWidget()
    {
        // Loop all widgets behind the focused one
        for (std::size_t i = m_focusedWidget; i < m_widgets.size(); ++i)
        {
            // If you are not allowed to focus the widget, then skip it
            if (m_widgets[i]->m_allowFocus == true)
            {
                // Make sure that the widget is visible and enabled
                if ((m_widgets[i]->m_visible) && (m_widgets[i]->m_enabled))
                {
                    if (m_focusedWidget)
                    {
                        // unfocus the current widget
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();
                    }

                    // Focus on the new widget
                    m_focusedWidget = i+1;
                    m_widgets[i]->m_focused = true;
                    m_widgets[i]->widgetFocused();
                    return;
                }
            }
        }

        // None of the widgets behind the focused one could be focused, so loop the ones before it
        if (m_focusedWidget)
        {
            for (std::size_t i = 0; i < m_focusedWidget - 1; ++i)
            {
                // If you are not allowed to focus the widget, then skip it
                if (m_widgets[i]->m_allowFocus == true)
                {
                    // Make sure that the widget is visible and enabled
                    if ((m_widgets[i]->m_visible) && (m_widgets[i]->m_enabled))
                    {
                        // unfocus the current widget
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();

                        // Focus on the new widget
                        m_focusedWidget = i+1;
                        m_widgets[i]->m_focused = true;
                        m_widgets[i]->widgetFocused();

                        return;
                    }
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::focusPreviousWidget()
    {
        // Loop the widgets before the focused one
        if (m_focusedWidget)
        {
            for (std::size_t i = m_focusedWidget - 1; i > 0; --i)
            {
                // If you are not allowed to focus the widget, then skip it
                if (m_widgets[i-1]->m_allowFocus == true)
                {
                    // Make sure that the widget is visible and enabled
                    if ((m_widgets[i-1]->m_visible) && (m_widgets[i-1]->m_enabled))
                    {
                        // unfocus the current widget
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();

                        // Focus on the new widget
                        m_focusedWidget = i;
                        m_widgets[i-1]->m_focused = true;
                        m_widgets[i-1]->widgetFocused();

                        return;
                    }
                }
            }
        }

        // None of the widgets before the focused one could be focused, so loop all widgets behind the focused one
        for (std::size_t i = m_widgets.size(); i > m_focusedWidget; --i)
        {
            // If you are not allowed to focus the widget, then skip it
            if (m_widgets[i-1]->m_allowFocus == true)
            {
                // Make sure that the widget is visible and enabled
                if ((m_widgets[i-1]->m_visible) && (m_widgets[i-1]->m_enabled))
                {
                    if (m_focusedWidget)
                    {
                        // unfocus the current widget
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();
                    }

                    // Focus on the new widget
                    m_focusedWidget = i;
                    m_widgets[i-1]->m_focused = true;
                    m_widgets[i-1]->widgetFocused();
                    return;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::unfocusWidgets()
    {
        if (m_focusedWidget)
        {
            m_widgets[m_focusedWidget-1]->m_focused = false;
            m_widgets[m_focusedWidget-1]->widgetUnfocused();
            m_focusedWidget = 0;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::uncheckRadioButtons()
    {
        // Loop through all radio buttons and uncheck them
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            if (m_widgets[i]->m_callback.widgetType == "RadioButton")
                std::static_pointer_cast<RadioButton>(m_widgets[i])->uncheck();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::moveWidgetToFront(Widget *const widget)
    {
        // Loop through all widgets
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            // Check if the widget is found
            if (m_widgets[i].get() == widget)
            {
                // Copy the widget
                m_widgets.push_back(m_widgets[i]);
                m_objName.push_back(m_objName[i]);

                // Focus the correct widget
                if ((m_focusedWidget == 0) || (m_focusedWidget == i+1))
                    m_focusedWidget = m_widgets.size()-1;
                else if (m_focusedWidget > i+1)
                    --m_focusedWidget;

                // Remove the old widget
                m_widgets.erase(m_widgets.begin() + i);
                m_objName.erase(m_objName.begin() + i);

                break;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::moveWidgetToBack(Widget *const widget)
    {
        // Loop through all widgets
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            // Check if the widget is found
            if (m_widgets[i].get() == widget)
            {
                // Copy the widget
                Widget::Ptr obj = m_widgets[i];
                std::string name = m_objName[i];
                m_widgets.insert(m_widgets.begin(), obj);
                m_objName.insert(m_objName.begin(), name);

                // Focus the correct widget
                if (m_focusedWidget == i + 1)
                    m_focusedWidget = 1;
                else if (m_focusedWidget)
                    ++m_focusedWidget;

                // Remove the old widget
                m_widgets.erase(m_widgets.begin() + i + 1);
                m_objName.erase(m_objName.begin() + i + 1);

                break;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::setOpacity(float opacity)
    {
        Widget::setOpacity(opacity);

        for (std::size_t i = 0; i < m_widgets.size(); ++i)
            m_widgets[i]->setOpacity(opacity);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::loadWidgetsFromFile(const std::string& filename)
    {
        std::ifstream in{filename};
        if (!in.is_open())
            throw Exception{"Failed to open '" + filename + "' to load the widgets from it."};

        std::stringstream stream;
        stream << in.rdbuf();
        WidgetLoader::load(std::static_pointer_cast<Container>(shared_from_this()), stream);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::saveWidgetsToFile(const std::string& filename)
    {
        std::stringstream stream;
        WidgetSaver::save(std::static_pointer_cast<Container>(shared_from_this()), stream);

        std::ofstream out{filename};
        if (!out.is_open())
            throw Exception{"Failed to open '" + filename + "' for saving the widgets to it."};

        out << stream.rdbuf();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::loadWidgetsFromStream(std::stringstream& stream)
    {
        WidgetLoader::load(std::static_pointer_cast<Container>(shared_from_this()), stream);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::saveWidgetsToStream(std::stringstream& stream)
    {
        WidgetSaver::save(std::static_pointer_cast<Container>(shared_from_this()), stream);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::leftMousePressed(float x, float y)
    {
        sf::Event event;
        event.type = sf::Event::MouseButtonPressed;
        event.mouseButton.button = sf::Mouse::Left;
        event.mouseButton.x = static_cast<int>(x - getPosition().x);
        event.mouseButton.y = static_cast<int>(y - getPosition().y);

        // Let the event manager handle the event
        handleEvent(event);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::leftMouseReleased(float x , float y)
    {
        sf::Event event;
        event.type = sf::Event::MouseButtonReleased;
        event.mouseButton.button = sf::Mouse::Left;
        event.mouseButton.x = static_cast<int>(x - getPosition().x);
        event.mouseButton.y = static_cast<int>(y - getPosition().y);

        // Let the event manager handle the event
        handleEvent(event);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::mouseMoved(float x, float y)
    {
        sf::Event event;
        event.type = sf::Event::MouseMoved;
        event.mouseMove.x = static_cast<int>(x - getPosition().x);
        event.mouseMove.y = static_cast<int>(y - getPosition().y);

        // Let the event manager handle the event
        handleEvent(event);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::keyPressed(const sf::Event::KeyEvent& event)
    {
        sf::Event newEvent;
        newEvent.type = sf::Event::KeyPressed;
        newEvent.key = event;

        // Let the event manager handle the event
        handleEvent(newEvent);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::textEntered(sf::Uint32 key)
    {
        sf::Event event;
        event.type = sf::Event::TextEntered;
        event.text.unicode = key;

        // Let the event manager handle the event
        handleEvent(event);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::mouseWheelMoved(int delta, int x, int y)
    {
        sf::Event event;
        event.type = sf::Event::MouseWheelMoved;
        event.mouseWheel.delta = delta;
        event.mouseWheel.x = static_cast<int>(x - getPosition().x);
        event.mouseWheel.y = static_cast<int>(y - getPosition().y);

        // Let the event manager handle the event
        handleEvent(event);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::mouseNotOnWidget()
    {
        if (m_mouseHover == true)
        {
            mouseLeftWidget();

            for (std::size_t i = 0; i < m_widgets.size(); ++i)
                m_widgets[i]->mouseNotOnWidget();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::mouseNoLongerDown()
    {
        Widget::mouseNoLongerDown();

        for (std::size_t i = 0; i < m_widgets.size(); ++i)
            m_widgets[i]->mouseNoLongerDown();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::widgetUnfocused()
    {
        unfocusWidgets();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Ptr Container::askToolTip(sf::Vector2f mousePos)
    {
        if (mouseOnWidget(mousePos.x, mousePos.y))
        {
            Widget::Ptr toolTip = nullptr;

            mousePos -= getPosition() + getChildWidgetsOffset();

            Widget::Ptr widget = mouseOnWhichWidget(mousePos.x, mousePos.y);
            if (widget)
            {
                toolTip = widget->askToolTip(mousePos);
                if (toolTip)
                    return toolTip;
            }

            if (m_toolTip)
                return getToolTip();
        }

        return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::update(sf::Time elapsedTime)
    {
        Widget::update(elapsedTime);

        // Loop through all widgets
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            // Update the elapsed time in widgets that need it
            if (m_widgets[i]->isVisible())
                m_widgets[i]->update(elapsedTime);
        }

        m_animationTimeElapsed = {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Container::handleEvent(sf::Event& event)
    {
        // Check if a mouse button has moved
        if ((event.type == sf::Event::MouseMoved) || ((event.type == sf::Event::TouchMoved) && (event.touch.finger == 0)))
        {
            float mouseX = (event.type == sf::Event::MouseMoved) ? static_cast<float>(event.mouseMove.x) : static_cast<float>(event.touch.x);
            float mouseY = (event.type == sf::Event::MouseMoved) ? static_cast<float>(event.mouseMove.y) : static_cast<float>(event.touch.y);

            // Loop through all widgets
            for (std::size_t i = 0; i < m_widgets.size(); ++i)
            {
                // Check if the mouse went down on the widget
                if (m_widgets[i]->m_mouseDown)
                {
                    // Some widgets should always receive mouse move events while dragging them, even if the mouse is no longer on top of them.
                    if ((m_widgets[i]->m_draggableWidget) || (m_widgets[i]->m_containerWidget))
                    {
                        m_widgets[i]->mouseMoved(mouseX, mouseY);
                        return true;
                    }
                }
            }

            // Check if the mouse is on top of a widget
            Widget::Ptr widget = mouseOnWhichWidget(mouseX, mouseY);
            if (widget != nullptr)
            {
                // Send the event to the widget
                widget->mouseMoved(mouseX, mouseY);
                return true;
            }

            return false;
        }

        // Check if a mouse button was pressed
        else if ((event.type == sf::Event::MouseButtonPressed) || ((event.type == sf::Event::TouchBegan) && (event.touch.finger == 0)))
        {
            float mouseX = (event.type == sf::Event::MouseButtonPressed) ? static_cast<float>(event.mouseButton.x) : static_cast<float>(event.touch.x);
            float mouseY = (event.type == sf::Event::MouseButtonPressed) ? static_cast<float>(event.mouseButton.y) : static_cast<float>(event.touch.y);

            // Check if the left mouse was pressed
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // Check if the mouse is on top of a widget
                Widget::Ptr widget = mouseOnWhichWidget(mouseX, mouseY);
                if (widget != nullptr)
                {
                    // Focus the widget
                    focusWidget(widget.get());

                    // Check if the widget is a container
                    if (widget->m_containerWidget)
                    {
                        // If another widget was focused then unfocus it now
                        if ((m_focusedWidget) && (m_widgets[m_focusedWidget-1] != widget))
                        {
                            m_widgets[m_focusedWidget-1]->m_focused = false;
                            m_widgets[m_focusedWidget-1]->widgetUnfocused();
                            m_focusedWidget = 0;
                        }
                    }

                    widget->leftMousePressed(mouseX, mouseY);
                    return true;
                }
                else // The mouse did not went down on a widget, so unfocus the focused widget
                    unfocusWidgets();
            }

            return false;
        }

        // Check if a mouse button was released
        else if ((event.type == sf::Event::MouseButtonReleased) || ((event.type == sf::Event::TouchEnded) && (event.touch.finger == 0)))
        {
            float mouseX = (event.type == sf::Event::MouseButtonReleased) ? static_cast<float>(event.mouseButton.x) : static_cast<float>(event.touch.x);
            float mouseY = (event.type == sf::Event::MouseButtonReleased) ? static_cast<float>(event.mouseButton.y) : static_cast<float>(event.touch.y);

            // Check if the left mouse was released
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // Check if the mouse is on top of a widget
                Widget::Ptr widget = mouseOnWhichWidget(mouseX, mouseY);
                if (widget != nullptr)
                    widget->leftMouseReleased(mouseX, mouseY);

                // Tell all the other widgets that the mouse has gone up
                for (std::vector<Widget::Ptr>::iterator it = m_widgets.begin(); it != m_widgets.end(); ++it)
                {
                    if (*it != widget)
                        (*it)->mouseNoLongerDown();
                }

                if (widget != nullptr)
                    return true;
            }

            return false;
        }

        // Check if a key was pressed
        else if (event.type == sf::Event::KeyPressed)
        {
            // Only continue when the character was recognised
            if (event.key.code != sf::Keyboard::Unknown)
            {
                // Check if there is a focused widget
                if (m_focusedWidget)
                {
                #ifdef SFML_SYSTEM_ANDROID
                    // Map delete to backspace on android
                    if (event.key.code == sf::Keyboard::Delete)
                        event.key.code = sf::Keyboard::BackSpace;
                #endif

                    // Tell the widget that the key was pressed
                    m_widgets[m_focusedWidget-1]->keyPressed(event.key);

                    return true;
                }
            }

            return false;
        }

        // Check if a key was released
        else if (event.type == sf::Event::KeyReleased)
        {
            // Change the focus to another widget when the tab key was pressed
            if (event.key.code == sf::Keyboard::Tab)
                return tabKeyPressed();
            else
                return false;
        }

        // Also check if text was entered (not a special key)
        else if (event.type == sf::Event::TextEntered)
        {
            // Check if the character that we pressed is allowed
            if ((event.text.unicode >= 32) && (event.text.unicode != 127))
            {
                // Tell the widget that the key was pressed
                if (m_focusedWidget)
                {
                    m_widgets[m_focusedWidget-1]->textEntered(event.text.unicode);
                    return true;
                }
            }

            return false;
        }

        // Check for mouse wheel scrolling
        else if (event.type == sf::Event::MouseWheelMoved)
        {
            // Find the widget under the mouse
            Widget::Ptr widget = mouseOnWhichWidget(static_cast<float>(event.mouseWheel.x), static_cast<float>(event.mouseWheel.y));
            if (widget != nullptr)
            {
                // Send the event to the widget
                widget->mouseWheelMoved(event.mouseWheel.delta, event.mouseWheel.x,  event.mouseWheel.y);
                return true;
            }

            return false;
        }
        else // Event is ignored
            return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Container::focusNextWidgetInContainer()
    {
        // Don't do anything when the tab key usage is disabled
        if (TGUI_TabKeyUsageEnabled == false)
            return false;

        // Loop through all widgets
        for (std::size_t i = m_focusedWidget; i < m_widgets.size(); ++i)
        {
            // If you are not allowed to focus the widget, then skip it
            if (m_widgets[i]->m_allowFocus == true)
            {
                // Make sure that the widget is visible and enabled
                if ((m_widgets[i]->m_visible) && (m_widgets[i]->m_enabled))
                {
                    // Container widgets can only be focused it they contain focusable widgets
                    if ((!m_widgets[i]->m_containerWidget) || (std::static_pointer_cast<Container>(m_widgets[i])->focusNextWidgetInContainer()))
                    {
                        if (m_focusedWidget > 0)
                        {
                            // Unfocus the current widget
                            m_widgets[m_focusedWidget-1]->m_focused = false;
                            m_widgets[m_focusedWidget-1]->widgetUnfocused();
                        }

                        // Focus on the new widget
                        m_focusedWidget = i+1;
                        m_widgets[i]->m_focused = true;
                        m_widgets[i]->widgetFocused();

                        return true;
                    }
                }
            }
        }

        // We have the highest id
        unfocusWidgets();
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Container::tabKeyPressed()
    {
        // Don't do anything when the tab key usage is disabled
        if (TGUI_TabKeyUsageEnabled == false)
            return false;

        // Check if a container is focused
        if (m_focusedWidget)
        {
            if (m_widgets[m_focusedWidget-1]->m_containerWidget)
            {
                // Focus the next widget in container
                if (std::static_pointer_cast<Container>(m_widgets[m_focusedWidget-1])->focusNextWidgetInContainer())
                    return true;
            }
        }

        // Loop all widgets behind the focused one
        for (std::size_t i = m_focusedWidget; i < m_widgets.size(); ++i)
        {
            // If you are not allowed to focus the widget, then skip it
            if (m_widgets[i]->m_allowFocus == true)
            {
                // Make sure that the widget is visible and enabled
                if ((m_widgets[i]->m_visible) && (m_widgets[i]->m_enabled))
                {
                    if (m_focusedWidget)
                    {
                        // unfocus the current widget
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();
                    }

                    // Focus on the new widget
                    m_focusedWidget = i+1;
                    m_widgets[i]->m_focused = true;
                    m_widgets[i]->widgetFocused();
                    return true;
                }
            }
        }

        // None of the widgets behind the focused one could be focused, so loop the ones before it
        if (m_focusedWidget)
        {
            for (std::size_t i = 0; i < m_focusedWidget-1; ++i)
            {
                // If you are not allowed to focus the widget, then skip it
                if (m_widgets[i]->m_allowFocus == true)
                {
                    // Make sure that the widget is visible and enabled
                    if ((m_widgets[i]->m_visible) && (m_widgets[i]->m_enabled))
                    {
                        // unfocus the current widget
                        m_widgets[m_focusedWidget-1]->m_focused = false;
                        m_widgets[m_focusedWidget-1]->widgetUnfocused();

                        // Focus on the new widget
                        m_focusedWidget = i+1;
                        m_widgets[i]->m_focused = true;
                        m_widgets[i]->widgetFocused();
                        return true;
                    }
                }
            }
        }

        // If the currently focused container widget is the only widget to focus, then focus its next child widget
        if ((m_focusedWidget) && (m_widgets[m_focusedWidget-1]->m_containerWidget))
        {
            std::static_pointer_cast<Container>(m_widgets[m_focusedWidget-1])->tabKeyPressed();
            return true;
        }

        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Widget::Ptr Container::mouseOnWhichWidget(float x, float y)
    {
        bool widgetFound = false;
        Widget::Ptr widget = nullptr;

        // Loop through all widgets
        for (std::vector<Widget::Ptr>::reverse_iterator it = m_widgets.rbegin(); it != m_widgets.rend(); ++it)
        {
            // Check if the widget is visible and enabled
            if (((*it)->m_visible) && ((*it)->m_enabled))
            {
                if (widgetFound == false)
                {
                    // Return the widget if the mouse is on top of it
                    if ((*it)->mouseOnWidget(x, y))
                    {
                        widget = *it;
                        widgetFound = true;
                    }
                }
                else // The widget was already found, so tell the other widgets that the mouse can't be on them
                    (*it)->mouseNotOnWidget();
            }
        }

        return widget;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Container::drawWidgetContainer(sf::RenderTarget* target, const sf::RenderStates& states) const
    {
        // Draw all widgets when they are visible
        for (std::size_t i = 0; i < m_widgets.size(); ++i)
        {
            if (m_widgets[i]->m_visible)
                m_widgets[i]->draw(*target, states);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    GuiContainer::GuiContainer()
    {
        m_callback.widgetType = "GuiContainer";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void GuiContainer::setSize(const Layout2d&)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool GuiContainer::mouseOnWidget(float, float)
    {
        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void GuiContainer::draw(sf::RenderTarget&, sf::RenderStates) const
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

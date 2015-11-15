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


#ifndef TGUI_WINDOW_HPP
#define TGUI_WINDOW_HPP


#include <queue>

#include <XPF/GUI/Container.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Gui class
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API Gui
    {
      public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Default constructor
        ///
        /// If you use this constructor then you will still have to call the setWindow yourself.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Gui();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Construct the gui and set the window on which the gui should be drawn.
        ///
        /// @param window  The sfml window that will be used by the gui.
        ///
        /// If you use this constructor then you will no longer have to call setWindow yourself.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Gui(sf::RenderWindow& window);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Construct the gui and set the target on which the gui should be drawn.
        ///
        /// @param window  The render target that will be used by the gui.
        ///
        /// If you use this constructor then you will no longer have to call setWindow yourself.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Gui(sf::RenderTarget& window);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Deleted copy constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Gui(const Gui& copy) = delete;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Deleted assignment operator overload
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Gui& operator=(const Gui& right) = delete;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set the window on which the gui should be drawn.
        ///
        /// @param window  The sfml window that will be used by the gui.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setWindow(sf::RenderWindow& window);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set the target on which the gui should be drawn.
        ///
        /// @param window  The render target that will be used by the gui.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setWindow(sf::RenderTarget& window);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the window on which the gui is being drawn.
        ///
        /// @return The sfml that is used by the gui.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        sf::RenderTarget* getWindow() const
        {
            return m_window;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the view that is used by the gui
        ///
        /// @param view  The new view
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setView(const sf::View& view);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Return the view that is currently used by the gui
        ///
        /// @return Currently set view
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const sf::View& getView() const
        {
            return m_view;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Passes the event to the widgets.
        ///
        /// @param event  The event that was polled from the gui
        ///
        /// @return Has the event been consumed?
        ///         When this function returns false, then the event was ignored by all widgets.
        ///
        /// You should call this function in your event loop.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool handleEvent(sf::Event event);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Draws all the widgets that were added to the gui.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void draw();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the size of the container.
        ///
        /// @return Size of the container.
        ///
        /// This size will equal the size of the window.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        sf::Vector2f getSize() const
        {
            return sf::Vector2f{m_window->getSize()};
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the internal container of the Gui.
        ///
        /// This could be useful when having a function that should accept both the gui and e.g. a child window as parameter.
        ///
        /// @warning Not all functions in the Container class make sense for the Gui (which is the reason that the Gui does not
        ///          inherit from Container). So calling some functions (e.g. setSize) will have no effect.
        ///
        /// @return Reference to the internal Container class
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        GuiContainer::Ptr getContainer() const
        {
            return m_container;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the global font.
        ///
        /// @param font  Font to use
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setFont(const Font& font)
        {
            m_container->setFont(font);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the global font.
        ///
        /// @return global font
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::shared_ptr<sf::Font> getFont() const
        {
            return m_container->getFont();
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns a list of all the widgets.
        ///
        /// @return Vector of all widget pointers
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const std::vector< Widget::Ptr >& getWidgets()
        {
            return m_container->getWidgets();
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns a list of the names of all the widgets.
        ///
        /// @return Vector of all widget names
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const std::vector<sf::String>& getWidgetNames()
        {
            return m_container->getWidgetNames();
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Adds a widget to the container.
        ///
        /// @param widgetPtr   Pointer to the widget you would like to add
        /// @param widgetName  If you want to access the widget later then you must do this with this name
        ///
        /// Usage example:
        /// @code
        /// tgui::Picture::Ptr pic(container); // Create a picture and add it to the container
        /// container.remove(pic);             // Remove the picture from the container
        /// container.add(pic);                // Add the picture to the container again
        /// @endcode
        ///
        /// @warning The widget name should not contain whitespace.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void add(const Widget::Ptr& widgetPtr, const sf::String& widgetName = "");


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns a pointer to an earlier created widget.
        ///
        /// @param widgetName The name that was given to the widget when it was added to the container.
        /// @param recursive  Should the function also search for widgets inside containers that are inside this container?
        ///
        /// @return Pointer to the earlier created widget
        ///
        /// @warning This function will return nullptr when an unknown widget name was passed.
        ///
        /// Usage example:
        /// @code
        /// tgui::Picture::Ptr pic(container, "picName");
        /// tgui::Picture::Ptr pic2 = container.get("picName");
        /// @endcode
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Widget::Ptr get(const sf::String& widgetName, bool recursive = false) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns a pointer to an earlier created widget.
        ///
        /// @param widgetName The name that was given to the widget when it was added to the container.
        /// @param recursive  Should the function also search for widgets inside containers that are inside this container?
        ///
        /// @return Pointer to the earlier created widget.
        ///         The pointer will already be casted to the desired type.
        ///
        /// @warning This function will return nullptr when an unknown widget name was passed.
        ///
        /// Usage example:
        /// @code
        /// tgui::Picture::Ptr pic(container, "picName");
        /// tgui::Picture::Ptr pic2 = container.get<tgui::Picture>("picName");
        /// @endcode
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        template <class T>
        typename T::Ptr get(const sf::String& widgetName, bool recursive = false) const
        {
            return m_container->get<T>(widgetName, recursive);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Removes a single widget that was added to the container.
        ///
        /// @param widget  Pointer to the widget to remove
        ///
        /// Usage example:
        /// @code
        /// tgui::Picture::Ptr pic(container, "picName");
        /// tgui::Picture::Ptr pic2(container, "picName2");
        /// container.remove(pic);
        /// container.remove(container.get("picName2"));
        /// @endcode
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void remove(const Widget::Ptr& widget);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Removes all widgets that were added to the container.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void removeAllWidgets();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the name of a widget.
        ///
        /// @param widget  Widget of which the name should be changed
        /// @param name    New name for the widget
        ///
        /// @return True when the name was changed, false when the widget wasn't part of this container.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool setWidgetName(const Widget::Ptr& widget, const std::string& name);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the name of a widget.
        ///
        /// @param widget  Widget of which the name should be retrieved
        /// @param name    Name for the widget
        ///
        /// @return False is returned when the widget wasn't part of this container.
        ///         In this case the name parameter is left unchanged.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool getWidgetName(const Widget::Ptr& widget, std::string& name) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Focuses a widget.
        ///
        /// The previously focused widget will be unfocused.
        ///
        /// @param widget  The widget that has to be focused.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void focusWidget(Widget::Ptr& widget);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Focuses the next widget.
        ///
        /// The currently focused widget will be unfocused, even if it was the only widget.
        /// When no widget was focused, the first widget in the container will be focused.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void focusNextWidget();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Focuses the previous widget.
        ///
        /// The currently focused widget will be unfocused, even if it was the only widget.
        /// When no widget was focused, the last widget in the container will be focused.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void focusPreviousWidget();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Unfocus all the widgets.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void unfocusWidgets();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Uncheck all the radio buttons.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void uncheckRadioButtons();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Places a widget before all other widgets.
        ///
        /// @param widget  The widget that should be moved to the front
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void moveWidgetToFront(Widget::Ptr& widget);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Places a widget behind all other widgets.
        ///
        /// @param widget  The widget that should be moved to the back
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void moveWidgetToBack(Widget::Ptr& widget);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Load the child widgets from a text file
        ///
        /// @param filename  Filename of the widget file
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void loadWidgetsFromFile(const std::string& filename);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Save the child widgets to a text file
        ///
        /// @param filename  Filename of the widget file
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void saveWidgetsToFile(const std::string& filename);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Load the child widgets from a string stream
        ///
        /// @param stream  stringstream that contains the widget file
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void loadWidgetsFromStream(std::stringstream& stream);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Save this the child widgets to a text file
        ///
        /// @param stream  stringstream to which the widget file will be added
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void saveWidgetsToStream(std::stringstream& stream);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        // Update the internal clock to make animation possible. This function is called automatically by the draw function.
        // You will thus only need to call it yourself when you are drawing everything manually.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void updateTime(const sf::Time& elapsedTime);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      protected:

        // The internal clock which is used for animation of widgets
        sf::Clock m_clock;

        // The sfml window or other target to draw on
        sf::RenderTarget* m_window;

        // Does m_Window contains a sf::RenderWindow?
        bool m_accessToWindow;

        // Internal container to store all widgets
        GuiContainer::Ptr m_container = std::make_shared<GuiContainer>();

        Widget::Ptr m_visibleToolTip = nullptr;
        sf::Time m_tooltipTime;
        bool m_tooltipPossible = false;
        sf::Vector2f m_lastMousePos;

        sf::View m_view;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_WINDOW_HPP

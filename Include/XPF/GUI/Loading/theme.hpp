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


#ifndef TGUI_THEME_HPP
#define TGUI_THEME_HPP


#include <XPF/GUI/Widget.hpp>
#include <XPF/GUI/Loading/WidgetConverter.hpp>
#include <XPF/GUI/Loading/ThemeLoader.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Base class for the Theme classes
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API BaseTheme : public std::enable_shared_from_this<BaseTheme>
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Virtual destructor
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual ~BaseTheme() = default;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Virtual function that gets called when a widget is connected to this theme
        ///
        /// @param widget  The widget that was attached to this theme
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void widgetAttached(Widget* widget);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Virtual function that gets called when a widget is disconnected from this theme
        ///
        /// @param widget  The widget that was detached from this theme
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void widgetDetached(Widget* widget);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief This function can be used inside a widget to load other widgets without access to the derived theme class
        ///
        /// @param primary  Primary parameter of the loader
        /// @param secondary Secondary parameter of the loader
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual WidgetConverter internalLoad(const std::string& primary, const std::string& secondary) = 0;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Virtual function called by the widget to finish its initialization
        ///
        /// @param widget    The widget that needs to be initialized
        /// @param primary   Primary parameter of the loader
        /// @param secondary Secondary parameter of the loader
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void initWidget(Widget* widget, std::string primary, std::string secondary) = 0;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the construct function of a specific widget type
        ///
        /// @param type        Type of the widget which will cause this construct function to be used
        /// @param constructor New function to be called when this type of widget is being loaded
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static void setConstructFunction(const std::string& type, const std::function<Widget::Ptr()>& constructor);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the function that will load the widget theme data
        ///
        /// @param themeLoader  Pointer to the new loader
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static void setThemeLoader(const std::shared_ptr<BaseThemeLoader>& themeLoader);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Calls the protected widget->reload(primary, secondary, force) function
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void widgetReload(Widget* widget, const std::string& primary = "", const std::string& secondary = "", bool force = false);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:
        static std::map<std::string, std::function<Widget::Ptr()>> m_constructors; ///< Widget creator functions
        static std::shared_ptr<BaseThemeLoader> m_themeLoader;  ///< Theme loading functions, they read the theme file
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Default Theme class
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API Theme : public BaseTheme
    {
    public:

        typedef std::shared_ptr<Theme> Ptr; ///< Shared theme pointer


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Construct the theme class, with an optional theme file to load
        ///
        /// @param filename  Filename of the theme file
        ///
        /// @warning The theme has to be used as a std::make_shared
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Theme(const std::string& filename = "");


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Load the widget from the theme
        ///
        /// @param className  Name of the class inside the theme file (equals widget type when no class is given)
        ///
        /// @exception Exception when the requested class name could not be loaded from the file
        /// @exception Exception when there was no loader for this type of widget
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        WidgetConverter load(std::string className);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Reload the theme with a different filename
        ///
        /// @param filename  Filename of the new theme file
        ///
        /// All widgets that are connected to this theme will be reloaded with the new theme file.
        /// The class names thus have to match with those from the old theme file!
        ///
        /// @exception Exception when one of the widgets failed to reload
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void reload(const std::string& filename);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Reload all widgets that were loaded with a certain class name
        ///
        /// @param oldClassName  Class name that was used to load the widget
        /// @param newClassName  Class name to load the widget with
        ///
        /// @exception Exception when the requested new class name could not be loaded from the file
        /// @exception Exception when one of the widgets failed to reload
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void reload(std::string oldClassName, std::string newClassName);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Reload a specific widget
        ///
        /// @param widget     The widget to reload
        /// @param className  Class name to load the widget with
        ///
        /// @exception Exception when the requested class name could not be loaded from the file
        /// @exception Exception when the widget failed to reload
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void reload(Widget::Ptr widget, std::string className);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change a property of the renderer of all widgets that were loaded with a certain class name
        ///
        /// @param className The class name of the widgets
        /// @param property  The property that you would like to change
        /// @param value     The new serialized value that you like to assign to the property
        ///
        /// @throw Exception when deserialization fails or when the widget does not have this property.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setProperty(std::string className, const std::string& property, const std::string& value);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change a property of the renderer of all widgets that were loaded with a certain class name
        ///
        /// @param className The class name of the widgets
        /// @param property  The property that you would like to change
        /// @param value     The new value that you like to assign to the property.
        ///                  The ObjectConverter is implicitly constructed from the possible value types.
        ///
        /// @throw Exception for unknown properties or when value was of a wrong type.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setProperty(std::string className, const std::string& property, ObjectConverter&& value);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Retrieve the serialized value of a certain property of widgets that use a certain class name
        ///
        /// @param className The class name of the widgets
        /// @param property  The property that you would like to retrieve
        ///
        /// @return The serialized value or an empty string when the property did not exist
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::string getProperty(std::string className, std::string property) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get a map with all properties and their values from widgets loaded with a certain class name
        ///
        /// @param className The class name of the widgets
        ///
        /// @return Serialized property-value pairs
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::map<std::string, std::string> getPropertyValuePairs(std::string className) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Function that gets called when a widget is disconnected from this theme
        ///
        /// @param widget  The widget that was detached from this theme
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void widgetDetached(Widget* widget) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief This function can be used inside a widget to load other widgets without access to the derived theme class
        ///
        /// @param filename  The filename that has to match the one used to load this theme
        /// @param className The class name of the theme to load
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual WidgetConverter internalLoad(const std::string& filename, const std::string& className) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Function called by the widget to finish its initialization
        ///
        /// @param widget    The widget that needs to be initialized
        /// @param filename  Filename that was used to load the widget
        /// @param className Class name that was used to load the widget
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void initWidget(Widget* widget, std::string filename, std::string className) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    private:
        std::string m_filename;
        std::string m_resourcePath;
        bool m_resourcePathLock = false;
        std::map<Widget*, std::string> m_widgets; // Map widget to class name
        std::map<std::string, std::string> m_widgetTypes; // Map class name to type
        std::map<std::string, std::map<std::string, std::string>> m_widgetProperties; // Map class name to property-value pairs

        friend class ThemeTest;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_THEME_HPP

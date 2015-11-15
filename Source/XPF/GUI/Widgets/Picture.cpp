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


#include <XPF/GUI/Widgets/Picture.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Picture()
    {
        m_callback.widgetType = "Picture";

        addSignal("DoubleClicked");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Picture(const std::string& filename, bool fullyClickable) :
        Picture{}
    {
        setTexture(filename, fullyClickable);
        setSize(m_texture.getImageSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Picture(const sf::Texture& texture) :
        Picture{}
    {
        setTexture(texture);
        setSize(m_texture.getImageSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Picture(const Texture& texture, bool fullyClickable) :
        Picture{}
    {
        auto size = m_texture.getSize();
        setTexture(texture, fullyClickable);
        setSize(size);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setTexture(const std::string& filename, bool fullyClickable)
    {
        m_fullyClickable = fullyClickable;
        m_loadedFilename = getResourcePath() + filename;

        m_texture.load(m_loadedFilename);
        m_texture.setSize(getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setTexture(const sf::Texture& texture)
    {
        auto data = std::make_shared<TextureData>();
        data->texture = texture;
        m_texture.setTexture(data);
        m_texture.setSize(getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setTexture(const Texture& texture, bool fullyClickable)
    {
        m_fullyClickable = fullyClickable;
        m_texture = texture;
        m_texture.setSize(getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Ptr Picture::copy(Picture::ConstPtr picture)
    {
        if (picture)
            return std::make_shared<Picture>(*picture);
        else
            return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setPosition(const Layout2d& position)
    {
        Widget::setPosition(position);

        m_texture.setPosition(getPosition());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setSize(const Layout2d& size)
    {
        Widget::setSize(size);

        m_texture.setSize(getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setSmooth(bool smooth)
    {
        m_texture.setSmooth(smooth);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setOpacity(float opacity)
    {
        Widget::setOpacity(opacity);

        m_texture.setColor({255, 255, 255, static_cast<sf::Uint8>(m_opacity * 255)});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Picture::mouseOnWidget(float x, float y)
    {
        // Check if the mouse is on top of the picture
        if (sf::FloatRect{getPosition().x, getPosition().y, getSize().x, getSize().y}.contains(x, y))
        {
            // We sometimes want clicks to go through transparent parts of the picture
            if (!m_fullyClickable && m_texture.isTransparentPixel(x, y))
                return false;
            else
                return true;
        }

        if (m_mouseHover)
            mouseLeftWidget();

        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::leftMouseReleased(float x, float y)
    {
        bool mouseDown = m_mouseDown;

        ClickableWidget::leftMouseReleased(x, y);

        if (mouseDown)
        {
            // Check if you double-clicked
            if (m_possibleDoubleClick)
            {
                m_possibleDoubleClick = false;

                m_callback.mouse.x = static_cast<int>(x - getPosition().x);
                m_callback.mouse.y = static_cast<int>(y - getPosition().y);
                sendSignal("DoubleClicked", sf::Vector2f{x - getPosition().x, y - getPosition().y});
            }
            else // This is the first click
            {
                m_animationTimeElapsed = {};
                m_possibleDoubleClick = true;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::update(sf::Time elapsedTime)
    {
        Widget::update(elapsedTime);

        // When double-clicking, the second click has to come within 500 milliseconds
        if (m_animationTimeElapsed >= sf::milliseconds(500))
        {
            m_animationTimeElapsed = {};
            m_possibleDoubleClick = false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_texture, states);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

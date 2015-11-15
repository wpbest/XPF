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


#include <XPF/OpenGL.hpp>

#include <XPF/GUI/Container.hpp>
#include <XPF/GUI/Widgets/extra/Slider2d.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
namespace ext
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Slider2d::Slider2d(const Texture& trackNormal, const Texture& thumbNormal, const Texture& trackHover, const Texture& thumbHover)
    {
        m_callback.widgetType = "Slider2d";
        m_draggableWidget = true;

        addSignal<sf::Vector2f>("ValueChanged");
        addSignal<sf::Vector2f>("ThumbReturnedToCenter");

        m_textureTrackNormal = trackNormal;
        m_textureThumbNormal = thumbNormal;
        m_textureTrackHover = trackHover;
        m_textureThumbHover = thumbHover;

        // Set the size of the slider
        setSize(m_textureTrackNormal.getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Slider2d::Ptr Slider2d::copy(Slider2d::ConstPtr slider)
    {
        if (slider)
            return std::make_shared<Slider2d>(*slider);
        else
            return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::setMinimum(const sf::Vector2f& minimum)
    {
        // Set the new minimum
        m_minimum = minimum;

        // The minimum can never be greater than the maximum
        if (m_minimum.x > m_maximum.x)
            m_maximum.x = m_minimum.x;
        if (m_minimum.y > m_maximum.y)
            m_maximum.y = m_minimum.y;

        // When the value is below the minimum then adjust it
        if (m_value.x < m_minimum.x)
            m_value.x = m_minimum.x;
        if (m_value.y < m_minimum.y)
            m_value.y = m_minimum.y;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::setMaximum(const sf::Vector2f& maximum)
    {
        // Set the new maximum
        m_maximum = maximum;

        // The maximum can never be below the minimum
        if (m_maximum.x < m_minimum.x)
            m_minimum.x = m_maximum.x;
        if (m_maximum.y < m_minimum.y)
            m_minimum.y = m_maximum.y;

        // When the value is above the maximum then adjust it
        if (m_value.x > m_maximum.x)
            m_value.x = m_maximum.x;
        if (m_value.y > m_maximum.y)
            m_value.y = m_maximum.y;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::setValue(sf::Vector2f value)
    {
        // When the value is below the minimum or above the maximum then adjust it
        if (value.x < m_minimum.x)
            value.x = m_minimum.x;
        else if (value.x > m_maximum.x)
            value.x = m_maximum.x;

        if (value.y < m_minimum.y)
            value.y = m_minimum.y;
        else if (value.y > m_maximum.y)
            value.y = m_maximum.y;

        if (m_value != value)
        {
            m_value = value;

            m_callback.value2d = m_value;
            sendSignal("ValueChanged", m_value);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::enableThumbCenter(bool autoCenterThumb)
    {
        m_returnThumbToCenter = autoCenterThumb;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::centerThumb()
    {
        sf::Vector2f newValue = {(m_maximum.x + m_minimum.x) * 0.5f, (m_maximum.y + m_minimum.y) * 0.5f};
        if (m_value != newValue)
        {
            m_callback.value2d = m_value;
            sendSignal("ThumbReturnedToCenter", m_value);

            setValue(newValue);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::leftMousePressed(float x, float y)
    {
        ClickableWidget::leftMousePressed(x, y);

        // Refresh the value
        mouseMoved(x, y);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::leftMouseReleased(float x, float y)
    {
        ClickableWidget::leftMouseReleased(x, y);

        if (m_returnThumbToCenter)
            centerThumb();
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::mouseMoved(float x, float y)
    {
        if (!m_mouseHover)
            mouseEnteredWidget();

        // Check if the mouse button is down
        if (m_mouseDown)
        {
            // If the position is positive then calculate the correct value
            if ((y - getPosition().y) > 0)
                m_value.y = ((y - getPosition().y) / getSize().y) * (m_maximum.y - m_minimum.y) + m_minimum.y;
            else // The position is negative, the calculation can't be done (but is not needed)
                m_value.y = m_minimum.y;

            // If the position is positive then calculate the correct value
            if ((x - getPosition().x) > 0)
                m_value.x = ((x - getPosition().x) / getSize().x) * (m_maximum.x - m_minimum.x) + m_minimum.x;
            else // The position is negative, the calculation can't be done (but is not needed)
                m_value.x = m_minimum.x;

            // Set the new value, making sure that it lies within the minimum and maximum
            setValue(m_value);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::widgetFocused()
    {
        // A slider can't be focused (yet)
        unfocus();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::mouseNoLongerDown()
    {
        m_mouseDown = false;

        if (m_returnThumbToCenter)
            centerThumb();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Slider2d::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        const sf::View& view = target.getView();

        // Calculate the scale factor of the view
        float scaleViewX = target.getSize().x / view.getSize().x;
        float scaleViewY = target.getSize().y / view.getSize().y;

        // Get the global position
        sf::Vector2f topLeftPosition = {((getAbsolutePosition().x - view.getCenter().x + (view.getSize().x / 2.f)) * view.getViewport().width) + (view.getSize().x * view.getViewport().left),
                                        ((getAbsolutePosition().y - view.getCenter().y + (view.getSize().y / 2.f)) * view.getViewport().height) + (view.getSize().y * view.getViewport().top)};
        sf::Vector2f bottomRightPosition = {(getAbsolutePosition().x + getSize().x - view.getCenter().x + (view.getSize().x / 2.f)) * view.getViewport().width + (view.getSize().x * view.getViewport().left),
                                            (getAbsolutePosition().y + getSize().y - view.getCenter().y + (view.getSize().y / 2.f)) * view.getViewport().height + (view.getSize().y * view.getViewport().top)};

        // Adjust the position
        states.transform.translate(getPosition());

        // Calculate the scale of the slider
        sf::Vector2f scaling;
        scaling.x = getSize().x / m_textureTrackNormal.getSize().x;
        scaling.y = getSize().y / m_textureTrackNormal.getSize().y;

        // Set the scale of the slider
        states.transform.scale(scaling);

        // Draw the track image
        if (m_mouseHover && m_textureTrackHover.isLoaded())
            target.draw(m_textureTrackHover, states);
        else
            target.draw(m_textureTrackNormal, states);

        // Undo the scale
        states.transform.scale(1.f / scaling.x, 1.f / scaling.y);

        // Check if the thumb should be scaled together with the slider
        if (m_fixedThumbSize)
        {
            states.transform.translate((((m_value.x - m_minimum.x) / (m_maximum.x - m_minimum.x)) * m_textureTrackNormal.getSize().x * scaling.x) - (m_textureThumbNormal.getSize().x * 0.5f),
                                       (((m_value.y - m_minimum.y) / (m_maximum.y - m_minimum.y)) * m_textureTrackNormal.getSize().y * scaling.y) - (m_textureThumbNormal.getSize().y * 0.5f));
        }
        else // The thumb must be scaled
        {
            states.transform.translate((((m_value.x - m_minimum.x) / (m_maximum.x - m_minimum.x)) * m_textureTrackNormal.getSize().x * scaling.x) - (m_textureThumbNormal.getSize().x * 0.5f * scaling.y),
                                       (((m_value.y - m_minimum.y) / (m_maximum.y - m_minimum.y)) * m_textureTrackNormal.getSize().y * scaling.y) - (m_textureThumbNormal.getSize().y * 0.5f * scaling.x));

            // Set the scale for the thumb
            states.transform.scale(scaling);
        }

        // Get the old clipping area
        GLint scissor[4];
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        // Calculate the clipping area
        GLint scissorLeft = std::max(static_cast<GLint>(topLeftPosition.x * scaleViewX), scissor[0]);
        GLint scissorTop = std::max(static_cast<GLint>(topLeftPosition.y * scaleViewY), static_cast<GLint>(target.getSize().y) - scissor[1] - scissor[3]);
        GLint scissorRight = std::min(static_cast<GLint>(bottomRightPosition.x * scaleViewX), scissor[0] + scissor[2]);
        GLint scissorBottom = std::min(static_cast<GLint>(bottomRightPosition.y * scaleViewY), static_cast<GLint>(target.getSize().y) - scissor[1]);

        if (scissorRight < scissorLeft)
            scissorRight = scissorLeft;
        else if (scissorBottom < scissorTop)
            scissorTop = scissorBottom;

        // Set the clipping area
        glScissor(scissorLeft, target.getSize().y - scissorBottom, scissorRight - scissorLeft, scissorBottom - scissorTop);

        // Draw the thumb image
        if (m_mouseHover && m_textureThumbHover.isLoaded())
            target.draw(m_textureThumbHover, states);
        else
            target.draw(m_textureThumbNormal, states);

        // Reset the old clipping area
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // ext
} // tgui

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

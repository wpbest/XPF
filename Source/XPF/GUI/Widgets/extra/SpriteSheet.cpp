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


#include <XPF/GUI/Widgets/extra/SpriteSheet.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
namespace ext
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SpriteSheet::SpriteSheet(const std::string& filename, unsigned int rows, unsigned int columns) :
        Picture{filename}
    {
        m_callback.widgetType = "SpriteSheet";

        setCells(rows, columns);
        if ((rows > 1) || (columns > 1))
            setSize(m_texture.getImageSize().x / columns, m_texture.getImageSize().y / rows);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SpriteSheet::Ptr SpriteSheet::copy(SpriteSheet::ConstPtr spriteSheet)
    {
        if (spriteSheet)
            return std::make_shared<SpriteSheet>(*spriteSheet);
        else
            return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SpriteSheet::setPosition(const Layout2d& position)
    {
        Widget::setPosition(position);

        m_texture.setPosition({getPosition().x - ((m_visibleCell.x-1) * m_texture.getSize().x / m_columns),
                               getPosition().y - ((m_visibleCell.y-1) * m_texture.getSize().y / m_rows)});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SpriteSheet::setSize(const Layout2d& size)
    {
        Widget::setSize(size);

        m_texture.setSize({getSize().x * m_columns, getSize().y * m_rows});

        // Make the correct part of the image visible
        m_texture.setTextureRect({(m_visibleCell.x-1) * m_texture.getSize().x / m_columns,
                                  (m_visibleCell.y-1) * m_texture.getSize().y / m_rows,
                                  m_texture.getSize().x / m_columns,
                                  m_texture.getSize().y / m_rows});

        // Make sure the image is displayed at the correct position
        m_texture.setPosition(getPosition().x - ((m_visibleCell.x-1) * m_texture.getSize().x / m_columns),
                              getPosition().y - ((m_visibleCell.y-1) * m_texture.getSize().y / m_rows));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SpriteSheet::setCells(unsigned int rows, unsigned int columns)
    {
        // You can't have 0 rows
        if (rows == 0)
            rows = 1;

        // You can't have 0 columns
        if (columns == 0)
            columns = 1;

        m_rows = rows;
        m_columns = columns;

        updateSize();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SpriteSheet::setVisibleCell(unsigned int row, unsigned int column)
    {
        // You can't make a row visible that doesn't exist
        if (row > m_rows)
            row = m_rows;
        else if (row == 0)
            row = 1;

        // You can't make a column visible that doesn't exist
        if (column > m_columns)
            column = m_columns;
        else if (column == 0)
            column = 1;

        // store the visible cell
        m_visibleCell.x = column;
        m_visibleCell.y = row;

        // Make the correct part of the image visible
        m_texture.setTextureRect({(m_visibleCell.x-1) * m_texture.getSize().x / m_columns,
                                  (m_visibleCell.y-1) * m_texture.getSize().y / m_rows,
                                  m_texture.getSize().x / m_columns,
                                  m_texture.getSize().y / m_rows});

        // Make sure the image is displayed at the correct position
        m_texture.setPosition(getPosition().x - ((m_visibleCell.x-1) * m_texture.getSize().x / m_columns),
                              getPosition().y - ((m_visibleCell.y-1) * m_texture.getSize().y / m_rows));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // ext
} // tgui

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

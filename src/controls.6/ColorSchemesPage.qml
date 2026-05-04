import QtQuick 
import QtQuick.Controls 
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.terminal as Term

/**
 * @brief A page entry for picking one of the available color schemes for the terminal display
 * 
 * @code
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import org.mauikit.terminal as Term

Maui.ApplicationWindow
{
    id: root

    Maui.Page
    {
        Maui.Controls.showCSD: true
        anchors.fill: parent

        headBar.leftContent: ToolButton
        {
            icon.name: "love"
            onClicked: _dialog.open()
        }

        Maui.SettingsDialog
        {
            id: _dialog

            Maui.SectionItem
            {
                text: "Colors"
                onClicked: _dialog.addPage(_colorsComp)
            }
            Component
            {
                id: _colorsComp
                Term.ColorSchemesPage
                {
                    currentColorScheme: _term.kterminal.colorScheme
                    onCurrentColorSchemeChanged: _term.kterminal.colorScheme = currentColorScheme
                }

            }
        }

        Term.Terminal
        {
            id: _term
            anchors.fill: parent
            kterminal.colorScheme: "Ubuntu"
        }
    }
}


 * @endcode
 * 
 */
Maui.SettingsPage
{
    id:  control
    title: i18n("Color Scheme")
    
    property string currentColorScheme            
    
    Maui.SectionItem
    {
        label1.text: i18n("Color Scheme")
        label2.text: i18n("Change the color scheme of the terminal.")
        
        GridLayout
        {
            id: _grid
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            columnSpacing: Maui.Style.space.medium
            rowSpacing: Maui.Style.space.medium

            readonly property int minimumCellWidth: 220
            readonly property int responsiveColumns: Math.max(1, Math.floor((width + columnSpacing) / (minimumCellWidth + columnSpacing)))
            columns: responsiveColumns
            
            Repeater
            {
                model: Term.ColorSchemesModel {}
                
                delegate: Maui.GridBrowserDelegate
                {
                    Layout.fillWidth: true
                    Layout.preferredWidth: Math.max(_grid.minimumCellWidth, Math.floor((_grid.width - ((_grid.columns - 1) * _grid.columnSpacing)) / _grid.columns))
                    Layout.minimumWidth: _grid.minimumCellWidth

                    checked: model.name === control.currentColorScheme
                    onClicked: control.currentColorScheme = model.name
                    
                    template.iconComponent: Control
                    {
                        implicitWidth: Math.max(220, _grid.minimumCellWidth)
                        implicitHeight: Math.max(_layout.implicitHeight + topPadding + bottomPadding, 64)
                        padding: Maui.Style.space.small
                        
                        background: Rectangle
                        {
                            color: model.background
                            radius: Maui.Style.radiusV
                        }
                        
                        contentItem: Column
                        {
                            id:_layout
                            spacing: 2
                            
                            Text
                            {
                                wrapMode: Text.NoWrap
                                elide: Text.ElideLeft
                                width: parent.width
                                //                                    font.pointSize: Maui.Style.fontSizes.small
                                text: "Hello world!"
                                color: model.foreground
                                font.family: "Monospaced"
                            }
                            
                            Rectangle
                            {
                                radius: 2
                                height: 8
                                width: parent.width
                                color: model.highlight
                            }
                            
                            Rectangle
                            {
                                radius: 2
                                height: 8
                                width: parent.width
                                color: model.color3
                            }
                            
                            Rectangle
                            {
                                radius: 2
                                height: 8
                                width: parent.width
                                color: model.color4
                            }
                        }
                    }
                    
                    label1.text: model.name
                    label1.wrapMode: Text.NoWrap
                    label1.elide: Text.ElideRight
                }
            }
        }
    }
}

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import Qt.labs.settings 1.1

Window
{
    id: window
    visible: true
    width: 300
    height: 500

    // Фиксация минимального размера окна
    minimumHeight : 500
    minimumWidth : 300

    Settings
    {
        category: "WindowSizeAndPozition"
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
    }

    // Сигнал нажатия на клавишу равно
    // durationTime - продолжительность вычисления
    // expression - выражения, которое необходимо вычислить
    signal genericExpression( int durationTime, string expression )

    // Функция обновления информационной панели
    function updateInfoArea( msg )
    {
        infoArea.append( msg )
        scrollView.ScrollBar.vertical.increase()
    }

    Rectangle
    {
        id: resultArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: parent.height / 8
        border.color: "white"
        border.width: 1
        color: "#46a2da"
        Text
        {
            id: resultText
            anchors.leftMargin: buttons.implicitMargin
            anchors.rightMargin: buttons.implicitMargin
            anchors.fill: parent
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            text: "0"
            color: "white"
            font.pixelSize: window.height * 3 / 32
            font.family: "Open Sans Regular"
            fontSizeMode: Text.Fit
        }
    }

    Rectangle
    {
        id: calcTimeArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: resultArea.bottom
        height: parent.height / 13
        border.color: "white"
        border.width: 1
        color: "gray"
        Text
        {
            id: calcTimeText
            anchors.leftMargin: parent.implicitMargin
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: control.left
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: " Время вычисления, с"
            wrapMode: Text.NoWrap
            color: "white"
            font.pixelSize: calcTimeArea.height * 3 / 8
            font.family: "Open Sans Regular"
            fontSizeMode: Text.Fit
        }

        SpinBox
        {
            id: control
            x: 71
            y: 3
            anchors.right: parent.right
            anchors.rightMargin: 1
            anchors.leftMargin: calcTimeArea.implicitMargin
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height - 2
            visible: true
            transformOrigin: Item.Center
            editable: false
            stepSize: 1
            from: 0
            to: 86400
            value: 0

            validator: IntValidator
            {
                locale: control.locale.name
                bottom: Math.min ( control.from, control.to )
                top: Math.max ( control.from, control.to )
            }
        }
    }

    Item
    {
        id: buttons
        anchors.top: calcTimeArea.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: logRect.top
        property real implicitMargin:
        {
            var ret = 0;
            for ( var i = 0; i < visibleChildren.length; ++i )
            {
                var child = visibleChildren[i];
                ret += (child.implicitMargin || 0);
            }
            return ret / visibleChildren.length;
        }

        Repeater
        {
            id: operations
            model: ["÷", "×", "+", "-"]
            Button
            {
                y: 0
                x: index * width
                width: parent.width / 4
                height: parent.height / 5
                color: pressed ? "#5caa15" : "#80c342"
                text: modelData
                fontHeight: 0.4
                onClicked:
                {
                    var existOperand = false
                    for ( var i = 0; i < operations.model.length; ++i )
                    {
                        if ( resultText.text.toString().indexOf(operations.model[i]) != -1 )
                        {
                            existOperand = true
                            break
                        }
                    }

                    if ( !existOperand )
                        resultText.text =  resultText.text + text
                }
            }
        }

        Repeater
        {
            id: digits
            model: ["7", "8", "9", "4", "5", "6", "1", "2", "3", "0", ".", "C"]
            Button
            {
                x: (index % 3) * width
                y: Math.floor(index / 3 + 1) * height
                width: parent.width / 4
                height: parent.height / 5
                color: pressed ? "#d6d6d6" : "#eeeeee"
                text: modelData
                onClicked:
                {
                    if ( text == "." )
                    {
                        var indexOperand = -1
                        for ( var i = 0; i < operations.model.length; ++i )
                        {
                            indexOperand = resultText.text.toString().indexOf ( operations.model[i] )
                            if ( indexOperand !== -1 )
                            {
                                break
                            }
                        }

                        if ( indexOperand !== -1 )
                        {
                            var operandB = resultText.text.toString().substr( indexOperand + 1, resultText.text.toString().length )

                            var operandBPointindex = operandB.indexOf( "." )
                            var operandBlen = operandB.length
                            var zeroText1 = operandBlen === 0 ? "0" : ""
                            if ( operandBPointindex === - 1 )
                                resultText.text =  resultText.text + zeroText1 + text
                        }
                        else
                        {
                            var operandAPointindex = resultText.text.toString().indexOf( "." )
                            var operandAlen = resultText.text.toString().length
                            var zeroText2 = operandAlen === 0 ? "0" : ""
                            if ( operandAPointindex === -1 )
                                resultText.text =  resultText.text + zeroText2 + text
                        }
                    }
                    else if ( text == "C" )
                    {
                        resultText.text = "0"
                    }
                    else
                    {
                        var oldText
                        if ( resultText.text === "0" )
                            oldText = ""
                        else
                            oldText = resultText.text

                        resultText.text = oldText + text
                    }
                }
            }
        }

        Button
        {
            id: resultButton

            x: 3 * width
            y: parent.height / 5
            textHeight: y - 2
            fontHeight: 0.4
            width: parent.width / 4
            height: y * 4
            color: pressed ? "#e0b91c" : "#face20"
            text: "="
            onClicked:
            {
                window.genericExpression ( control.value, resultText.text )
                resultText.text = "0"
            }
        }
    }

    Rectangle
    {
        id: logRect
        anchors.top: buttons.button
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: scrollView.top
        height: parent.height / 30
        border.color: "white"
        border.width: 1
        color: "gray"
        Text
        {
            id: logText
            anchors.leftMargin: parent.implicitMargin
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.left
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: " Журнал событий:"
            wrapMode: Text.NoWrap
            color: "white"
            font.pixelSize: logRect.height * 3 / 4
            font.family: "Open Sans Regular"
            fontSizeMode: Text.Fit
        }
    }
    ScrollView
    {
        id: scrollView
        anchors.top: logRect.button
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height / 3
        clip: true
        focus: true       
        Keys.onUpPressed: ScrollBar.vertical.decrease()
        Keys.onDownPressed: ScrollBar.vertical.increase()
        TextArea
        {
            id: infoArea
            anchors.fill: parent
            height: parent.height
            readOnly: true
            textFormat: TextEdit.RichText
            font.pixelSize: 14
            font.family: "Open Sans Regular"
            background: Rectangle
            {
                anchors.fill: parent
                gradient: Gradient
                {
                    GradientStop { position: 0.0; color: "#eeeeee" }
                    GradientStop { position: 1.0; color: "white" }
                }
            }
            onTextChanged:
            {
                if ( scrollView.ScrollBar.vertical )
                    scrollView.ScrollBar.vertical.position = 1
            }
        }
    }
}

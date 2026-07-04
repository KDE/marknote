import QtQuick
import QtQuick.Layouts
import QtQml.Models

DelegateChooser {
    id: blockChooser
    role: "blockType"

    DelegateChoice {
        roleValue: MDOptions.ElementType.Heading
        BlockHeading { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.Paragraph
        BlockParagraph { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.Blockquote
        BlockQuote { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.List
        BlockList { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.ListItem
        BlockListItem { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.HorizontalLine
        BlockHorizontalLine { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.Code
        BlockCode { }
    }

    DelegateChoice {
        roleValue: MDOptions.ElementType.Table
        BlockTable { }
    }

    // Default choice
    DelegateChoice {
        Item {
            visible: false
        }
    }
}
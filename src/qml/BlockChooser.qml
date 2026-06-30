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
        Text {
            text: "Unknown block type"
        }
    }
}
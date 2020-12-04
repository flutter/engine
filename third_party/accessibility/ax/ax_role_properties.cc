// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_role_properties.h"

#include "ax_build/build_config.h"

#include "ax_enums.h"

namespace ax {

namespace {

#if defined(OS_WIN) || defined(OS_CHROMEOS)
constexpr bool kExposeLayoutTableAsDataTable = true;
#else
constexpr bool kExposeLayoutTableAsDataTable = false;
#endif  // defined(OS_WIN)

}  // namespace

bool HasPresentationalChildren(const ax::Role role) {
  // See http://www.w3.org/TR/core-aam-1.1/#exclude_elements2.
  if (IsImage(role))
    return true;

  switch (role) {
    case ax::Role::kButton:
    case ax::Role::kCheckBox:
    case ax::Role::kMath:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kMenuItemRadio:
    case ax::Role::kMenuListOption:
    case ax::Role::kProgressIndicator:
    case ax::Role::kScrollBar:
    case ax::Role::kSlider:
    case ax::Role::kSwitch:
    case ax::Role::kTab:
      return true;
    default:
      return false;
  }
}

bool IsAlert(const ax::Role role) {
  switch (role) {
    case ax::Role::kAlert:
    case ax::Role::kAlertDialog:
      return true;
    default:
      return false;
  }
}

bool IsButton(const ax::Role role) {
  // According to the WAI-ARIA spec, native button or role="button"
  // supports |aria-expanded| and |aria-pressed|.
  // If the button has |aria-expanded| set, then it takes on
  // Role::kPopUpButton.
  // If the button has |aria-pressed| set, then it takes on
  // Role::kToggleButton.
  // https://www.w3.org/TR/wai-aria-1.1/#button
  return role == ax::Role::kButton || role == ax::Role::kPopUpButton ||
         role == ax::Role::kToggleButton;
}

bool IsClickable(const ax::Role role) {
  switch (role) {
    case ax::Role::kButton:
    case ax::Role::kCheckBox:
    case ax::Role::kColorWell:
    case ax::Role::kComboBoxMenuButton:
    case ax::Role::kDate:
    case ax::Role::kDateTime:
    case ax::Role::kDisclosureTriangle:
    case ax::Role::kDocBackLink:
    case ax::Role::kDocBiblioRef:
    case ax::Role::kDocGlossRef:
    case ax::Role::kDocNoteRef:
    case ax::Role::kImeCandidate:
    case ax::Role::kInputTime:
    case ax::Role::kLink:
    case ax::Role::kListBox:
    case ax::Role::kListBoxOption:
    case ax::Role::kMenuItem:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kMenuItemRadio:
    case ax::Role::kMenuListOption:
    case ax::Role::kPdfActionableHighlight:
    case ax::Role::kPopUpButton:
    case ax::Role::kPortal:
    case ax::Role::kRadioButton:
    case ax::Role::kSearchBox:
    case ax::Role::kSpinButton:
    case ax::Role::kSwitch:
    case ax::Role::kTab:
    case ax::Role::kTextField:
    case ax::Role::kTextFieldWithComboBox:
    // kTree and related roles are not included because they are not natively
    // supported by HTML and so their "clickable" behavior is uncertain.
    case ax::Role::kToggleButton:
      return true;
    default:
      return false;
  }
}

bool IsCellOrTableHeader(const ax::Role role) {
  switch (role) {
    case ax::Role::kCell:
    case ax::Role::kColumnHeader:
    case ax::Role::kRowHeader:
      return true;
    case ax::Role::kLayoutTableCell:
      return kExposeLayoutTableAsDataTable;
    default:
      return false;
  }
}

bool IsContainerWithSelectableChildren(const ax::Role role) {
  switch (role) {
    case ax::Role::kComboBoxGrouping:
    case ax::Role::kComboBoxMenuButton:
    case ax::Role::kGrid:
    case ax::Role::kListBox:
    case ax::Role::kListGrid:
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:
    case ax::Role::kMenuListPopup:
    case ax::Role::kRadioGroup:
    case ax::Role::kTabList:
    case ax::Role::kToolbar:
    case ax::Role::kTree:
    case ax::Role::kTreeGrid:
      return true;
    default:
      return false;
  }
}

bool IsControl(const ax::Role role) {
  switch (role) {
    case ax::Role::kButton:
    case ax::Role::kCheckBox:
    case ax::Role::kColorWell:
    case ax::Role::kComboBoxMenuButton:
    case ax::Role::kDisclosureTriangle:
    case ax::Role::kListBox:
    case ax::Role::kListGrid:
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:
    case ax::Role::kMenuItem:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kMenuItemRadio:
    case ax::Role::kMenuListOption:
    case ax::Role::kMenuListPopup:
    case ax::Role::kPdfActionableHighlight:
    case ax::Role::kPopUpButton:
    case ax::Role::kRadioButton:
    case ax::Role::kScrollBar:
    case ax::Role::kSearchBox:
    case ax::Role::kSlider:
    case ax::Role::kSpinButton:
    case ax::Role::kSwitch:
    case ax::Role::kTab:
    case ax::Role::kTextField:
    case ax::Role::kTextFieldWithComboBox:
    case ax::Role::kToggleButton:
    case ax::Role::kTree:
      return true;
    default:
      return false;
  }
}

bool IsControlOnAndroid(const ax::Role role, bool isFocusable) {
  switch (role) {
    case ax::Role::kSplitter:
      return isFocusable;
    case ax::Role::kTreeItem:
    case ax::Role::kDate:
    case ax::Role::kDateTime:
    case ax::Role::kInputTime:
    case ax::Role::kDocBackLink:
    case ax::Role::kDocBiblioRef:
    case ax::Role::kDocGlossRef:
    case ax::Role::kDocNoteRef:
    case ax::Role::kLink:
      return true;
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:
    case ax::Role::kNone:
    case ax::Role::kUnknown:
    case ax::Role::kTree:
    case ax::Role::kDialog:
    case ax::Role::kAlert:
      return false;
    default:
      return IsControl(role);
  }
}

bool IsDocument(const ax::Role role) {
  switch (role) {
    case ax::Role::kDocument:
    case ax::Role::kRootWebArea:
    case ax::Role::kWebArea:
      return true;
    default:
      return false;
  }
}

bool IsDialog(const ax::Role role) {
  switch (role) {
    case ax::Role::kAlertDialog:
    case ax::Role::kDialog:
      return true;
    default:
      return false;
  }
}

bool IsForm(const ax::Role role) {
  switch (role) {
    case ax::Role::kForm:
      return true;
    default:
      return false;
  }
}

bool IsFormatBoundary(const ax::Role role) {
  return IsControl(role) || IsHeading(role) || IsImageOrVideo(role);
}

bool IsHeading(const ax::Role role) {
  switch (role) {
    case ax::Role::kHeading:
    case ax::Role::kDocSubtitle:
      return true;
    default:
      return false;
  }
}

bool IsHeadingOrTableHeader(const ax::Role role) {
  switch (role) {
    case ax::Role::kColumnHeader:
    case ax::Role::kDocSubtitle:
    case ax::Role::kHeading:
    case ax::Role::kRowHeader:
      return true;
    default:
      return false;
  }
}

bool IsIframe(ax::Role role) {
  switch (role) {
    case ax::Role::kIframe:
    case ax::Role::kIframePresentational:
      return true;
    default:
      return false;
  }
}

bool IsImageOrVideo(const ax::Role role) {
  return IsImage(role) || role == ax::Role::kVideo;
}

bool IsImage(const ax::Role role) {
  switch (role) {
    case ax::Role::kCanvas:
    case ax::Role::kDocCover:
    case ax::Role::kGraphicsSymbol:
    case ax::Role::kImage:
    case ax::Role::kImageMap:
    case ax::Role::kSvgRoot:
      return true;
    default:
      return false;
  }
}

bool IsItemLike(const ax::Role role) {
  switch (role) {
    case ax::Role::kArticle:
    case ax::Role::kComment:
    case ax::Role::kListItem:
    case ax::Role::kMenuItem:
    case ax::Role::kMenuItemRadio:
    case ax::Role::kTab:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kTreeItem:
    case ax::Role::kListBoxOption:
    case ax::Role::kMenuListOption:
    case ax::Role::kRadioButton:
    case ax::Role::kDescriptionListTerm:
    case ax::Role::kTerm:
      return true;
    default:
      return false;
  }
}

bool IsLandmark(const ax::Role role) {
  switch (role) {
    case ax::Role::kBanner:
    case ax::Role::kComplementary:
    case ax::Role::kContentInfo:
    case ax::Role::kForm:
    case ax::Role::kMain:
    case ax::Role::kNavigation:
    case ax::Role::kRegion:
    case ax::Role::kSearch:
      return true;
    default:
      return false;
  }
}

bool IsLink(const ax::Role role) {
  switch (role) {
    case ax::Role::kDocBackLink:
    case ax::Role::kDocBiblioRef:
    case ax::Role::kDocGlossRef:
    case ax::Role::kDocNoteRef:
    case ax::Role::kLink:
      return true;
    default:
      return false;
  }
}

bool IsList(const ax::Role role) {
  switch (role) {
    case ax::Role::kDescriptionList:
    case ax::Role::kDirectory:
    case ax::Role::kDocBibliography:
    case ax::Role::kList:
    case ax::Role::kListBox:
    case ax::Role::kListGrid:
      return true;
    default:
      return false;
  }
}

bool IsListItem(const ax::Role role) {
  switch (role) {
    case ax::Role::kDescriptionListTerm:
    case ax::Role::kDocBiblioEntry:
    case ax::Role::kDocEndnote:
    case ax::Role::kListBoxOption:
    case ax::Role::kListItem:
    case ax::Role::kTerm:
      return true;
    default:
      return false;
  }
}

bool IsMenuItem(ax::Role role) {
  switch (role) {
    case ax::Role::kMenuItem:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kMenuItemRadio:
      return true;
    default:
      return false;
  }
}

bool IsMenuRelated(const ax::Role role) {
  switch (role) {
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:
    case ax::Role::kMenuItem:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kMenuItemRadio:
    case ax::Role::kMenuListOption:
    case ax::Role::kMenuListPopup:
      return true;
    default:
      return false;
  }
}

bool IsPresentational(const ax::Role role) {
  switch (role) {
    case ax::Role::kNone:
    case ax::Role::kPresentational:
      return true;
    default:
      return false;
  }
}

bool IsRadio(const ax::Role role) {
  switch (role) {
    case ax::Role::kRadioButton:
    case ax::Role::kMenuItemRadio:
      return true;
    default:
      return false;
  }
}

bool IsRangeValueSupported(const ax::Role role) {
  // https://www.w3.org/TR/wai-aria-1.1/#aria-valuenow
  // https://www.w3.org/TR/wai-aria-1.1/#aria-valuetext
  // Roles that support aria-valuetext / aria-valuenow
  switch (role) {
    case ax::Role::kMeter:
    case ax::Role::kProgressIndicator:
    case ax::Role::kScrollBar:
    case ax::Role::kSlider:
    case ax::Role::kSpinButton:
    case ax::Role::kSplitter:
      return true;
    default:
      return false;
  }
}

bool IsReadOnlySupported(const ax::Role role) {
  // https://www.w3.org/TR/wai-aria-1.1/#aria-readonly
  // Roles that support aria-readonly
  switch (role) {
    case ax::Role::kCheckBox:
    case ax::Role::kColorWell:
    case ax::Role::kComboBoxGrouping:
    case ax::Role::kComboBoxMenuButton:
    case ax::Role::kDate:
    case ax::Role::kDateTime:
    case ax::Role::kGrid:
    case ax::Role::kInputTime:
    case ax::Role::kListBox:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kMenuItemRadio:
    case ax::Role::kMenuListPopup:
    case ax::Role::kPopUpButton:
    case ax::Role::kRadioButton:
    case ax::Role::kRadioGroup:
    case ax::Role::kSearchBox:
    case ax::Role::kSlider:
    case ax::Role::kSpinButton:
    case ax::Role::kSwitch:
    case ax::Role::kTextField:
    case ax::Role::kTextFieldWithComboBox:
    case ax::Role::kToggleButton:
    case ax::Role::kTreeGrid:
      return true;

    // https://www.w3.org/TR/wai-aria-1.1/#aria-readonly
    // ARIA-1.1+ 'gridcell', supports aria-readonly, but 'cell' does not.
    //
    // https://www.w3.org/TR/wai-aria-1.1/#columnheader
    // https://www.w3.org/TR/wai-aria-1.1/#rowheader
    // While the [columnheader|rowheader] role can be used in both interactive
    // grids and non-interactive tables, the use of aria-readonly and
    // aria-required is only applicable to interactive elements.
    // Therefore, [...] user agents SHOULD NOT expose either property to
    // assistive technologies unless the columnheader descends from a grid.
    case ax::Role::kCell:
    case ax::Role::kRowHeader:
    case ax::Role::kColumnHeader:
      return false;
    default:
      return false;
  }
}

bool IsRowContainer(const ax::Role role) {
  switch (role) {
    case ax::Role::kGrid:
    case ax::Role::kListGrid:
    case ax::Role::kTable:
    case ax::Role::kTree:
    case ax::Role::kTreeGrid:
      return true;
    case ax::Role::kLayoutTable:
      return kExposeLayoutTableAsDataTable;
    default:
      return false;
  }
}

bool IsSection(const ax::Role role) {
  if (IsLandmark(role) || IsSelect(role))
    return true;

  switch (role) {
    case ax::Role::kAlert:
    case ax::Role::kAlertDialog:  // Subclass of kAlert.
    case ax::Role::kCell:
    case ax::Role::kColumnHeader:  // Subclass of kCell.
    case ax::Role::kDefinition:
    case ax::Role::kDirectory:  // Subclass of kList.
    case ax::Role::kFeed:       // Subclass of kList.
    case ax::Role::kFigure:
    case ax::Role::kGrid:  // Subclass of kTable.
    case ax::Role::kGroup:
    case ax::Role::kImage:
    case ax::Role::kList:
    case ax::Role::kListItem:
    case ax::Role::kLog:
    case ax::Role::kMarquee:
    case ax::Role::kMath:
    case ax::Role::kNote:
    case ax::Role::kProgressIndicator:  // Subclass of kStatus.
    case ax::Role::kRow:                // Subclass of kGroup.
    case ax::Role::kRowHeader:          // Subclass of kCell.
    case ax::Role::kSection:
    case ax::Role::kStatus:
    case ax::Role::kTable:
    case ax::Role::kTabPanel:
    case ax::Role::kTerm:
    case ax::Role::kTimer:    // Subclass of kStatus.
    case ax::Role::kToolbar:  // Subclass of kGroup.
    case ax::Role::kTooltip:
    case ax::Role::kTreeItem:  // Subclass of kListItem.
      return true;
    default:
      return false;
  }
}

bool IsSectionhead(const ax::Role role) {
  switch (role) {
    case ax::Role::kColumnHeader:
    case ax::Role::kHeading:
    case ax::Role::kRowHeader:
    case ax::Role::kTab:
      return true;
    default:
      return false;
  }
}

bool IsSelect(const ax::Role role) {
  switch (role) {
    case ax::Role::kComboBoxGrouping:
    case ax::Role::kListBox:
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:  // Subclass of kMenu.
    case ax::Role::kRadioGroup:
    case ax::Role::kTree:
    case ax::Role::kTreeGrid:  // Subclass of kTree.
      return true;
    default:
      return false;
  }
}

bool IsSelectElement(const ax::Role role) {
  // Depending on their "size" attribute, <select> elements come in two flavors:
  // the first appears like a list box and the second like a popup menu.
  switch (role) {
    case ax::Role::kListBox:
    case ax::Role::kPopUpButton:
      return true;
    default:
      return false;
  }
}

bool IsSetLike(const ax::Role role) {
  switch (role) {
    case ax::Role::kDescriptionList:
    case ax::Role::kDirectory:
    case ax::Role::kDocBibliography:
    case ax::Role::kFeed:
    case ax::Role::kGroup:
    case ax::Role::kList:
    case ax::Role::kListBox:
    case ax::Role::kListGrid:
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:
    case ax::Role::kMenuListPopup:
    case ax::Role::kPopUpButton:
    case ax::Role::kRadioGroup:
    case ax::Role::kTabList:
    case ax::Role::kTree:
      return true;
    default:
      return false;
  }
}

bool IsStaticList(const ax::Role role) {
  switch (role) {
    case ax::Role::kList:
    case ax::Role::kDescriptionList:
      return true;
    default:
      return false;
  }
}

bool IsStructure(const ax::Role role) {
  if (IsSection(role) || IsSectionhead(role))
    return true;

  switch (role) {
    case ax::Role::kApplication:
    case ax::Role::kDocument:
    case ax::Role::kArticle:  // Subclass of kDocument.
    case ax::Role::kPresentational:
    case ax::Role::kRowGroup:
    case ax::Role::kSplitter:
    // Dpub roles.
    case ax::Role::kDocAbstract:
    case ax::Role::kDocAcknowledgments:
    case ax::Role::kDocAfterword:
    case ax::Role::kDocAppendix:
    case ax::Role::kDocBiblioEntry:
    case ax::Role::kDocBibliography:
    case ax::Role::kDocChapter:
    case ax::Role::kDocColophon:
    case ax::Role::kDocConclusion:
    case ax::Role::kDocCover:
    case ax::Role::kDocCredit:
    case ax::Role::kDocCredits:
    case ax::Role::kDocDedication:
    case ax::Role::kDocEndnote:
    case ax::Role::kDocEndnotes:
    case ax::Role::kDocEpigraph:
    case ax::Role::kDocEpilogue:
    case ax::Role::kDocErrata:
    case ax::Role::kDocExample:
    case ax::Role::kDocFootnote:
    case ax::Role::kDocForeword:
    case ax::Role::kDocGlossary:
    case ax::Role::kDocIndex:
    case ax::Role::kDocIntroduction:
    case ax::Role::kDocNotice:
    case ax::Role::kDocPageBreak:
    case ax::Role::kDocPageList:
    case ax::Role::kDocPart:
    case ax::Role::kDocPreface:
    case ax::Role::kDocPrologue:
    case ax::Role::kDocQna:
    case ax::Role::kDocSubtitle:
    case ax::Role::kDocTip:
    case ax::Role::kDocToc:
      return true;
    default:
      return false;
  }
}

bool IsTableColumn(ax::Role role) {
  return role == ax::Role::kColumn;
}

bool IsTableHeader(ax::Role role) {
  switch (role) {
    case ax::Role::kColumnHeader:
    case ax::Role::kRowHeader:
      return true;
    default:
      return false;
  }
}

bool IsTableLike(const ax::Role role) {
  switch (role) {
    case ax::Role::kGrid:
    case ax::Role::kListGrid:
    case ax::Role::kTable:
    case ax::Role::kTreeGrid:
      return true;
    case ax::Role::kLayoutTable:
      return kExposeLayoutTableAsDataTable;
    default:
      return false;
  }
}

bool IsTableRow(ax::Role role) {
  switch (role) {
    case ax::Role::kRow:
      return true;
    case ax::Role::kLayoutTableRow:
      return kExposeLayoutTableAsDataTable;
    default:
      return false;
  }
}

bool IsText(ax::Role role) {
  switch (role) {
    case ax::Role::kInlineTextBox:
    case ax::Role::kLineBreak:
    case ax::Role::kStaticText:
      return true;
    default:
      return false;
  }
}

bool SupportsExpandCollapse(const ax::Role role) {
  switch (role) {
    case ax::Role::kComboBoxGrouping:
    case ax::Role::kComboBoxMenuButton:
    case ax::Role::kDisclosureTriangle:
    case ax::Role::kTextFieldWithComboBox:
    case ax::Role::kTreeItem:
      return true;
    default:
      return false;
  }
}

bool SupportsHierarchicalLevel(const ax::Role role) {
  switch (role) {
    case ax::Role::kComment:
    case ax::Role::kListItem:
    case ax::Role::kRow:
    case ax::Role::kTabList:
    case ax::Role::kTreeItem:
      return true;
    default:
      return false;
  }
}

bool SupportsOrientation(const ax::Role role) {
  switch (role) {
    case ax::Role::kComboBoxGrouping:
    case ax::Role::kComboBoxMenuButton:
    case ax::Role::kListBox:
    case ax::Role::kMenu:
    case ax::Role::kMenuBar:
    case ax::Role::kRadioGroup:
    case ax::Role::kScrollBar:
    case ax::Role::kSlider:
    case ax::Role::kSplitter:
    case ax::Role::kTabList:
    case ax::Role::kToolbar:
    case ax::Role::kTreeGrid:
    case ax::Role::kTree:
      return true;
    default:
      return false;
  }
}

bool SupportsSelected(const ax::Role role) {
  switch (role) {
    case ax::Role::kCell:
    case ax::Role::kColumnHeader:
    case ax::Role::kListBoxOption:
    case ax::Role::kMenuListOption:
    case ax::Role::kRow:
    case ax::Role::kRowHeader:
    case ax::Role::kTab:
    case ax::Role::kTreeItem:
      return true;
    default:
      return false;
  }
}

bool SupportsToggle(const ax::Role role) {
  switch (role) {
    case ax::Role::kCheckBox:
    case ax::Role::kMenuItemCheckBox:
    case ax::Role::kSwitch:
    case ax::Role::kToggleButton:
      return true;
    default:
      return false;
  }
}

bool ShouldHaveReadonlyStateByDefault(const ax::Role role) {
  switch (role) {
    case ax::Role::kArticle:
    case ax::Role::kDefinition:
    case ax::Role::kDescriptionList:
    case ax::Role::kDescriptionListTerm:
    case ax::Role::kDocument:
    case ax::Role::kGraphicsDocument:
    case ax::Role::kImage:
    case ax::Role::kImageMap:
    case ax::Role::kList:
    case ax::Role::kListItem:
    case ax::Role::kProgressIndicator:
    case ax::Role::kRootWebArea:
    case ax::Role::kTerm:
    case ax::Role::kTimer:
    case ax::Role::kToolbar:
    case ax::Role::kTooltip:
    case ax::Role::kWebArea:
      return true;

    case ax::Role::kGrid:
      // TODO(aleventhal) this changed between ARIA 1.0 and 1.1,
      // need to determine whether grids/treegrids should really be readonly
      // or editable by default
      break;

    default:
      break;
  }
  return false;
}

}  // namespace ax
